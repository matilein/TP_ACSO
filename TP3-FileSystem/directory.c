#include <string.h>
#include <stdint.h>
#include "directory.h"
#include "file.h"
#include "inode.h"
#include "unixfilesystem.h"
#include "direntv6.h"
#include "ino.h"
#include "diskimg.h"


int directory_findname(struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt) {
    struct inode inp;
    if (inode_iget(fs, dirinumber, &inp) < 0) return -1;
    if ((inp.i_mode & IFMT) != IFDIR) return -1;

    int32_t size = (inp.i_size0 << 16) | inp.i_size1;
    int nblocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    for (int b = 0; b < nblocks; b++) {
        uint8_t buf[DISKIMG_SECTOR_SIZE];
        int n = file_getblock(fs, dirinumber, b, buf);
        if (n < sizeof(*dirEnt)) return -1;
        int entries = n / sizeof(*dirEnt);
        for (int i = 0; i < entries; i++) {
            struct direntv6 *d = (void *)(buf + i * sizeof(*d));
            size_t len = strlen(name);
            if (len < sizeof(d->d_name) &&
                strncmp(d->d_name, name, len) == 0 &&
                d->d_name[len] == '\0') {
                *dirEnt = *d;
                return 0;
            }
        }
    }
    return -1;
}
