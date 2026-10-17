/* Prints the engine the core would pick for each .pak given on the command
 * line:  detect_cli game1.pak [game2.pak ...] */
#include <stdio.h>
#include "detect.h"

int main(int argc, char **argv)
{
    static const int ids[] = { 3711, 6391, 7555 };
    int i;
    for (i = 1; i < argc; i++)
    {
        ob_detect_result r;
        ob_detect_version(argv[i], ids, 3, &r);
        printf("%s: %d (%s)\n", argv[i], r.version, r.reason);
    }
    return 0;
}
