/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718 
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "time.h"
#include "storage.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "bsp.h"
#include "packet.h"

#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "ff.h"
#endif
#ifdef FEATURE_SUBSYS_ZIP_ENABLE
#include <zlib.h>
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE

#ifdef FEATURE_SUBSYS_ZIP_ENABLE

static uint8_t buffer[100]={0};
static uint8_t outbuffer[1000]={0};

int zip_file(char *in_file, char *out_file)
{
    printf("zip_file start! \r\n");
    int ret = 0;
    struct stat  buf        = {0};
    FILE     *infile_h    = NULL;
    FILE     *outfile_h   = NULL;
    
    z_stream stream;

    infile_h = file_fopen(in_file, "rb");
    if (infile_h == NULL)
    {
        printf("Failed to open the file %s.\r\n", in_file);
    }

    file_fstat((int)infile_h, &buf);
    if (buf.st_size == 0)
    {
        printf("File is empty.\r\n");
    }

    outfile_h = file_fopen(out_file, "wb");
    if (infile_h == NULL)
    {
        printf("Failed to open the out file %s.\r\n", out_file);
    }

    // 初始化流
    printf("Initialize the deflate stream start.\r\n");
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        // 错误处理
        printf("Failed to initialize the deflate stream.\r\n");
    }
    
    // 压缩数据
    stream.next_in = buffer;
    stream.avail_in = sizeof(buffer);
    stream.next_out = outbuffer;
    stream.avail_out = sizeof(outbuffer);

    printf("deflateInit over.\r\n");
    for(;;)
    {
        if(file_fread((void *)buffer, sizeof(buffer), 1, infile_h) != 100)
        {
            printf("last data.\r\n");
            ret = deflate(&stream, Z_FINISH);
            if (ret == Z_STREAM_END) {
                // 压缩完成
                printf("stream.avail_out:%d.\r\n",stream.avail_out);
                file_fwrite(outbuffer, 1, sizeof(outbuffer) - stream.avail_out, outfile_h);
                printf("Compression completed.\r\n");
            } else {
                // 错误处理
                printf("Failed to compress the data.\r\n");
            }
            break;
        }else{
            ret = deflate(&stream, Z_NO_FLUSH);
            file_fwrite(outbuffer, 1, sizeof(outbuffer) - stream.avail_out, outfile_h);
            stream.next_out = outbuffer;
            stream.avail_out = sizeof(outbuffer);
            printf("Compression loop.\r\n");          
        }
    }
    
    // 清理并结束压缩
    ret = deflateEnd(&stream);
    if (ret != Z_OK) {
        // 错误处理
        printf("Failed to end the deflate stream.\r\n");
    }else{
        printf("zip file success\r\n");
    }

    file_fclose(infile_h);
    file_fclose(outfile_h);
    
    return ret;
}

void console_zip_file(int argc, char **argv)
{
    printf("console_zip_file\r\n");
    printf("start zip file %s -> %s\r\n",argv[1],argv[2]);
    zip_file(argv[1],argv[2]);
}
MSH_CMD_EXPORT_ALIAS(console_zip_file, zip, app zip file);

void console_unzip_file(int argc, char **argv)
{
    printf("console_unzip_file\r\n");
    printf("%s\r\n",argv[1]);
}
MSH_CMD_EXPORT_ALIAS(console_unzip_file, unzip, app unzip file);

#endif

#endif

#endif