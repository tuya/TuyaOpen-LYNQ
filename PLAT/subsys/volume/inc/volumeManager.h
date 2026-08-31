#ifndef __VOLUME_MANAGER_H__
#define __VOLUME_MANAGER_H__


#include <stdint.h>
#include <stdbool.h>


void    volumeManagerInit(void);
int32_t volumeManagerGet(char *name, uint8_t *volume);
int32_t volumeManagerSet(char *name, uint8_t volume);
int32_t volumeManagerMute(char *name);
int32_t volumeManagerUnmute(char *name);
int32_t volumeManagerIsMute(char *name, bool *isMute);
int32_t volumeManagerActivate(char *name);


#endif
