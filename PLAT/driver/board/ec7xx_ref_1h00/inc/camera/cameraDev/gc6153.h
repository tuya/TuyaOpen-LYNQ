#ifndef  GC6153_H
#define  GC6153_H

#define GC6153_I2C_ADDR 0x40

uint16_t gc6153GetRegCnt(char* regName);
int gc6153SetMirrorFlip(uint8_t direct);
int gc6153SetFps(uint32_t fps);
int gc6153GetSensorID(uint32_t *id);
int gc6153SetEv(uint8_t ev);
int gc6153SetContrast(uint8_t contrast);
int gc6153SetSaturation(uint8_t sat);
int gc6153SetSharp(uint8_t sharp);
int gc6153SetAwb(bool awbEnable, uint8_t scene);
int gc6153SetGamma(uint8_t *table, uint32_t size);
int gc6153SetAE(uint8_t aeMode);
int gc6153SetScene(uint8_t scene);
#endif

