#include <stdlib.h>
#include <string.h>
#include "pathname.h"
#include "directory.h"
#include "unixfilesystem.h"

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname[0] != '/') return -1;
    int inum = ROOT_INUMBER; 
    if (pathname[1] == '\0') return inum;

    char *copy = strdup(pathname + 1);
    if (!copy) return -1;

    char *token = strtok(copy, "/");
    while (token) {
        struct direntv6 de;
        if (directory_findname(fs, token, inum, &de) < 0) {
            free(copy);
            return -1;
        }
        inum = de.d_inumber;
        token = strtok(NULL, "/");
    }

    free(copy);
    return inum;
}

