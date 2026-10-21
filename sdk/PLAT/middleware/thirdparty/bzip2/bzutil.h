
#ifndef BZUTIL_H
# define BZUTIL_H

# include <stdint.h>
# include <string.h>
# include <stdio.h>

//#define BZ2_BUFF_BLOCK_SIZE 70000

#define BZ2_BUFF_BLOCK_SMALL_SIZE 8192
#define BZ2_BUFF_BLOCK_LARGE_SIZE 70000


typedef struct {
  uint32_t cursor;
  uint32_t encBufSize;
  uint8_t* encBuf;
}buf_handle;

int32_t BZ2_bzBuffBlkSize (void);
void    BZ2_bzBuffOpen (buf_handle* hd, uint32_t size, uint8_t* buff);
int32_t BZ2_bzBuffRead (char* buffer, uint32_t size, uint32_t count, buf_handle* handle);
int32_t BZ2_bzBuffWrite (char* buffer, uint32_t size, uint32_t count, buf_handle* handle);


#endif

