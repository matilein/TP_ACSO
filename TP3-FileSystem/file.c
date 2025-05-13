#include "file.h"
#include "inode.h"
#include "unixfilesystem.h"
#include "diskimg.h"
#include <stdint.h>

int file_getblock(struct unixfilesystem *fs,
                  int inumber,
                  int blockNo,
                  void *buf)
{
    struct inode inp;

    if (inode_iget(fs, inumber, &inp) < 0)
        return -1;


    uint32_t fsize = (inp.i_size0 << 16) | inp.i_size1;
    if (fsize == 0)
        return 0;


    int diskBlk = inode_indexlookup(fs, &inp, blockNo);
    if (diskBlk < 0)
        return -1;


    int r = diskimg_readsector(fs->dfd, diskBlk, buf);
    if (r != DISKIMG_SECTOR_SIZE)
        return -1;


    int offset = blockNo * DISKIMG_SECTOR_SIZE;
    if (offset >= (int)fsize)
        return 0;    

    int valid = fsize - offset;
    if (valid > DISKIMG_SECTOR_SIZE)
        valid = DISKIMG_SECTOR_SIZE;

    return valid;
}
