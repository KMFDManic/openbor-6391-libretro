/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - threads (only used by the WebM decoder)
 *
 * Same semantics as the SDL backend: recursive mutexes, and
 * cond_wait_timed() returns 0 when signalled, 1 on timeout, -1 on error.
 * pthreads everywhere except Windows, which uses native Win32 primitives so
 * the core does not depend on a pthreads DLL.
 */

#include <stdlib.h>
#include "threads.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

struct bor_thread { HANDLE handle; int (*fn)(void *); void *data; };
struct bor_mutex  { CRITICAL_SECTION cs; };
struct bor_cond   { CONDITION_VARIABLE cv; };

static unsigned __stdcall thread_entry(void *arg)
{
    bor_thread *t = (bor_thread *)arg;
    return (unsigned)t->fn(t->data);
}

bor_thread *thread_create(int (*fn)(void *), const char *name, void *data)
{
    bor_thread *t = (bor_thread *)calloc(1, sizeof(*t));
    (void)name;
    if (!t) return NULL;
    t->fn = fn;
    t->data = data;
    t->handle = (HANDLE)_beginthreadex(NULL, 0, thread_entry, t, 0, NULL);
    if (!t->handle) { free(t); return NULL; }
    return t;
}

void thread_join(bor_thread *t)
{
    if (!t) return;
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
    free(t);
}

bor_mutex *mutex_create(void)
{
    bor_mutex *m = (bor_mutex *)calloc(1, sizeof(*m));
    if (m) InitializeCriticalSection(&m->cs);
    return m;
}

void mutex_destroy(bor_mutex *m)
{
    if (!m) return;
    DeleteCriticalSection(&m->cs);
    free(m);
}

int mutex_lock(bor_mutex *m)   { if (!m) return -1; EnterCriticalSection(&m->cs); return 0; }
int mutex_unlock(bor_mutex *m) { if (!m) return -1; LeaveCriticalSection(&m->cs); return 0; }

bor_cond *cond_create(void)
{
    bor_cond *c = (bor_cond *)calloc(1, sizeof(*c));
    if (c) InitializeConditionVariable(&c->cv);
    return c;
}

void cond_destroy(bor_cond *c) { free(c); }
int cond_signal(bor_cond *c)    { if (!c) return -1; WakeConditionVariable(&c->cv); return 0; }
int cond_broadcast(bor_cond *c) { if (!c) return -1; WakeAllConditionVariable(&c->cv); return 0; }

int cond_wait(bor_cond *c, bor_mutex *m)
{
    if (!c || !m) return -1;
    return SleepConditionVariableCS(&c->cv, &m->cs, INFINITE) ? 0 : -1;
}

int cond_wait_timed(bor_cond *c, bor_mutex *m, int ms)
{
    if (!c || !m) return -1;
    if (SleepConditionVariableCS(&c->cv, &m->cs, ms < 0 ? 0 : (DWORD)ms))
        return 0;
    return GetLastError() == ERROR_TIMEOUT ? 1 : -1;
}

int SDL_GetCPUCount(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors > 0 ? (int)info.dwNumberOfProcessors : 1;
}

int SDL_AtomicGet(SDL_atomic_t *a)
{
    return InterlockedCompareExchange((volatile LONG *)&a->value, 0, 0);
}

int SDL_AtomicSet(SDL_atomic_t *a, int v)
{
    return InterlockedExchange((volatile LONG *)&a->value, v);
}

#else /* pthreads */

#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

struct bor_thread { pthread_t thread; int (*fn)(void *); void *data; };
struct bor_mutex  { pthread_mutex_t mutex; };
struct bor_cond   { pthread_cond_t cond; };

static void *thread_entry(void *arg)
{
    bor_thread *t = (bor_thread *)arg;
    t->fn(t->data);
    return NULL;
}

bor_thread *thread_create(int (*fn)(void *), const char *name, void *data)
{
    bor_thread *t = (bor_thread *)calloc(1, sizeof(*t));
    (void)name;
    if (!t) return NULL;
    t->fn = fn;
    t->data = data;
    if (pthread_create(&t->thread, NULL, thread_entry, t) != 0)
    {
        free(t);
        return NULL;
    }
    return t;
}

void thread_join(bor_thread *t)
{
    if (!t) return;
    pthread_join(t->thread, NULL);
    free(t);
}

bor_mutex *mutex_create(void)
{
    pthread_mutexattr_t attr;
    bor_mutex *m = (bor_mutex *)calloc(1, sizeof(*m));
    if (!m) return NULL;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (pthread_mutex_init(&m->mutex, &attr) != 0)
    {
        free(m);
        m = NULL;
    }
    pthread_mutexattr_destroy(&attr);
    return m;
}

void mutex_destroy(bor_mutex *m)
{
    if (!m) return;
    pthread_mutex_destroy(&m->mutex);
    free(m);
}

int mutex_lock(bor_mutex *m)   { return (m && pthread_mutex_lock(&m->mutex) == 0) ? 0 : -1; }
int mutex_unlock(bor_mutex *m) { return (m && pthread_mutex_unlock(&m->mutex) == 0) ? 0 : -1; }

bor_cond *cond_create(void)
{
    bor_cond *c = (bor_cond *)calloc(1, sizeof(*c));
    if (c && pthread_cond_init(&c->cond, NULL) != 0)
    {
        free(c);
        c = NULL;
    }
    return c;
}

void cond_destroy(bor_cond *c)
{
    if (!c) return;
    pthread_cond_destroy(&c->cond);
    free(c);
}

int cond_signal(bor_cond *c)    { return (c && pthread_cond_signal(&c->cond) == 0) ? 0 : -1; }
int cond_broadcast(bor_cond *c) { return (c && pthread_cond_broadcast(&c->cond) == 0) ? 0 : -1; }

int cond_wait(bor_cond *c, bor_mutex *m)
{
    return (c && m && pthread_cond_wait(&c->cond, &m->mutex) == 0) ? 0 : -1;
}

int cond_wait_timed(bor_cond *c, bor_mutex *m, int ms)
{
    struct timespec ts;
    struct timeval tv;
    int r;

    if (!c || !m) return -1;
    gettimeofday(&tv, NULL);
    if (ms < 0) ms = 0;
    ts.tv_sec = tv.tv_sec + ms / 1000;
    ts.tv_nsec = tv.tv_usec * 1000L + (long)(ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
    r = pthread_cond_timedwait(&c->cond, &m->mutex, &ts);
    if (r == 0) return 0;
    return r == ETIMEDOUT ? 1 : -1;
}

int SDL_GetCPUCount(void)
{
#ifdef _SC_NPROCESSORS_ONLN
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n > 0) return (int)n;
#endif
    return 1;
}

int SDL_AtomicGet(SDL_atomic_t *a)
{
    return __atomic_load_n(&a->value, __ATOMIC_SEQ_CST);
}

int SDL_AtomicSet(SDL_atomic_t *a, int v)
{
    return __atomic_exchange_n(&a->value, v, __ATOMIC_SEQ_CST);
}

#endif
