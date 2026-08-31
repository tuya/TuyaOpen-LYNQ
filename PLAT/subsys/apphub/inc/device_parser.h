#ifndef __DEVICE_PARSER_H__
#define __DEVICE_PARSER_H__ 
#include <stdint.h>

#define DEV_CFG_PATH "D:/device-config.json"

int device_parser_parse(const char* dev_path, void** cfg, uint32_t* size, void* def_cfg);
int device_parser_free(void* cfg);

#endif