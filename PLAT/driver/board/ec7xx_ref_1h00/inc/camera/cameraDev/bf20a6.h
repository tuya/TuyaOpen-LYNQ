#ifndef  BF20A6_H
#define  BF20A6_H

#define BF20A6_I2C_ADDR 0x6E

uint16_t bf20a6GetRegCnt(char* regName);
int bf20a6SetMirrorFlip(uint8_t direct);
int bf20a6SetFps(uint32_t fps);
int bf20a6GetSensorID(uint32_t *id);
int bf20a6SetEv(uint8_t ev);
int bf20a6SetContrast(uint8_t contrast);
int bf20a6SetSaturation(uint8_t sat);
int bf20a6SetSharp(uint8_t sharp);
int bf20a6SetAwb(bool awbEnable, uint8_t scene);
int bf20a6SetGamma(uint8_t *table, uint32_t size);
int bf20a6SetAE(uint8_t aeMode);
int bf20a6SetScene(uint8_t scene);
#endif

