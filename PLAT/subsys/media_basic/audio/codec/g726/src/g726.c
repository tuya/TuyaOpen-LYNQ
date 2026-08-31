#include <stdio.h>
#include <stdlib.h>
#include "g726codec.h"

/* (zqj)
 * function: initialize the g726 info struct.
 * 
 * check return:
 *      g_state726_xx on success, and if failed, NULL is returned.
 */
g726_state_t *G726_Init( g726_state_t *gs, int bit )
{
	if( (bit != 2) && (bit != 3) && (bit != 4) && (bit != 5) )
	{
		printf("encode to unsupported audio format, exit !\n");	
		return NULL;
	}

	switch( bit )
	{
		case 2:
			gs = (g726_state_t *)malloc( sizeof(g726_state_t) );
			gs = g726_init( gs, 8000*2 );
			break;

		case 3:
			printf("g726_24 init start......\n");
			gs = (g726_state_t *)malloc(sizeof(g726_state_t));
			gs = g726_init( gs, 8000*3 );
			break;

		case 4:
			printf("g726_32 init start......\n");
			gs = (g726_state_t *)malloc(sizeof(g726_state_t));
			gs = g726_init( gs, 8000*4 );
			break;

		case 5:
			printf("g726_40 init start......\n");
			gs = (g726_state_t *)malloc(sizeof(g726_state_t));
			gs = g726_init( gs, 8000*5 );
			break;

		default:
			break;
	}
	return gs;
}


/*
 * function: convert PCM to g726 16kbps audio format.
 *   bit:			index g726 ==> 16/24/32/40 kbps.
 *   gsenc:			g726 struct for encoder.
 *	 InAudioBuf:	PCM audio format data.
 *	 OutAudioBuf:	g726 16kbps.
 *	 DataLen:		the size of PCM data.
 *	 reserver:		reversed param, no use.
 * return: -1 --> failed.
 *		   >0 --> success.
 */
int PCM2G726(int bit, g726_state_t *gsenc, char *InAudioBuf, char *OutAudioBuf, int DataLen, int reserve)
{
	if( NULL == gsenc )
	{
		printf("empty g726 struct info, g726 init failed or error !, %s, %d\n", __func__, __LINE__);	
		return -1;
	}
	if( (NULL == InAudioBuf) && (NULL == OutAudioBuf) && (0 == DataLen) )
	{
		printf("empty data or transmit failed, exit !\n");	
		return -1;
	}
	if( (bit != 2) && (bit != 3) && (bit != 4) && (bit != 5) )
	{
		printf("encode to unsupported audio format, exit !\n");	
		return -1;
	}
	// printf("bit = %d, DataLen = %d, %s, %d\n", bit, DataLen, __func__, __LINE__);

	int Retenc = 0;
	switch( bit )
	{
		case 2:
			// printf("g726_16 encode start......\n");
			Retenc = g726_encode(gsenc, (unsigned char *)OutAudioBuf, (short*)InAudioBuf, DataLen/2);
			// printf("Retenc = %d, %s, %d\n", Retenc, __func__, __LINE__);
			break;

		case 3:
			printf("g726_24 encode start......\n");
			Retenc = g726_encode(gsenc, (unsigned char *)OutAudioBuf, (short*)InAudioBuf, DataLen/2);
			printf("Retenc = %d, %s, %d\n", Retenc, __func__, __LINE__);
			break;

		case 4:
			printf("g726_32 encode start......\n");
			Retenc = g726_encode(gsenc, (unsigned char *)OutAudioBuf, (short*)InAudioBuf, DataLen/2);
			printf("Retenc = %d, %s, %d\n", Retenc, __func__, __LINE__);
			break;

		case 5:
			printf("g726_40 encode start......\n");
			Retenc = g726_encode(gsenc, (unsigned char *)OutAudioBuf, (short*)InAudioBuf, DataLen/2);
			printf("Retenc = %d, %s, %d\n", Retenc, __func__, __LINE__);
			break;

		default:
			break;
	}

	return Retenc;	
}


/*
 * function: convert g726(16/24/32/40 kbps) to PCM audio format.
 *   bit:			index g726 ==> 16/24/32/40 kbps.
 *   gsdec:			g726 struct for decoder.
 *	 InAudioBuf:	PCM audio format data.
 *	 OutAudioBuf:	g726 16kbps.
 *	 DataLen:		the size of PCM data.
 *	 reserver:		reversed param, no use.
 * return: -1 --> failed.
 *		   >0 --> success.
 */
int G726_2PCM(int bit, g726_state_t *gsdec, char *InAudioBuf, char *OutAudioBuf, int DataLen, int reserve)
{
	if( NULL == gsdec )
	{
		printf("empty g726 struct info, or g726 decoder init failed !, %s, %d\n", __func__, __LINE__);	
		return -1;
	}
	if( (NULL == InAudioBuf) && (NULL == OutAudioBuf) && (0 == DataLen) )
	{
		printf("empty data or transmit failed, exit !\n");	
		return -1;
	}
	if( (bit != 2) && (bit != 3) && (bit != 4) && (bit != 5) )
	{
		printf("encode to unsupported audio format, exit !\n");	
		return -1;
	}
	//printf("bit = %d, DataLen = %d, %s, %d\n", bit, DataLen, __func__, __LINE__);

	int Retdec = 0;
	switch( bit )
	{
		case 2:
			// printf("g726_16 decode start......\n");
			Retdec = g726_decode( gsdec, (short*)OutAudioBuf, (unsigned char *)InAudioBuf, DataLen );
			// printf("Retdec = %d, %s, %d\n", Retdec, __func__, __LINE__);
			break;

		case 3:
			printf("g726_24 decode start......\n");
			Retdec = g726_decode( gsdec, (short*)OutAudioBuf, (unsigned char *)InAudioBuf, DataLen );
			printf("Retdec = %d, %s, %d\n", Retdec, __func__, __LINE__);
			break;

		case 4:
			printf("g726_32 decode start......\n");
			Retdec = g726_decode( gsdec, (short*)OutAudioBuf, (unsigned char *)InAudioBuf, DataLen );
			printf("Retdec = %d, %s, %d\n", Retdec, __func__, __LINE__);
			break;

		case 5:
			printf("g726_40 decode start......\n");
			Retdec = g726_decode( gsdec, (short*)OutAudioBuf, (unsigned char *)InAudioBuf, DataLen );
			printf("Retdec = %d, %s, %d\n", Retdec, __func__, __LINE__);
			break;

		default:
			break;
	}

	return Retdec;
}
