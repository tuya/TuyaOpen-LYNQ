#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "packet.h"

static const unsigned int gTinfCrc32Tab[16] = {
   0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190,
   0x6b6b51f4, 0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344,
   0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278,
   0xbdbdf21c
};

/* crc is previous value for incremental computation, 0xffffffff initially */
uint32_t uzlibCrc32(const void *data, unsigned int length, uint32_t crc)
{
   const unsigned char *buf = (const unsigned char *)data;
   unsigned int i;

   for (i = 0; i < length; ++i)
   {
      crc ^= buf[i];
      crc = gTinfCrc32Tab[crc & 0x0f] ^ (crc >> 4);
      crc = gTinfCrc32Tab[crc & 0x0f] ^ (crc >> 4);
   }

   // return value suitable for passing in next time, for final value invert it
   return crc/* ^ 0xffffffff*/;
}

uint32_t packageEncode(uint8_t *scr,uint8_t *buf,uint32_t lens)
{
	uint32_t dataCrc;
	uint32_t headerCrc;
	uint8_t *dataBuf;
	PacketHdrT *phdr;
	
	phdr = (PacketHdrT *)buf;
	dataBuf = buf+sizeof(PacketHdrT);
	
	//data crc
	dataCrc = uzlibCrc32(scr, lens,0xFFFFFFFF);
	
	//header fill
	phdr->packetFlag = PAK_FLAG;
	phdr->packetDataSize = lens;
	phdr->packetReserve = 0xFFFF;
	phdr->dataCrc = dataCrc ^ 0xFFFFFFFF;
	
	//header crc
	headerCrc = uzlibCrc32(phdr, sizeof(PacketHdrT)-4,0xFFFFFFFF);
	phdr->headerCrc = headerCrc ^ 0xFFFFFFFF;
	
	//data fill
	memcpy(dataBuf,scr,lens);
	
	return 0;
}

uint32_t packageDecode(uint8_t *scr,uint8_t *buf)
{
	uint32_t dataCrc=0;
	uint32_t headerCrc=0;
	uint32_t orgDataCrc=0;
	uint32_t orgHeaderCrc=0;
	uint32_t dataLens=0;
	
	PacketHdrT *phdr=0;
	uint8_t *dataBuf=0;

	phdr = (PacketHdrT *)scr;
	dataBuf = scr+sizeof(PacketHdrT);
	dataLens = phdr->packetDataSize;
	
	if( phdr->packetFlag != PAK_FLAG)
		return -1;
	
	if(phdr->packetDataSize>MAX_PAK_SIZE)
		return -2;
	
	headerCrc = uzlibCrc32(phdr, sizeof(PacketHdrT)-4,0xFFFFFFFF);
	orgHeaderCrc = phdr->headerCrc;
	if((headerCrc^0xFFFFFFFF) != orgHeaderCrc)
		return -3;
	
	dataCrc = uzlibCrc32(dataBuf, dataLens,0xFFFFFFFF);
	orgDataCrc = phdr->dataCrc;
	if((dataCrc^0xFFFFFFFF) != orgDataCrc)
		return -4;
	
	memcpy(dataBuf,buf,dataLens);
	
	return dataLens;
}

