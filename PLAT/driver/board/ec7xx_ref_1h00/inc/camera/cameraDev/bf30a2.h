#ifndef  BF30A2_H
#define  BF30A2_H

#define BF30A2_I2C_ADDR 0x6E

uint16_t bf30a2GetRegCnt(char* regName);
int bf30a2SetMirrorFlip(uint8_t direct);
int bf30a2SetFps(uint32_t fps);
int bf30a2GetSensorID(uint32_t *id);
int bf30a2SetEv(uint8_t ev);
int bf30a2SetContrast(uint8_t contrast);
int bf30a2SetSaturation(uint8_t sat);
int bf30a2SetSharp(uint8_t sharp);
int bf30a2SetAwb(bool awbEnable, uint8_t scene);
int bf30a2SetGamma(uint8_t *table, uint32_t size);
int bf30a2SetAE(uint8_t aeMode);
int bf30a2SetScene(uint8_t scene);
#endif
