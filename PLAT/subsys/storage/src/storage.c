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
#ifdef FLASH_X_ENABLE
#include "merged.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif


#ifdef FEATURE_SUBSYS_FATFS_ENABLE
FATFS fsobject = {0};
#endif


void subStorageInit(void)
{
    static bool inited = false;
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    int8_t      retry  = 3;
#endif

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
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
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
#endif
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

    if (!((path[1] == ':')))
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
                memset(virtualFile, 0, sizeof(VirtualFileT));
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

int rmdir (const char *__path)
{
    return remove(__path);
}

int remove(const char *pathname)
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

int rename(const char *oldpath, const char *newpath)
{
    int32_t retVal = LFS_ERR_INVAL;

    if ((pathPrefixIsValid((char *)oldpath) != true) || (pathPrefixIsValid((char *)newpath) != true))
    {
        goto labelEnd;
    }

    if (strncasecmp(oldpath, newpath, 1) != 0)
    {
        SYSLOG_ERR("Must be in same partition.\r\n");
        goto labelEnd;
    }

    switch (oldpath[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_rename(&oldpath[3], &newpath[3]);
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_rename(&oldpath[3], &newpath[3]);
            }
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
            else if (pathToVirtualFile((char *)pathname, &virtualFile) != 0)
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
            retVal = f_close(pf);
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
            retVal = f_read(pf, ptr, length, (UINT *)&read_size);
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
            retVal = f_write(pf, ptr, length, (UINT *)&write_size);
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

char *file_fgets(char *s, int n, FILE *stream)
{
    char     *retVal = NULL;
    uint32_t  index  = 0;
    int32_t   size   = 0;
    int32_t   tell   = 0;

    if ((s == NULL) || (n <= 1) || (stream == NULL))
    {
        SYSLOG_ERR("Parameter error.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file   = (lfs_file_t *)(((FileHeadT *)stream)->fd);
    uint32_t    length = n - 1;
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL     *pf      = (FIL *)(((FileHeadT *)stream)->fd);
    FILINFO  fno     = {0};
    uint32_t readLen = 0;
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            size = LFS_fileSize(file);
            if (size <= 0)
            {
                goto labelEnd;
            }

            tell = LFS_fileTell(file);
            if ((tell < 0) || (tell == size))
            {
                goto labelEnd;
            }
            else if ((size - tell) < length)
            {
                length = size - tell;
            }

            for (index=0; index<length; index++)
            {
                if (LFS_fileRead(file, (void *)&s[index], 1) != 1)
                {
                    SYSLOG_ERR("Failed to read the file.\r\n");
                    s[index] = '\0';
                    goto labelEnd;
                }
                else
                {
                    if (s[index] == '\n')
                    {
                        index += 1;
                        break;
                    }
                }
            }
            s[index] = '\0';
            retVal = s;
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                size = LFSEX_fileSize(file);
                if (size <= 0)
                {
                    goto labelEnd;
                }

                tell = LFSEX_fileTell(file);
                if ((tell < 0) || (tell == size))
                {
                    goto labelEnd;
                }
                else if ((size - tell) < length)
                {
                    length = size - tell;
                }

                for (index=0; index<length; index++)
                {
                    if (LFSEX_fileRead(file, (void *)&s[index], 1) != 1)
                    {
                        SYSLOG_ERR("Failed to read the file.\r\n");
                        s[index] = '\0';
                        goto labelEnd;
                    }
                    else
                    {
                        if (s[index] == '\n')
                        {
                            index += 1;
                            break;
                        }
                    }
                }
                s[index] = '\0';
                retVal = s;
            }
#endif
            break;

        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            if ((f_stat((const TCHAR *)pf->fpath, &fno) != FR_OK) || (fno.fsize <= 0))
            {
                goto labelEnd;
            }
            size = fno.fsize;

            tell = f_tell(pf);
            if ((tell < 0) || (tell == size))
            {
                goto labelEnd;
            }
            else if ((size - tell) < length)
            {
                length = size - tell;
            }

            for (index=0; index<length; index++)
            {
                if ((f_read(pf, (void *)&s[index], 1, (UINT *)&readLen) != FR_OK) || (readLen != 1))
                {
                    SYSLOG_ERR("Failed to read the file.\r\n");
                    s[index] = '\0';
                    goto labelEnd;
                }
                else
                {
                    if (s[index] == '\n')
                    {
                        index += 1;
                        break;
                    }
                }
            }
            s[index] = '\0';
            retVal = s;
#endif
            break;

        case FLASH_PARTITION_R:
            size = file->ctz.size;
            tell = file->pos;
            if ((size <= 0) || (tell < 0) || (tell == size))
            {
                goto labelEnd;
            }
            else if ((size - tell) < length)
            {
                length = size - tell;
            }

            for (index=0; index<length; index++)
            {
                s[index] = ((char *)((uint32_t)file->name + file->pos))[index];
                if (s[index] == '\n')
                {
                    index += 1;
                    break;
                }
            }
            s[index] = '\0';
            retVal = s;
            break;

        case FLASH_PARTITION_X:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            size = file->ctz.size;
            tell = file->pos;
            if ((size <= 0) || (tell < 0) || (tell == size))
            {
                goto labelEnd;
            }
            else if ((size - tell) < length)
            {
                length = size - tell;
            }

            for (index=0; index<length; index++)
            {
                spiFlashRead((uint32_t)file->name + file->pos, (uint8_t *)&s[index], 1);
                if (s[index] == '\n')
                {
                    index += 1;
                    break;
                }
            }
            s[index] = '\0';
            retVal = s;
#endif
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

int file_fputs(const char *s, FILE *stream)
{
    int32_t retVal = -1;

    if ((s == NULL) || (strlen(s) == 0) || (stream == NULL))
    {
        SYSLOG_ERR("Parameter error.\r\n");
        goto labelEnd;
    }

    lfs_file_t *file = (lfs_file_t *)(((FileHeadT *)stream)->fd);
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    FIL     *pf       = (FIL *)(((FileHeadT *)stream)->fd);
    uint32_t writeLen = 0;
#endif

    switch (((FileHeadT *)stream)->partition)
    {
        case FLASH_PARTITION_C:
            if (LFS_fileWrite(file, (void *)s, strlen(s)) <= 0)
            {
                SYSLOG_ERR("Failed to write the file.\r\n");
                goto labelEnd;
            }
            retVal = 0;
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                if (LFSEX_fileWrite(file, (void *)s, strlen(s)) <= 0)
                {
                    SYSLOG_ERR("Failed to write the file.\r\n");
                    goto labelEnd;
                }
                retVal = 0;
            }
#endif
            break;

        case FLASH_PARTITION_E:
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
            if ((f_write(pf, s, strlen(s), (UINT *)&writeLen) != FR_OK) || (writeLen <= 0))
            {
                SYSLOG_ERR("Failed to write the file.\r\n");
                goto labelEnd;
            }
            f_sync(pf);
            retVal = 0;
#endif
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

int file_fseek(FILE *stream, long offset, int whence)
{
    int32_t retVal = -1;

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
            retVal = (retVal >= 0) ? 0 : -1;
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_fileSeek(file, offset, whence);
                retVal = (retVal >= 0) ? 0 : -1;
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
                        f_stat((const TCHAR *)pf->fpath, &fno);
                        retVal = f_lseek(pf, fno.fsize-offset);
                        break;
                    default:
                        break;
                }
                retVal = (retVal == FR_OK) ? 0 : -1;
            // SYSLOG_DEBUG("f_lseek pf->fptr:%d \r\n",pf->fptr);
#endif
            break;
        case FLASH_PARTITION_R:
        case FLASH_PARTITION_X:
            switch (whence)
            {
                case LFS_SEEK_SET:
                    file->pos = offset;
                    retVal    = 0;
                    break;

                case LFS_SEEK_CUR:
                    file->pos += offset;
                    retVal     = 0;
                    break;

                case LFS_SEEK_END:
                    file->pos = file->ctz.size - offset;
                    retVal    = 0;
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
    long retVal = LFS_ERR_INVAL;

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
            retVal = f_stat((const TCHAR *)pf->fpath,&fno);
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
    // uint32_t   read_size      = 0;
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

int mkdir(const char *pathname, mode_t mode)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (pathname == NULL)
    {
        SYSLOG_ERR("Param error.\r\n");
        goto labelEnd;
    }

    if (pathPrefixIsValid((char *)pathname) != true)
    {
        goto labelEnd;
    }

    switch (pathname[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_mkdir(&pathname[2]);
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_mkdir(&pathname[2]);
            }
#endif
            break;

        default:
            SYSLOG_ERR("Unknown partition: %c\r\n", pathname[0]);
            break;
    }

labelEnd:
    return ((retVal == LFS_ERR_OK) ? 0 : -1);
}

DIR *opendir(const char *pathname)
{
    int32_t    retVal   = LFS_ERR_INVAL;
    FileHeadT *fileHead = NULL;
    lfs_dir_t *dir      = NULL;

    if (pathname == NULL)
    {
        SYSLOG_ERR("Param error.\r\n");
        goto labelEnd;
    }

    if (pathPrefixIsValid((char *)pathname) != true)
    {
        goto labelEnd;
    }

    fileHead = (FileHeadT *)malloc(sizeof(FileHeadT));
    if (fileHead == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for fileHead.\r\n", sizeof(FileHeadT));
        goto labelEnd;
    }
    memset(fileHead, 0, sizeof(FileHeadT));

    switch (pathname[0])
    {
        case 'c':
        case 'C':
            dir = (lfs_dir_t *)malloc(sizeof(lfs_dir_t));
            if (dir == NULL)
            {
                SYSLOG_ERR("Failed to malloc %d bytes for dir.\r\n", sizeof(lfs_dir_t));
                goto labelEnd;
            }
            memset(dir, 0, sizeof(lfs_dir_t));
            retVal = LFS_dirOpen(dir, &pathname[2]);

            fileHead->partition = FLASH_PARTITION_C;
            fileHead->fd        = dir;
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                dir = (lfs_dir_t *)malloc(sizeof(lfs_dir_t));
                if (dir == NULL)
                {
                    SYSLOG_ERR("Failed to malloc %d bytes for dir.\r\n", sizeof(lfs_dir_t));
                    goto labelEnd;
                }
                memset(dir, 0, sizeof(lfs_dir_t));
                retVal = LFSEX_dirOpen(dir, &pathname[2]);

                fileHead->partition = FLASH_PARTITION_D;
                fileHead->fd        = dir;
            }
#endif
            break;

        default:
            SYSLOG_ERR("Unknown partition: %c\r\n", pathname[0]);
            break;
    }

labelEnd:
    if (retVal != LFS_ERR_OK)
    {
        if (dir != NULL)
        {
            free(dir);
            dir = NULL;
        }
        if (fileHead != NULL)
        {
            free(fileHead);
            fileHead = NULL;
        }
    }
    return (DIR *)fileHead;
}

int closedir(DIR *dir)
{
    int32_t    retVal   = LFS_ERR_INVAL;
    FileHeadT *fileHead = (FileHeadT *)dir;

    if (dir == NULL)
    {
        SYSLOG_ERR("Param error.\r\n");
        goto labelEnd;
    }

    switch (fileHead->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_dirClose(fileHead->fd);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_dirClose(fileHead->fd);
            }
#endif
            break;

        default:
            SYSLOG_ERR("Unknown partition: %c\r\n", fileHead->partition);
            break;
    }

labelEnd:
    if (fileHead != NULL)
    {
        free(fileHead);
        fileHead = NULL;
    }

    return ((retVal == LFS_ERR_OK) ? 0 : -1);
}

struct dirent *readdir(DIR *dir)
{
    int32_t                 retVal   = LFS_ERR_INVAL;
    FileHeadT              *fileHead = (FileHeadT *)dir;
    static struct lfs_info  info     = {0};

    if (dir == NULL)
    {
        SYSLOG_ERR("Param error.\r\n");
        goto labelEnd;
    }

    memset(&info, 0, sizeof(struct lfs_info));
    switch (fileHead->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_dirRead(fileHead->fd, &info);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_dirRead(fileHead->fd, &info);
            }
#endif
            break;

        default:
            SYSLOG_ERR("Unknown partition: %c\r\n", fileHead->partition);
            break;
    }

labelEnd:
    return ((retVal > 0) ? ((struct dirent *)(&info)) : NULL);
}

int32_t getFsTotalSize(uint8_t partation)
{
    int32_t      total  = -1;
    lfs_status_t status = {0};

    switch(partation)
    {
        case FLASH_PARTITION_C:
            if (LFS_statfs(&status) == LFS_ERR_OK)
            {
                total = status.total_block * status.block_size;
            }
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if ((spiFlashExist() == true) && (LFSEX_statfs(&status) == LFS_ERR_OK))
            {
                total = status.total_block * status.block_size;
            }
#endif
            break;

        default:
            break;
    }

    return total;
}

int32_t getFsFreeSize(uint8_t partation)
{
    int32_t      total  = 0;
    lfs_status_t status = {0};

    switch(partation)
    {
        case FLASH_PARTITION_C:
            if (LFS_statfs(&status) == LFS_ERR_OK)
            {
                total = (status.total_block - status.block_used) * status.block_size;
            }
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if ((spiFlashExist() == true) && (LFSEX_statfs(&status) == LFS_ERR_OK))
            {
                total = (status.total_block - status.block_used) * status.block_size;
            }
#endif
            break;

        default:
            break;
    }

    return total;
}

int statvfs(const char *path, struct statvfs *buf)
{
    int32_t      retVal = -1;
    lfs_status_t status = {0};

    if ((path == NULL) || (buf == NULL))
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    SYSLOG_DEBUG("path=%s\r\n", path);

    if (strcasecmp(path, "C:/") == 0)
    {
        if (LFS_statfs(&status) != LFS_ERR_OK)
        {
            SYSLOG_DEBUG("Failed to get the file system status.\r\n");
            goto labelEnd;
        }
    }
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    else if (strcasecmp(path, "D:/") == 0)
    {
        if ((spiFlashExist() != true) || (LFSEX_statfs(&status) != LFS_ERR_OK))
        {
            SYSLOG_DEBUG("Failed to get the file system status.\r\n");
            goto labelEnd;
        }
    }
#endif
    else
    {
        SYSLOG_DEBUG("Unknown file system: %s\r\n", path);
        goto labelEnd;
    }

    memset(buf, 0, sizeof(struct statvfs));
    buf->f_bsize   = status.block_size;
    buf->f_frsize  = buf->f_bsize;
    buf->f_blocks  = status.total_block;
    buf->f_bfree   = status.total_block - status.block_used;
    buf->f_bavail  = buf->f_bfree;
    buf->f_namemax = LFS_NAME_MAX;

    retVal = 0;

labelEnd:
    return retVal;
}

int stat(const char *pathname, struct stat *buf)
{
    int32_t         retVal  = -1;
    struct lfs_info lfsInfo = {0};

    if ((pathname == NULL) || (buf == NULL))
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    switch(pathname[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_stat(&pathname[3], &lfsInfo);
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_stat(&pathname[3], &lfsInfo);
            }
#endif
            break;

        default:
            break;
    }

    memset(buf, 0, sizeof(struct stat));
    if (retVal == 0)
    {
        switch (lfsInfo.type)
        {
            case LFS_TYPE_REG:
                buf->st_mode  = S_IFREG;
                break;

            case LFS_TYPE_DIR:
                buf->st_mode  = S_IFDIR;
                break;

            default:
                retVal = -1;
                break;
        }
    }

labelEnd:
    return retVal;
}

bool pathExists(const char *pathname)
{
    int32_t         retVal  = -1;
    struct lfs_info lfsInfo = {0};

    if (pathname == NULL)
    {
        SYSLOG_DEBUG("Parameter error.\r\n");
        goto labelEnd;
    }

    switch(pathname[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_stat(&pathname[3], &lfsInfo);
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (spiFlashExist() == true)
            {
                retVal = LFSEX_stat(&pathname[3], &lfsInfo);
            }
#endif
            break;

        default:
            break;
    }

labelEnd:
    return ((retVal == 0) ? true : false);
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

#ifdef DIR_TEST
void dirTest(void)
{
    char *dirPathRes     = "D:/res";
    char *dirPathResIcon = "D:/res/icon";
    char *dirPathResPic  = "D:/res/pic";

    mkdir(dirPathRes, 0);
    mkdir(dirPathResIcon, 0);
    mkdir(dirPathResPic, 0);


    printf("write %s begin\r\n", dirPathRes);
    DIR *dirRes = opendir(dirPathRes);
    if (dirRes == NULL)
    {
        printf("Failed to open %s\r\n", dirPathRes);
        return;
    }

    char *filePathResResJson = "D:/res/res.json";
    FILE *resResJson         = file_fopen(filePathResResJson, "w");
    if (resResJson != NULL)
    {
        file_fwrite(filePathResResJson, 1, strlen(filePathResResJson) + 1, resResJson);
        file_fclose(resResJson);
    }

    closedir(dirRes);
    printf("write %s end\r\n\r\n", dirPathRes);


    printf("write %s begin\r\n", dirPathResIcon);
    DIR *dirResIcon = opendir(dirPathResIcon);
    if (dirResIcon == NULL)
    {
        printf("Failed to open %s\r\n", dirPathResIcon);
        return;
    }

    char *filePathResIconIconJpeg = "D:/res/icon/icon.jpeg";
    FILE *resIconIconJpeg         = file_fopen(filePathResIconIconJpeg, "w");
    if (resIconIconJpeg != NULL)
    {
        file_fwrite(filePathResIconIconJpeg, 1, strlen(filePathResIconIconJpeg) + 1, resIconIconJpeg);
        file_fclose(resIconIconJpeg);
    }

    closedir(dirResIcon);
    printf("write %s end\r\n\r\n", dirPathResIcon);


    printf("write %s begin\r\n", dirPathResPic);
    DIR *dirResPic = opendir(dirPathResPic);
    if (dirResPic == NULL)
    {
        printf("Failed to open %s\r\n", dirPathResPic);
        return;
    }

    char *filePathResPicPicJpeg = "D:/res/pic/pic.jpeg";
    FILE *resPicPicJpeg         = file_fopen(filePathResPicPicJpeg, "w");
    if (resPicPicJpeg != NULL)
    {
        file_fwrite(filePathResPicPicJpeg, 1, strlen(filePathResPicPicJpeg) + 1, resPicPicJpeg);
        file_fclose(resPicPicJpeg);
    }

    closedir(dirResPic);
    printf("write %s end\r\n\r\n", dirPathResPic);


    printf("read %s begin\r\n", dirPathRes);
    dirRes = opendir(dirPathRes);
    if (dirRes == NULL)
    {
        printf("Failed to open %s\r\n", dirPathRes);
        return;
    }

    struct dirent   *direntInfo    = NULL;
    struct lfs_info *info          = NULL;
    char             filePath[256] = {0};
    FILE            *file          = NULL;
    uint8_t          buffer[256]   = {0};
    int32_t          length        = 0;
    while (1)
    {
        info = (struct lfs_info *)readdir(dirRes);
        if (info != NULL)
        {
            if (info->type == LFS_TYPE_REG)
            {
                memset(filePath, 0, sizeof(filePath));
                snprintf(filePath, sizeof(filePath), "%s/%s", dirPathRes, info->name);
                printf("FileName=%s, filePath=%s\r\n", info->name, filePath);
                file = file_fopen(filePath, "r");
                if (file != NULL)
                {
                    length = file_fread(buffer, 1, sizeof(buffer), file);
                    printf("buffer: length=%d, %s\r\n", length, buffer);
                    file_fclose(file);
                }
                else
                {
                    printf("Failed to open the file \"%s\"\r\n", filePath);
                }
            }
            else if (info->type == LFS_TYPE_DIR)
            {
                printf("DirName=%s\r\n", info->name);
            }
            else
            {
                printf("Unknown type: %d\r\n", info->type);
            }
        }
        else
        {
            break;
        }
    }

    closedir(dirRes);
    printf("read %s end\r\n\r\n", dirPathRes);


    printf("read %s begin\r\n", dirPathResIcon);
    dirResIcon = opendir(dirPathResIcon);
    if (dirResIcon == NULL)
    {
        printf("Failed to open %s\r\n", dirPathResIcon);
        return;
    }

    while (1)
    {
        info = (struct lfs_info *)readdir(dirResIcon);
        if (info != NULL)
        {
            if (info->type == LFS_TYPE_REG)
            {
                memset(filePath, 0, sizeof(filePath));
                snprintf(filePath, sizeof(filePath), "%s/%s", dirPathResIcon, info->name);
                printf("FileName=%s, filePath=%s\r\n", info->name, filePath);
                file = file_fopen(filePath, "r");
                if (file != NULL)
                {
                    memset(buffer, 0, sizeof(buffer));
                    length = file_fread(buffer, 1, sizeof(buffer), file);
                    printf("buffer: length=%d, %s\r\n", length, buffer);
                    file_fclose(file);
                }
                else
                {
                    printf("Failed to open the file \"%s\"\r\n", filePath);
                }
            }
            else if (info->type == LFS_TYPE_DIR)
            {
                printf("DirName=%s\r\n", info->name);
            }
            else
            {
                printf("Unknown type: %d\r\n", info->type);
            }
        }
        else
        {
            break;
        }
    }

    closedir(dirResIcon);
    printf("read %s end\r\n\r\n", dirPathResIcon);


    printf("read %s begin\r\n", dirPathResPic);
    dirResPic = opendir(dirPathResPic);
    if (dirResPic == NULL)
    {
        printf("Failed to open %s\r\n", dirPathResPic);
        return;
    }

    while (1)
    {
        info = (struct lfs_info *)readdir(dirResPic);
        if (info != NULL)
        {
            if (info->type == LFS_TYPE_REG)
            {
                memset(filePath, 0, sizeof(filePath));
                snprintf(filePath, sizeof(filePath), "%s/%s", dirPathResPic, info->name);
                printf("FileName=%s, filePath=%s\r\n", info->name, filePath);
                file = file_fopen(filePath, "r");
                if (file != NULL)
                {
                    memset(buffer, 0, sizeof(buffer));
                    length = file_fread(buffer, 1, sizeof(buffer), file);
                    printf("buffer: length=%d, %s\r\n", length, buffer);
                    file_fclose(file);
                }
                else
                {
                    printf("Failed to open the file \"%s\"\r\n", filePath);
                }
            }
            else if (info->type == LFS_TYPE_DIR)
            {
                printf("DirName=%s\r\n", info->name);
            }
            else
            {
                printf("Unknown type: %d\r\n", info->type);
            }
        }
        else
        {
            break;
        }
    }

    closedir(dirResPic);
    printf("read %s end\r\n\r\n", dirPathResPic);
}
#endif

#ifdef RENAME_TEST
void renameTest(void)
{
    char *dirPath     = "D:/dir";
    char *dirPathNew  = "D:/dirNew";
    char *filePath    = "D:/file.txt";
    char *filePathNew = "D:/fileNew.txt";

    mkdir(dirPath, 0);
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        printf("Failed to open the dir %s\r\n", dirPath);
        return;
    }
    closedir(dir);
    dir = NULL;

    rename(dirPath, dirPathNew);

    dir = opendir(dirPath);
    if (dir != NULL)
    {
        printf("Failed to Rename dir.\r\n");
        closedir(dir);
        dir = NULL;
        return;
    }

    dir = opendir(dirPathNew);
    if (dir == NULL)
    {
        printf("Failed to Rename dir.\r\n");
        return;
    }
    closedir(dir);
    dir = NULL;
    remove(dirPathNew);
    printf("Rename dir success.\r\n");


    FILE *file = file_fopen(filePath, "w");
    if (file == NULL)
    {
        printf("Failed to open the file %s\r\n", filePath);
        return;
    }
    file_fwrite(filePath, 1, strlen(filePath) + 1, file);
    file_fclose(file);

    rename(filePath, filePathNew);

    file = file_fopen(filePathNew, "r");
    if (file == NULL)
    {
        printf("Failed to rename file.\r\n");
        return;
    }
    file_fclose(file);
    remove(filePathNew);
    printf("Rename file success.\r\n");
}
#endif

#ifdef STAT_TEST
void statTest(void)
{
    char *dirPath        = "D:/dir";
    char *filePath       = "D:/file.txt";
    struct stat statInfo = {0};

    mkdir(dirPath, 0);
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        printf("Failed to open the dir %s\r\n", dirPath);
        return;
    }
    closedir(dir);
    dir = NULL;

    if ((stat(dirPath, &statInfo) == 0) && S_ISDIR(statInfo.st_mode))
    {
        printf("stat dir test: success\r\n");
    }
    else
    {
        printf("stat dir test: error\r\n");
    }

    if ((pathExists(dirPath) == true) && (pathExists("D:/pathExistsTest") != true))
    {
        printf("Dir Path Exists Test: success\r\n");
    }
    else
    {
        printf("Dir Path Exists Test: error\r\n");
    }

    remove(dirPath);


    FILE *file = file_fopen(filePath, "w");
    if (file == NULL)
    {
        printf("Failed to open the file %s\r\n", filePath);
        return;
    }
    file_fwrite(filePath, 1, strlen(filePath) + 1, file);
    file_fclose(file);

    if ((stat(filePath, &statInfo) == 0) && S_ISREG(statInfo.st_mode))
    {
        printf("stat file test: success\r\n");
    }
    else
    {
        printf("stat file test: error\r\n");
    }

    if ((pathExists(filePath) == true) && (pathExists("D:/pathExistsTest.txt") != true))
    {
        printf("File Path Exists Test: success\r\n");
    }
    else
    {
        printf("File Path Exists Test: error\r\n");
    }

    remove(filePath);
}
#endif

#ifdef FGETS_FPUTS_TEST
void fgetsFputsTest(void)
{
    char *path = "D:/fgetsFputsTest.txt";
    FILE *file = NULL;
    char *data = "12345\r\nabc";
    char buffer[32] = {0};

    file = file_fopen(path, "w+");
    if (file == NULL)
    {
        return;
    }

    memset(buffer, 0, sizeof(buffer));
    file_fputs(data, file);
    file_fseek(file, 0, LFS_SEEK_SET);
    file_fread(buffer, 1, strlen(data), file);
    printf("[%s][%d] %s\r\n", __FUNCTION__, __LINE__, buffer);

    memset(buffer, 0, sizeof(buffer));
    file_fseek(file, 0, LFS_SEEK_SET);
    file_fgets(buffer, sizeof(buffer), file);
    printf("[%s][%d] %s\r\n", __FUNCTION__, __LINE__, buffer);

    memset(buffer, 0, sizeof(buffer));
    file_fseek(file, 0, LFS_SEEK_SET);
    file_fgets(buffer, 3, file);
    printf("[%s][%d] %s\r\n", __FUNCTION__, __LINE__, buffer);

    data = "12345abcde";
    file_fseek(file, 0, LFS_SEEK_SET);
    file_fputs(data, file);

    memset(buffer, 0, sizeof(buffer));
    file_fseek(file, 0, LFS_SEEK_SET);
    file_fgets(buffer, sizeof(buffer), file);
    printf("[%s][%d] %s\r\n", __FUNCTION__, __LINE__, buffer);

    memset(buffer, 0, sizeof(buffer));
    file_fseek(file, 0, LFS_SEEK_SET);
    file_fgets(buffer, 3, file);
    printf("[%s][%d] %s\r\n", __FUNCTION__, __LINE__, buffer);

    file_fclose(file);
    remove(path);
}
#endif
