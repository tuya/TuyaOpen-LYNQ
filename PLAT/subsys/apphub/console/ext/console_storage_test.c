/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_storage.c
 * Description:  EC718
 * History:      Rev1.0   2024-08-06
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
#include "bsp.h"
#include "packet.h"

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

#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#endif

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "cJSON.h"
#include "jsonDb.h"
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

typedef struct
{
    char name[40];
    char number[40];
} JsonDbTestT;

extern int skip_atoi(const char **s);

int cmd_storage(int argc, char **argv)
{

    char *sub_cmd = argv[1];
    // storage write c:/test.txt w+ 123456
    if (strcmp(sub_cmd, "write") == 0)
    {
        FILE *write_file = NULL;
        char *fileName = argv[2];
        char *mode = argv[3];
        char *data = argv[4];
        write_file = file_fopen(fileName, mode);
        if (write_file != NULL)
        {
            file_fwrite(data, strlen(data), 1, write_file);
            file_fclose(write_file);
        }
        else
        {
            rt_kprintlnf("WRITE FAIL");
            

            return 1;
        }
    }
    // storage read c:/test.txt
    else if (strcmp(sub_cmd, "read") == 0)
    {
        uint8_t size = 0;
        char *fileName = 0;
        FILE *readFile = NULL;
        struct stat buf = {0};

        fileName = argv[2];
        readFile = file_fopen(fileName, "r");
        if (readFile != NULL)
        {
            file_fstat((int)readFile, &buf);
            size = buf.st_size;
            rt_kprintlnf("read size = %d", size);
            char buffer[size];
            file_fread(buffer, size, 1, readFile);
            buffer[size] = '\0';
            rt_kprintlnf("%s", buffer);
            file_fclose(readFile);
        }
        else
        {
            rt_kprintlnf("READ FAIL");
            

            return 1;
        }
    }
    // storage rm c:/test.txt
    else if (strcmp(sub_cmd, "rm") == 0)
    {
        char *fileName = 0;
        fileName = argv[2];
        int ret = remove(fileName);
        if (ret == 0)
        {
        }
        else
        {
            rt_kprintlnf("RM FAIL");
            
            return ret;
        }
    }
    // storage ls c
    else if (strcmp(sub_cmd, "ls") == 0)
    {
        int32_t errCode;
        CHAR printfBuf[256] = {0};
        CHAR *pcWriteBuffer = printfBuf;
        char *drive = argv[2];
        DIR *dir = NULL;
        struct lfs_info *info = NULL;
        lfs_status_t status[2] = {0};

        uint8_t index = 0;

        if (strcmp(drive, "c") == 0 || strcmp(drive, "C") == 0)
        {

            dir = opendir("C:/");
        }
        else if (strcmp(drive, "d") == 0 || strcmp(drive, "D") == 0)
        {

            dir = opendir("D:/");
            index = 1;
        }
        else if (strcmp(drive, "e") == 0 || strcmp(drive, "E") == 0)
        {

            dir = opendir("E:/");
        }
        else
        {
            rt_kprintlnf("Bad drive: %s", drive);
            rt_kprintlnf("LS FAIL");
            return 1;
        }

        if (dir == NULL)
        {
            rt_kprintlnf("LS FAIL");
            return -1;
        }
        rt_kprintlnf("\r\nfile name\tsize");
        while (true)
        {
            info = (struct lfs_info *)readdir(dir);
            if (info == NULL)
            {
                break;
            }
            snprintf(pcWriteBuffer, 256, "%s\t%uB", info->name, info->size);
            rt_kprintlnf("%s", pcWriteBuffer);
        }
        errCode = closedir(dir);
        if (errCode)
        {
            rt_kprintlnf("LS FAIL");
            return errCode;
        }
        snprintf(printfBuf, 256, "Totoal:%dK; Free:%dK", getFsTotalSize(index) / 1024, getFsFreeSize(index) / 1024);
        rt_kprintlnf("%s", printfBuf);
    }
    // storage seek_write c:/test.txt w+ 2 0 123456
    else if (strcmp(sub_cmd, "seek_write") == 0)
    {
        FILE *write_file = NULL;
        char *fileName = argv[2];
        char *mode = argv[3];
        long offset = skip_atoi(&argv[4]);
        int whence = skip_atoi(&argv[5]);
        char *data = argv[6];

        write_file = file_fopen(fileName, mode);
        if (write_file != NULL)
        {
            file_fseek(write_file, offset, whence);
            rt_kprintlnf("file position = %d", file_ftell(write_file));
            file_fwrite(data, strlen(data), 1, write_file);
            file_fclose(write_file);
        }
        else
        {
            rt_kprintlnf("SEEK_WRITE FAIL");
            

            return 1;
        }
    }
    // storage rewind_write c:/test.txt w+ 123456
    else if (strcmp(sub_cmd, "rewind_write") == 0)
    {
        FILE *write_file = NULL;
        char *fileName = argv[2];
        char *mode = argv[3];
        char *data = argv[4];

        write_file = file_fopen(fileName, mode);
        if (write_file != NULL)
        {
            file_rewind(write_file);
            rt_kprintlnf("file position = %d", file_ftell(write_file));
            file_fwrite(data, strlen(data), 1, write_file);
            file_fclose(write_file);
        }
        else
        {
            rt_kprintlnf("REWIND_WRITE FAIL");
            

            return 1;
        }
    }
    // storage fsstat c:/test.txt
    else if (strcmp(sub_cmd, "fsstat") == 0)
    {
        uint8_t size = 0;
        char *fileName = 0;
        FILE *readFile = NULL;
        struct stat buf = {0};

        fileName = argv[2];
        readFile = file_fopen(fileName, "r");
        if (readFile != NULL)
        {
            file_fstat((int)readFile, &buf);
            size = buf.st_size;
            rt_kprintlnf("read size = %d", size);
            file_fclose(readFile);
        }
        else
        {
            rt_kprintlnf("FSSTAT FAIL");
            

            return 1;
        }
    }
    // storage truncate c:/test.txt r+ 2
    else if (strcmp(sub_cmd, "truncate") == 0)
    {
        char *fileName = argv[2];
        char *mode = argv[3];
        FILE *truncateFile = NULL;
        long length = skip_atoi(&argv[4]);

        truncateFile = file_fopen(fileName, mode);
        if (truncateFile != NULL)
        {
            file_truncate((int)truncateFile, length);
            file_fclose(truncateFile);
        }
        else
        {
            rt_kprintlnf("TRUNCATE FAIL");
            

            return 1;
        }
    }
    // storage format d
    else if (strcmp(sub_cmd, "format") == 0)
    {
        char *drive = argv[2];

        int err = LFSEX_format();
        if (err != 0)
        {
            rt_kprintlnf("FORMAT FAIL");
            return 1;
        }
        err = LFSEX_init();
        if (err != 0)
        {
            rt_kprintlnf("INIT FAIL");
            return 1;
        }
    }

#if (defined(FEATURE_SUBSYS_JSONDB_TEST_ENABLE) && defined(FEATURE_SUBSYS_JSONDB_ENABLE))
    // storage jsondb_create
    else if (strcmp(sub_cmd, "jsondb_create") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        int ret = 0;
        ret = remove(fileName);
        // if (ret != 0)
        // {
        //     rt_kprintlnf("Error: rm test json file fail");
        // }
        ret = remove(typeFileName);
        // if (ret != 0)
        // {
        //     rt_kprintlnf("Error: rm test type file fail");
        // }
        FILE *write_file = NULL;
        char *mode = "w";
        char *data = "{\"name\":\"json_db_test\",\"nextLevel\":[]}";
        write_file = file_fopen(fileName, mode);
        if (write_file != NULL)
        {
            file_fwrite(data, strlen(data), 1, write_file);
            file_fclose(write_file);
        }
        else
        {
            rt_kprintlnf("Error: write test json file fail");
            return 1;
        }
        data = "name:char:40\r\nnumber:char:40";
        write_file = file_fopen(typeFileName, mode);
        if (write_file != NULL)
        {
            file_fwrite(data, strlen(data), 1, write_file);
            file_fclose(write_file);
        }
        else
        {
            rt_kprintlnf("Error: write test type file fail");
            return 1;
        }
    }
    // storage jsondb_add
    else if (strcmp(sub_cmd, "jsondb_add") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        char contact0[][40] = {"张三", "123"};
        char contact1[][40] = {"李四", "456"};
        char contact2[][40] = {"王五", "789"};
        jsonDbAddItem(fileName, typeFileName, contact0);
        jsonDbAddItem(fileName, typeFileName, contact1);
        jsonDbAddItem(fileName, typeFileName, contact2);
    }
    // storage jsondb_update
    else if (strcmp(sub_cmd, "jsondb_update") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        char contactNew[][40] = {"李23四", "654"};
        jsonDbUpdateItem(fileName, typeFileName, 1, contactNew);
    }
    // storage jsondb_delete
    else if (strcmp(sub_cmd, "jsondb_delete") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        jsonDbDeleteItem(fileName, typeFileName, 1);
    }
    // storage jsondb_get_json
    else if (strcmp(sub_cmd, "jsondb_get_json") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        cJSON *root = jsonGetRoot(fileName);
        if (root == NULL)
        {
            rt_kprintlnf("Error: failed to read json db");
            return 1;
        }

        char *jsonStr = cJSON_Print(root);
        if (jsonStr == NULL)
        {
            rt_kprintlnf("Error: failed to get json str");
            free(root);
            return 1;
        }
        else
        {
            rt_kprintlnf(jsonStr);
        }
        cJSON_Delete(root);
        root = NULL;
        cJSON_free(jsonStr);
        jsonStr = NULL;
    }
    // storage jsondb_get_type
    else if (strcmp(sub_cmd, "jsondb_get_type") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        uint32_t length = 0;
        char *name = NULL;
        uint32_t count = jsonDbGetTypeNumber(typeFileName);
        for (uint32_t i = 0; i < count; i++)
        {
            length = jsonDbGetTypeName(typeFileName, i, &name);
            if (name != NULL)
            {
                rt_kprintlnf("%s: %d", name, length);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(name);
#else
                free(name);
#endif
                name = NULL;
            }
        }
    }
    // storage jsondb_get_total_count
    else if (strcmp(sub_cmd, "jsondb_get_total_count") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        int32_t count = jsonDbGetTotalCount(fileName, typeFileName);
        rt_kprintlnf("%d", count);
    }
    // storage jsondb_get_all_items
    else if (strcmp(sub_cmd, "jsondb_get_all_items") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        JsonDbTestT *items = NULL;
        int32_t count = jsonDbGetAllItems(fileName, typeFileName, &items);
        for (int i = 0; i < count; i++)
        {
            rt_kprintlnf("%d %s %s", i, items[i].name, items[i].number);
        }
    }
    // storage jsondb_get_item_by_id 1
    else if (strcmp(sub_cmd, "jsondb_get_item_by_id") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        uint32_t id = skip_atoi(&argv[2]);
        JsonDbTestT *items = NULL;
        jsonDbGetItemById(fileName, typeFileName, id, (void **)&items);
        rt_kprintlnf("%s %s", items->name, items->number);
    }
    // storage jsondb_match xx
    else if (strcmp(sub_cmd, "jsondb_match") == 0)
    {
        char *fileName = "D:/json_db_test.json";
        char *typeFileName = "D:/json_db_test.type";
        char *data = argv[2];
        JsonDbTestT *items = NULL;
        uint32_t *ids = NULL;
        uint32_t count = jsonDbGetMatchedItemsByStr(fileName, typeFileName, data, (void **)&items, &ids);
        if ((count != 0) && (items != NULL) && (ids != NULL))
        {
            for (uint32_t i = 0; i < count; i++)
            {
                rt_kprintlnf("%s: %s (%d)", items[i].name, items[i].number, ids[i]);
            }
        }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(items);
#else
        free(items);
#endif
        items = NULL;
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(ids);
#else
        free(ids);
#endif
        ids = NULL;
    }
#endif // FEATURE_SUBSYS_JSONDB_TEST_ENABLE
    else
    {
        goto _usage;
    }
    
    return 0;

_usage:
    rt_kprintf("Usage: storage [options]\r\n");
    rt_kprintf("[options]:\r\n");
    rt_kprintf("    %-31s - write file\r\n", "write");
    rt_kprintf("    %-31s - read file\r\n", "read");
    rt_kprintf("    %-31s - remove file\r\n", "rm");
    rt_kprintf("    %-31s - list current drive files\r\n", "ls");
    rt_kprintf("    %-31s - set file read or write position\r\n", "seek_write");
    rt_kprintf("    %-31s - set file read or write position to head\r\n", "rewind_write");
    rt_kprintf("    %-31s - get file status\r\n", "fsstat");
    rt_kprintf("    %-31s - truncate file to specific length\r\n", "truncate");
    rt_kprintf("    %-31s - format flash\r\n", "format");
#ifdef FEATURE_SUBSYS_JSONDB_TEST_ENABLE
    rt_kprintf("    %-31s - jsondb_create\r\n", "jsondb_create");
    rt_kprintf("    %-31s - jsondb_add\r\n", "jsondb_add");
    rt_kprintf("    %-31s - jsondb_update\r\n", "jsondb_update");
    rt_kprintf("    %-31s - jsondb_delete\r\n", "jsondb_delete");
    rt_kprintf("    %-31s - jsondb_get_json\r\n", "jsondb_get_json");
    rt_kprintf("    %-31s - jsondb_get_type\r\n", "jsondb_get_type");
    rt_kprintf("    %-31s - jsondb_match\r\n", "jsondb_match");
    rt_kprintf("    %-31s - jsondb_get_total_count\r\n", "jsondb_get_total_count");
    rt_kprintf("    %-31s - jsondb_get_item_by_id\r\n", "jsondb_get_item_by_id");
    rt_kprintf("    %-31s - jsondb_get_all_items\r\n", "jsondb_get_all_items");
#endif // FEATURE_SUBSYS_JSONDB_TEST_ENABLE
    
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_storage, storage, storage test);

#endif
#endif
