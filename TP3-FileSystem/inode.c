#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "inode.h"
#include "unixfilesystem.h"
#include "diskimg.h"
#include "ino.h"

#define NADDR 8                                             
#define NINDIR (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))    
#define INODES_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(struct inode))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) return -1;
    int blk_index = (inumber - 1) / INODES_PER_BLOCK;
    if (blk_index >= fs->superblock.s_isize) return -1;
    int sector = INODE_START_SECTOR + blk_index;
    struct inode buf[INODES_PER_BLOCK];
    if (diskimg_readsector(fs->dfd, sector, buf) != DISKIMG_SECTOR_SIZE)
        return -1;
    *inp = buf[(inumber - 1) % INODES_PER_BLOCK];
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if (blockNum < 0) return -1;
    int32_t size = (inp->i_size0 << 16) | inp->i_size1;
    int maxBlock = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    if (blockNum >= maxBlock) return -1;

    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum >= NADDR) return -1;
        return inp->i_addr[blockNum];
    }

    uint16_t indirect[NINDIR];

    if (blockNum < 7 * NINDIR) {
        int idx = blockNum / NINDIR;
        int off = blockNum % NINDIR;
        int sec = inp->i_addr[idx];
        if (diskimg_readsector(fs->dfd, sec, indirect) != DISKIMG_SECTOR_SIZE)
            return -1;
        return indirect[off];
    }

    int rem = blockNum - 7 * NINDIR;
    if (rem >= NINDIR * NINDIR) return -1;

    if (diskimg_readsector(fs->dfd, inp->i_addr[7], indirect) != DISKIMG_SECTOR_SIZE)
        return -1;
    int outer = rem / NINDIR;
    int sec2 = indirect[outer];

    if (diskimg_readsector(fs->dfd, sec2, indirect) != DISKIMG_SECTOR_SIZE)
        return -1;
    return indirect[rem % NINDIR];
}


int inode_getsize(struct inode *inp) {
    return ( (inp->i_size0 << 16)  | inp->i_size1);
}
