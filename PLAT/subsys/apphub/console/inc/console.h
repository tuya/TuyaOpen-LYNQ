
#ifndef __CONSOLE_H__
#define __CONSOLE_H__
#ifdef __cplusplus
extern "C" {
#endif
#define TASK_SIZE_CONSOLE               (5 * 1024)

typedef enum console_event_bits {
    TASK_MSH = ( 1 << 0 ),
    TASK_PIKA = ( 1 << 1 ),
    TASK_FILE = ( 1 << 2 ),
    TASK_EXIT = ( 1 << 3 )
} ConsoleEventBitsT;

enum console_mode_t{
    C_MODE_MSH = 0,
    C_MODE_PY ,
    C_MODE_FILE 
};
void consoleTaskInit(void);
void consoleTaskDeinit(void);
void consoleMshInit(void);
uint8_t getConsoleMode(void);
void sendConsoleCmd(char *msg, uint32_t timeout);
#ifdef __cplusplus
}
#endif
#endif
