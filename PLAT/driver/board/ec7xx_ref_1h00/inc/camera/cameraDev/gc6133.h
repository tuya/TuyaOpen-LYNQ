#ifndef  GC6133_H
#define  GC6133_H

#define GC6133_I2C_ADDR 0x40

uint16_t gc6133GetRegCnt(char* regName);
int gc6133SetMirrorFlip(uint8_t direct);
int gc6133SetFps(uint32_t fps);
int gc6133GetSensorID(uint32_t *id);
int gc6133SetEv(uint8_t ev);
int gc6133SetContrast(uint8_t contrast);
int gc6133SetSaturation(uint8_t sat);
int gc6133SetSharp(uint8_t sharp);
int gc6133SetAwb(bool awbEnable, uint8_t scene);
int gc6133SetGamma(uint8_t *table, uint32_t size);
int gc6133SetAE(uint8_t aeMode);
int gc6133SetScene(uint8_t scene);
#endif

