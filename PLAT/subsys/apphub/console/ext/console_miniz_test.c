/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_ril_test.c
 * Description:  EC718
 * History:      Rev1.0   2025-08-06
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "miniz.h"
#include "rtthread.h"
#include "string.h"
#include "storage.h"

extern int skip_atoi(const char **s);

char *remove_drive_prefix_strcmp(char *path)
{
    if(strlen(path) >= 3 && isalpha(path[0]) && path[1] == ':' &&
       (path[2] == '/' || path[2] == '\\'))
    {
        return path + 3;
    }
    return path;
}

int add_file_to_zip(mz_zip_archive *zip_archive, const char *filename,
                    const char *file_to_add)
{
    // 读取文件内容
    FILE *file = file_fopen(file_to_add, "rb");
    if(!file)
    {
        printf("Failed to open file: %s\n", file_to_add);
        return 0;
    }

    file_fseek(file, 0, SEEK_END);
    size_t file_size = file_ftell(file);
    file_fseek(file, 0, SEEK_SET);

    void *file_data = malloc(file_size);
    if(!file_data)
    {
        file_fclose(file);
        printf("Memory allocation failed\n");
        return 0;
    }

    size_t read_size = file_fread(file_data, 1, file_size, file);

    file_fclose(file);

    // 添加文件到 ZIP
    mz_bool status = mz_zip_writer_add_mem(zip_archive,
                                           filename,   // ZIP 中的文件名
                                           file_data,  // 文件数据
                                           file_size,  // 数据大小
                                           MZ_BEST_COMPRESSION  // 压缩级别
    );

    free(file_data);
    return status;
}

static void create_directory(const char *path)
{
    // 复制原始路径到临时缓冲区
    char *temp = strdup(path);
    if(!temp)
    {
        printf("strdup fail\r\n");
        return -1;
    }

    // 分割路径为token数组
    char *tokens[20];
    int num_tokens = 0;
    char *token = strtok(temp, "/");
    while(token && num_tokens < 20)
    {
        tokens[num_tokens++] = token;
        token = strtok(NULL, "/");
    }

    // 逐步构建并输出路径
    if(num_tokens > 0)
    {
        char buffer[1024];  // 足够大的缓冲区
        strcpy(buffer, tokens[0]);
        for(int i = 1; i < num_tokens; ++i)
        {
            strcat(buffer, "/");
            strcat(buffer, tokens[i]);
            int ret = mkdir(buffer, 0777);
        }
    }
    free(temp);
}

int extract_file(mz_zip_archive *zip_archive, mz_uint file_index,
                 const char *output_dir)
{
    mz_zip_archive_file_stat file_stat;
    if(!mz_zip_reader_file_stat(zip_archive, file_index, &file_stat))
    {
        printf("Failed to get file info for index %u\n", file_index);
        return 1;
    }

    if(mz_zip_reader_is_file_a_directory(zip_archive, file_index))
    {
        char dir_path[256];
        snprintf(dir_path, sizeof(dir_path), "%s/%s", output_dir,
                 file_stat.m_filename);
        create_directory(dir_path);
        return 0;
    }

    // 创建完整输出路径
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s", output_dir,
             file_stat.m_filename);

    // 创建父目录
    char *last_slash = strrchr(output_path, '/');
    if(last_slash)
    {
        *last_slash = '\0';
        create_directory(output_path);
        *last_slash = '/';
    }

    size_t uncomp_size;
    void *p =
        mz_zip_reader_extract_to_heap(zip_archive, file_index, &uncomp_size, 0);
    if(!p)
    {
        printf("Failed to extract file: %s\n", file_stat.m_filename);
        return 0;
    }

    FILE *f = file_fopen(output_path, "wb");
    if(!f)
    {
        printf("Failed to create file: %s\n", output_path);
        mz_free(p);
        return 1;
    }

    if(file_fwrite(p, 1, uncomp_size, f) != uncomp_size)
    {
        printf("Failed to write file: %s\n", output_path);
        file_fclose(f);
        mz_free(p);
        return 1;
    }

    file_fclose(f);
    mz_free(p);
    return 0;
}

int cmd_miniz(int argc, char **argv)
{
    char *sub_cmd = argv[1];
    // miniz zip xxx.zip c:/a/a.txt c:/b/b.txt d:/c/c.txt
    if(strcmp(sub_cmd, "zip") == 0)
    {
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        // 初始化 ZIP 写入器
        mz_bool status = mz_zip_writer_init_file(&zip_archive, argv[2], 0);

        if(!status)
        {
            printf("Failed to initialize ZIP writer\n");
            return 1;
        }

        for(int i = 0; i < argc - 3; i++)
        {
            char *filename = remove_drive_prefix_strcmp(argv[i + 3]);
            if(!add_file_to_zip(&zip_archive, filename, argv[i + 3]))
            {
                mz_zip_writer_end(&zip_archive);
                return 1;
            }
        }

        status = mz_zip_writer_finalize_archive(&zip_archive);
        if(!status)
        {
            printf("Failed to finalize ZIP archive\n");
        }

        status = mz_zip_writer_end(&zip_archive);
        if(!status)
        {
            printf("Failed to cleanup ZIP writer\n");
            return 1;
        }

        printf("ZIP archive created successfully\n");
    }
    // miniz unzip xxx.zip c:/a/
    else if(strcmp(sub_cmd, "unzip") == 0)
    {
        const char *zip_filename = argv[2];
        const char *output_dir = argv[3];

        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        mz_bool status = mz_zip_reader_init_file(&zip_archive, zip_filename, 0);
        if(!status)
        {
            printf("Failed to open ZIP file: %s\n", zip_filename);
            return 1;
        }

        create_directory(output_dir);

        mz_uint num_files = mz_zip_reader_get_num_files(&zip_archive);
        for(mz_uint i = 0; i < num_files; i++)
        {
            if(extract_file(&zip_archive, i, output_dir))
            {
                mz_zip_reader_end(&zip_archive);
                return 1;
            }
        }

        status = mz_zip_reader_end(&zip_archive);
        if(!status)
        {
            printf("Failed to close ZIP reader\n");
            return 1;
        }

        printf("Successfully extracted %u files to: %s\n", num_files,
               output_dir);
    }
    else
    {
        rt_kprintlnf("Usage: miniz [options]");
        rt_kprintlnf("[options]:");
        rt_kprintlnf("    %-10s - zip files into a zip archive ", "zip");
        rt_kprintlnf("    %-10s - unzip a zip archive ", "unzip");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_miniz, miniz, miniz test);

#endif  // FEATURE_SUBSYS_CONSOLE_ENABLE