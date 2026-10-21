#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#include "storage.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "../fatfs/source/ff.h"
#ifdef FLASH_X_ENABLE
#include "merged.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif


FATFS fsobject = {0};
BYTE work[FF_MAX_SS] = {0};


void subStorageInit(void)
{
    static bool inited = false;
    int8_t      retry  = 3;

    if (inited == false)
    {
        inited = true;
    }
    else
    {
        return;
    }

#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    if (spiFlashInit() == 0)
    {
        while ((retry--) && (LFSEX_init() != 0))
        {
            SYSLOG_DEBUG("Failed to init LFS: retry=%d\r\n", retry);
            osDelay(10);
        }
    }
#endif

#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    if (sdInit() == 0)
    {
        // Init FAT32
        FRESULT  res ;  
        res = f_mount(&fsobject,  "0:",  1); 
        // SYSLOG_DEBUG("res = %d\r\n",res);
        if(res == FR_NO_FILESYSTEM) 
        {
            SYSLOG_DEBUG("FR_NO_FILESYSTEM\r\n");
            SYSLOG_DEBUG("res = %d\r\n",res);
            res = f_mount(NULL,  "0:",  1); 
            // res = f_mount(&fsobject,  "0:",  1);
 	    }else
        {
            SYSLOG_INFO("f_mount  is  OK\r\n");
        }
    }else
        SYSLOG_EMERG("sdInit err\r\n");
#endif
}

bool pathPrefixIsValid(char *path)
{
    bool retVal = false;

    if (!((path[0] == 'c') || (path[0] == 'C')
       || (path[0] == 'd') || (path[0] == 'D')
       || (path[0] == 'e') || (path[0] == 'E')
       || (path[0] == 'r') || (path[0] == 'R')
       || (path[0] == 'x') || (path[0] == 'X')))
    {
        SYSLOG_ERR("Unsupported Flash partition. path : %s, partition: %c\r\n", path, path[0]);
        goto labelEnd;
    }

    if (!((path[1] == ':') && (path[2] == '/')))
    {
        SYSLOG_ERR("path format error. path : %s\r\n", path);
        goto labelEnd;
    }

    retVal = true;

labelEnd:
    return retVal;
}

static int32_t pathToVirtualFile(char *path, VirtualFileT *virtualFile)
{
    int32_t retVal = -1;

#ifdef FLASH_X_ENABLE
    if ((path != NULL) && (strncasecmp(path, "X:/", strlen("X:/")) == 0))
    {
        for (uint32_t i=0; i<TOTAL_BIN_NUM; i++)
        {
            if ((strlen(&path[3]) == strlen(ext_bin_desc[i].path))
             && (memcmp(&path[3], ext_bin_desc[i].path, strlen(ext_bin_desc[i].path)) == 0))
            {
                memset(virtualFile, 0, sizeof(virtualFile));
                memcpy(virtualFile->prefix, "X:/", strlen("X:/"));
                virtualFile->address = ext_bin_desc[i].addr + (EF_IMG_GUI_FONT_LNA & (~EF_FLASH_XIP_ADDR));
                virtualFile->size    = ext_bin_desc[i].size;
                retVal = 0;
                break;
            }
        }
    }
#else
    SYSLOG_ERR("FLASH_X is Disable.\r\n");
#endif
    return retVal;
}

int file_remove(const char *pathname)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (pathPrefixIsValid((char *)pathname) != true)
    {
        goto labelEnd;
    }

    switch (pathname[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_remove(&pathname[3]);
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_remove(&pathname[3]);
            }
#endif
            break;
        case 'E':
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            retVal = f_unlink(&pathname[3]);
#endif
            break;
        default:
            break;
    }

labelEnd:
    return retVal;
}

FILE *file_fopen(const char *pathname, const char *mode)
{
    int32_t       retVal      = LFS_ERR_INVAL;
    int32_t       flags       = 0;
    FileHeadT    *fileHead    = NULL;
    uint8_t       partition   = 0xFF;
    lfs_file_t   *file        = NULL;
    VirtualFileT  virtualFile = {0};
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL          *pf          = NULL;
    int32_t       flagsFat    = 0;
#endif

    if ((pathname == NULL) || (mode == NULL) || (strlen(mode) > 3))
    {
        SYSLOG_ERR("Param error in file_fopen.\r\n");
        goto labelEnd;
    }

    if (pathPrefixIsValid((char *)pathname) != true)
    {
        goto labelEnd;
    }

    if ((strcmp(mode, "r") == 0) || ((strcmp(mode, "rb") == 0)))
    {
        flags = LFS_O_RDONLY;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        flagsFat = FA_READ;
#endif
    }
    else if ((strcmp(mode, "w") == 0) || (strcmp(mode, "wb") == 0))
    {
        flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        flagsFat = FA_WRITE | FA_CREATE_ALWAYS;
#endif
    }
    else if ((strcmp(mode, "a") == 0) || (strcmp(mode, "ab") == 0))
    {
        flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        flagsFat = FA_OPEN_APPEND | FA_WRITE | FA_READ ;
#endif
    }
    else if ((strcmp(mode, "r+") == 0) || (strcmp(mode, "rb+") == 0) || (strcmp(mode, "r+b") == 0))
    {
        flags = LFS_O_RDWR;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        flagsFat = FA_READ | FA_WRITE ;
#endif
    }
    else if ((strcmp(mode, "w+") == 0) || (strcmp(mode, "wb+") == 0) || (strcmp(mode, "w+b") == 0))
    {
        flags = LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        flagsFat = FA_READ | FA_WRITE | FA_CREATE_NEW;
#endif
    }
    else if ((strcmp(mode, "a+") == 0) || (strcmp(mode, "ab+") == 0) || (strcmp(mode, "a+b") == 0))
    {
        flags = LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        flagsFat = FA_READ | FA_WRITE | FA_OPEN_APPEND | FA_CREATE_NEW;
#endif
    }
    else
    {
        SYSLOG_ERR("Param \"mode\" not support.\r\n");
        goto labelEnd;
    }

    switch (pathname[0])
    {
        case 'c':
        case 'C':
            file = (lfs_file_t *)malloc(sizeof(lfs_file_t));
            if (file == NULL)
            {
                SYSLOG_ERR("Failed to malloc %d bytes for file.\r\n", sizeof(lfs_file_t));
                goto labelEnd;
            }
            memset(file, 0, sizeof(lfs_file_t));
            retVal = LFS_fileOpen(file, &pathname[3], flags);
            if (retVal != LFS_ERR_OK)
            {
                free(file);
                file = NULL;
                goto labelEnd;
            }
            partition = FLASH_PARTITION_C;
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            file = (lfs_file_t *)malloc(sizeof(lfs_file_t));
            if (file == NULL)
            {
                SYSLOG_ERR("Failed to malloc %d bytes for file.\r\n", sizeof(lfs_file_t));
                goto labelEnd;
            }
            memset(file, 0, sizeof(lfs_file_t));
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileOpen(file, &pathname[3], flags);
            }
            if (retVal != LFS_ERR_OK)
            {
                free(file);
                file = NULL;
                goto labelEnd;
            }
            partition = FLASH_PARTITION_D;
#endif
            break;
        case 'e':
        case 'E':
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            pf = (FIL *)malloc(sizeof(FIL));
            if (pf == NULL)
            {
                SYSLOG_ERR("Failed to malloc %d bytes for file.\r\n", sizeof(FIL));
                goto labelEnd;
            }
            memset(pf, 0, sizeof(FIL));
            retVal = f_open(pf,&pathname[3], flagsFat); 
            // SYSLOG_DEBUG("file_fopen partition: %d .\r\n", pf->partition);
            // SYSLOG_DEBUG("file_fopen fpath: %s .\r\n", pf->fpath);
            // SYSLOG_DEBUG("f_open ret: %d .\r\n", retVal);
            if (retVal != FR_OK)
            {
                free(pf);
                pf = NULL;
                goto labelEnd;
            }
            partition = FLASH_PARTITION_E;
#endif
            break;
        case 'r':
        case 'R':
        case 'x':
        case 'X':
            partition = ((pathname[0] == 'r') || (pathname[0] == 'R')) ? FLASH_PARTITION_R : FLASH_PARTITION_X;
            if (partition == FLASH_PARTITION_R)
            {
                memcpy(&virtualFile, pathname, sizeof(virtualFile));
            }
            else if (pathToVirtualFile(pathname, &virtualFile) != 0)
            {
                SYSLOG_ERR("Failed to get virtual file head.\r\n");
                goto labelEnd;
            }

            file = (lfs_file_t *)malloc(sizeof(lfs_file_t));
            if (file == NULL)
            {
                SYSLOG_ERR("Failed to malloc %d bytes for file.\r\n", sizeof(lfs_file_t));
                goto labelEnd;
            }
            memset(file, 0, sizeof(lfs_file_t));
            file->ctz.size = virtualFile.size;
            file->name     = (char *)(virtualFile.address);
            file->pos      = 0;
            retVal         = LFS_ERR_OK;
            break;

        default:
            goto labelEnd;
            break;
    }

    fileHead = (FileHeadT *)malloc(sizeof(FileHeadT));
    if (fileHead == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for fileHead.\r\n", sizeof(FileHeadT));
        if (file != NULL)
        {
            free(file);
            file = NULL;
        }
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
        if (pf != NULL)
        {
            free(pf);
            pf = NULL;
        }
#endif
        goto labelEnd;
    }

    fileHead->partition = partition;
    switch (partition)
    {
        case FLASH_PARTITION_C:
        case FLASH_PARTITION_D:
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            fileHead->fd = file;
            break;

        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            fileHead->fd = pf;
#endif
            break;

        default:
            if (fileHead != NULL)
            {
                free(fileHead);
                fileHead = NULL;
            }
            break;
    }

labelEnd:
    return (FILE *)fileHead;
}

int file_fclose(FILE *stream)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (stream == NULL)
    {
        SYSLOG_ERR("Param error in file_fclose.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file = (lfs_file_t *)(((FileHeadT *)stream)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL        *pf   = (FIL *)(((FileHeadT *)stream)->fd);
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileClose(file);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileClose(file);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            retVal = f_closedir(pf);
#endif
            break;
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            file->name     = NULL;
            file->ctz.size = 0;
            file->pos      = 0;
            retVal         = LFS_ERR_OK;
            break;

        default:
            break;
    }

    free(((FileHeadT *)stream)->fd);
    ((FileHeadT *)stream)->fd = NULL;
    free(stream);
    stream = NULL;

labelEnd:
    return retVal;
}

size_t file_fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
    int32_t     retVal  = LFS_ERR_INVAL;

    if ((ptr == NULL) || (size == 0) || (nitems == 0) || (stream == NULL))
    {
        SYSLOG_ERR("Param error in file_fread.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file   = (lfs_file_t *)(((FileHeadT *)stream)->fd);
    uint32_t    length = size * nitems;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)stream)->fd);
    uint32_t   read_size      = 0;
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileRead(file, ptr, length);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileRead(file, ptr, length);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            retVal = f_read(pf, ptr, length,&read_size);
#endif
            break;
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            if ((file->pos + length) > file->ctz.size)
            {
                length = file->ctz.size - file->pos;
            }
            if (((FileHeadT *)stream)->partition == FLASH_PARTITION_R)
            {
                memcpy(ptr, (void *)((uint32_t)file->name + file->pos), length);
            }
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            else
            {
                spiFlashRead((uint32_t)file->name + file->pos, (uint8_t *)ptr, length);
            }
#endif
            file->pos += length;
            retVal     = length;
            break;

        default:
            break;
    }

labelEnd:
    return ((retVal < 0) ? 0 : (retVal / size));
}

/* Partial writes were not considered. */ 
size_t file_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream)
{
    int32_t retVal = LFS_ERR_INVAL;

    if ((ptr == NULL) || (size == 0) || (nitems == 0) || (stream == NULL))
    {
        SYSLOG_ERR("Param error in file_fwrite.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file   = (lfs_file_t *)(((FileHeadT *)stream)->fd);
    uint32_t    length = size * nitems;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)stream)->fd);
    uint32_t   write_size      = 0;
    // SYSLOG_DEBUG("file_fwrite \r\n");
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileWrite(file, ptr, length);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileWrite(file, ptr, length);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            retVal = f_write(pf, ptr, length,&write_size);
            // SYSLOG_DEBUG("f_write length:%d , write_size :%d\r\n",length,write_size);
            f_sync(pf);
#endif
            break;
        default:
            break;
    }

labelEnd:
    return ((retVal < 0) ? 0 : nitems);
}


int file_fseek(FILE *stream, long offset, int whence)
{
    int32_t retVal = LFS_ERR_INVAL;

    if ((stream == NULL) || (whence < SEEK_SET) || (whence > SEEK_END))
    {
        SYSLOG_ERR("Param error in file_fseek.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file = (lfs_file_t *)(((FileHeadT *)stream)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)stream)->fd);
    FILINFO fno = {0};
    FSIZE_t f_size = offset;
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileSeek(file, offset, whence);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileSeek(file, offset, whence);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
                switch (whence)
                {
                    case LFS_SEEK_SET:
                        retVal = f_lseek(pf, f_size);
                        break;
                    case LFS_SEEK_CUR:
                        
                        retVal = f_lseek(pf, pf->fptr+offset);
                        break;
                    case LFS_SEEK_END:
                        f_stat(pf->fpath,&fno);
                        retVal = f_lseek(pf, fno.fsize-offset);
                        break;
                    default:
                        break;
                }
            // SYSLOG_DEBUG("f_lseek pf->fptr:%d \r\n",pf->fptr);
#endif
            break;
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            switch (whence)
            {
                case LFS_SEEK_SET:
                    file->pos = offset;
                    retVal    = file->pos;
                    break;

                case LFS_SEEK_CUR:
                    file->pos += offset;
                    retVal     = file->pos;
                    break;

                case LFS_SEEK_END:
                    file->pos = file->ctz.size - offset;
                    retVal    = file->pos;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

long file_ftell(FILE *stream)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (stream == NULL)
    {
        SYSLOG_ERR("Param error in file_ftell.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file = (lfs_file_t *)(((FileHeadT *)stream)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)stream)->fd);
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileTell(file);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileTell(file);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            retVal = f_tell(pf);
#endif
            break;

        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            retVal = file->pos;
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

void file_rewind(FILE *stream)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (stream == NULL)
    {
        SYSLOG_ERR("Param error in file_rewind.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file = (lfs_file_t *)(((FileHeadT *)stream)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)stream)->fd);
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileRewind(file);
            break;
        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileRewind(file);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            retVal = f_rewind(pf);
#endif
            break;
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            file->pos = 0;
            retVal    = LFS_ERR_OK;
            break;

        default:
            break;
    }

    if (retVal != LFS_ERR_OK)
    {
        SYSLOG_ERR("Failed to file_rewind.\r\n");
    }

labelEnd:
    return;
}

int file_fstat(int fildes, struct stat *buf)
{
    int32_t retVal = LFS_ERR_INVAL;

    if ((fildes == 0) || (buf == NULL))
    {
        SYSLOG_ERR("Param error in file_fstat.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file   = (lfs_file_t *)(((FileHeadT *)fildes)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)fildes)->fd);
    FILINFO fno = {0};
#endif

    switch (((FileHeadT *)fildes)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileSize(file);
            if (retVal >= 0)
            {
                buf->st_size = retVal;
                retVal       = 0;
            }
            else
            {
                buf->st_size = 0;
                retVal       = -1;
            }
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileSize(file);
            }
            if (retVal >= 0)
            {
                buf->st_size = retVal;
                retVal       = 0;
            }
            else
            {
                buf->st_size = 0;
                retVal       = -1;
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            // SYSLOG_DEBUG("f_stat fpath: %s .\r\n", pf->fpath);
            retVal = f_stat(pf->fpath,&fno);
            // SYSLOG_DEBUG("fno.fsize : %d\r\n",fno.fsize);
            buf->st_size = fno.fsize;
            retVal  = 0;
#endif
            break;
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            buf->st_size = file->ctz.size;
            retVal       = 0;
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

int file_truncate(int fildes, off_t length)
{
    int32_t     retVal = LFS_ERR_INVAL;
    lfs_file_t *file   = (lfs_file_t *)(((FileHeadT *)fildes)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL *pf = (FIL *)(((FileHeadT *)fildes)->fd);
    FSIZE_t f_size = (FSIZE_t)length;
    uint32_t   read_size      = 0;
    // SYSLOG_DEBUG("file_truncate.\r\n");
#endif
    if (file == NULL)
    {
        SYSLOG_ERR("Param error in file_truncate.\r\n");
        goto labelEnd;
    }

    switch (((FileHeadT *)fildes)->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileTruncate(file, length);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileTruncate(file, length);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            // SYSLOG_DEBUG("file_truncate length:%d.\r\n",length);
            f_lseek(pf,f_size);
            // uint8_t *ptr = malloc(length);
            // f_read(pf, ptr, length,&read_size);
            // SYSLOG_DEBUG("file_truncate f_read %d \r\n",read_size);
            // SYSLOG_DEBUG("file_truncate pf->fptr:%d \r\n",pf->fptr);
            retVal = f_truncate(pf);
            // free(ptr);
            f_sync(pf);
            
#endif
            break;
        default:
            break;
    }

labelEnd:
    return ((retVal == LFS_ERR_OK) ? 0 : -1);
}

int32_t fsDirOpen(lfs_dir_t *dir, char *path)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (pathPrefixIsValid(path) != true)
    {
        goto labelEnd;
    }

    switch (path[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_dirOpen(dir, &path[2]);
            dir->partition = FLASH_PARTITION_C;
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_dirOpen(dir, &path[2]);
                dir->partition = FLASH_PARTITION_D;
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
#endif
            break;
        default:
            break;
    }

labelEnd:
    return retVal;
}

int32_t fsDirClose(lfs_dir_t *dir)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (dir->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_dirClose(dir);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_dirClose(dir);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
#endif
            break;
        default:
            break;
    }

    return retVal;
}

int32_t fsDirRead(lfs_dir_t *dir, struct lfs_info *info)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (dir->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_dirRead(dir, info);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_dirRead(dir, info);
            }
#endif
            break;
        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
#endif
            break;
        default:
            break;
    }

    return retVal;
}

int32_t fsStatFs(lfs_status_t *status)
{
    int32_t retVal = LFS_ERR_INVAL;

    retVal = LFS_statfs(status);
    if (retVal != LFS_ERR_OK)
    {
        goto labelEnd;
    }
    
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    if (spiFlashExist() == true)
    {
        retVal = LFSEX_statfs(status + 1);
        if (retVal != LFS_ERR_OK)
        {
            goto labelEnd;
        }
    }
#endif

#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
#endif

labelEnd:
    return retVal;
}

#ifdef FLASH_X_TEST
static PLAT_FPSRAM_ZI_CUST uint8_t gBufferTest[80*1024] = {0};
void testBinFile(void)
{
    FILE *file = NULL;
    char *path = "X:/lv_font_free_36.bin";

    osDelay(10000);
    file = file_fopen(path, "r");
    if (file != NULL)
    {
        file_fread((void *)gBufferTest, sizeof(gBufferTest), 1, file);
        SYSLOG_ERR("gBufferTest: \r\n");
        for (uint32_t i=0; i<sizeof(gBufferTest); i++)
        {
            printf("%02X ", gBufferTest[i]);
            if ((i % 1024) == 0)
            {
                osDelay(1);
            }
        }
        printf("\r\n\r\n");
    }
    else
    {
        SYSLOG_ERR("Failed to open the file \"%s\"\r\n", path);
    }
}
#endif
