
#ifndef __CONSOLE_FILE_H__
#define __CONSOLE_FILE_H__
#ifdef __cplusplus
extern "C" {
#endif

#define TASK_SIZE_CONSOLE_FILE          (10*1024)
#define CONSOLE_FILE_SIZE               (4*1024)
#define FILE_MARK                       "#!python"
#define TAIL_MARK                       "#!pika"

#define TASK_PRIO_CONSOLE_FILE          osPriorityNormal

enum console_file_dir{
    DISK_C = 0,
    DISK_D ,
    DISK_E , 
    DISK_NONE 
};

uint8_t getConsoleDir(void);
void setConsoleDir(uint8_t disk);
void runPythonByName(char *name);
int consoleFileFilter(char *str, char *name);
int sendConsoleFile(char *src, uint32_t timeout);
#ifdef __cplusplus
}
#endif
#endif
