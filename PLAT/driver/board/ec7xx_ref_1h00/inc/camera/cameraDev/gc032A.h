#ifndef  GC032A_H
#define  GC032A_H

#define GC032A_I2C_ADDR 0x21

uint16_t gc032aGetRegCnt(char* regName);
int gc032aSetMirrorFlip(uint8_t direct);
int gc032aSetFps(uint32_t fps);
int gc032aGetSensorID(uint32_t *id);
int gc032aSetEv(uint8_t ev);
int gc032aSetContrast(uint8_t contrast);
int gc032aSetSaturation(uint8_t sat);
int gc032aSetSharp(uint8_t sharp);
int gc032aSetAwb(bool awbEnable, uint8_t scene);
int gc032aSetGamma(uint8_t *table, uint32_t size);
int gc032aSetAE(uint8_t aeMode);
int gc032aSetScene(uint8_t scene);
#endif

