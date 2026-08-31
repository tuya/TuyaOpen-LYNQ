/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    packet.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_INPUT_PACKET_H
#define  SUBSYS_INPUT_PACKET_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAK_DATA_SIZE	1000
#define MAX_PAK_SIZE	MAX_PAK_DATA_SIZE+sizeof(PacketHdrT)
#define PAK_FLAG		0xEEEEEEEE

typedef struct _packet_hdr_t
{
	uint32_t packetFlag;
	uint16_t packetDataSize;
	uint16_t packetReserve;
	uint32_t dataCrc;
	uint32_t headerCrc;
} PacketHdrT;

typedef struct _packet_t
{
	PacketHdrT packetHdr;
	uint32_t* packetData;
} PacketT;

typedef struct _packet_ack_t
{
	uint32_t packetCounter;
	uint32_t packetResult;
} PacketAckT;

typedef struct _file_packet_hdr_t
{
	uint32_t fileSize;
	char fileName[128];
	uint32_t packetSize;
	uint32_t fileCounter;
} FilePacketHdrT;

typedef struct _file_packet_data_t
{
	uint32_t packetCounter;
	uint8_t packetData[MAX_PAK_SIZE];
} FilePacketDataT;

uint32_t packageEncode(uint8_t *scr,uint8_t *buf,uint32_t lens);
uint32_t packageDecode(uint8_t *scr,uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_INPUT_PACKET_H */