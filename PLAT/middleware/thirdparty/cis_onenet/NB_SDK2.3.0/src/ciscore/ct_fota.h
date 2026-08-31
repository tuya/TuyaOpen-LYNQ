#ifndef _CT_FOTA_H
#define _CT_FOTA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "cis_internals.h"

#define START_MORE 0
#define BLOCK_SIZE 512


typedef struct
{
    char protocol[6];
    char address[51];
    int port;
    char uri[201];
}CT_URI;

int ct_fota_start(st_context_t * contextP, char* url);

void ct_fota_state_changed(st_context_t * contextP, uint8_t state);
void ct_fota_handle_result(st_context_t * contextP);

#endif

