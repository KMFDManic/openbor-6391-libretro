#include <stdio.h>
#include "detect.h"

void ob_detect_version(const char *pak_path, const int *ids, int count, ob_detect_result *out)
{
    (void)pak_path;
    out->version = count ? ids[count - 1] : -1;
    snprintf(out->reason, sizeof(out->reason), "detection not implemented, using newest");
}
