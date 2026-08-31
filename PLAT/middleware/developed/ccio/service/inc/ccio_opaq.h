/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_opaq.h
*
*  Description:
*
*  History: 2022/3/15 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_OPAQ_H
#define CCIO_OPAQ_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_misc.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
typedef enum
{
    OPAQ_CHAN_0 = 0,
    OPAQ_CHAN_UNDEF = OPAQ_CHAN_0,
    OPAQ_CHAN_1,
    OPAQ_CHAN_IPOS = OPAQ_CHAN_1,       /*WAN IP Packet over SPI service */
    OPAQ_CHAN_2,
    OPAQ_CHAN_IPOS_MIFI = OPAQ_CHAN_2,  /*MIFI IP Packet over SPI service */
    OPAQ_CHAN_3,
    OPAQ_CHAN_CUST1 = OPAQ_CHAN_3,      /* rename OPAQ_CHAN_CUST1 to whatever customer wants */
    OPAQ_CHAN_4,
    OPAQ_CHAN_CUST2 = OPAQ_CHAN_4,      /* rename OPAQ_CHAN_CUST2 to whatever customer wants */
    OPAQ_CHAN_5,
    OPAQ_CHAN_CUST3 = OPAQ_CHAN_5,      /* rename OPAQ_CHAN_CUST3 to whatever customer wants */
	OPAQ_CHAN_6,
    OPAQ_CHAN_SPIMIX = OPAQ_CHAN_6,		/*Mix Packet over SPI service */
    OPAQ_CHAN_MAXNUM
}OpaqChanId_e;

typedef void (*opaqServiceCallback)(void *,bool);
typedef struct{
		opaqServiceCallback serHdl;
}opaqServicCtx_t;

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
  \fn     int32_t opaqDataInput(UlPduBlock_t *ulpdu, void *extras)
  \brief  to further handle opaq data received from serial device
  \return
  \note   invoked by RxTask automatically once opaq data is received
*/
int32_t opaqDataInput(UlPduBlock_t *ulpdu, void *extras);

#if (defined(FEATURE_SUBSYS_CONSOLE_ENABLE) && defined(USB_CONSOLE))
void usbReceive(uint8_t *buffer, uint32_t size);
#endif

#if 0
/**
  \fn     int32_t opaqDataOutput(uint8_t chanNo, DlPduBlock_t *dlpdu, void *extras)
  \brief  to send opaq data out of serial device via chanNo
  \return
  \note   invoked by customer's AppTask for outputting opaq data
*/
int32_t opaqDataOutput(uint8_t chanNo, DlPduBlock_t *dlpdu, void *extras);
#endif

/**
  \fn     int32_t opaqDataOutputEx(uint8_t opaqCid, DlPduBlock_t *dlpdu, void *extras)
  \brief  to send opaq data out of serial device via opaqCid
  \return
  \note   invoked by customer's AppTask for outputting opaq data
*/
int32_t opaqDataOutputEx(uint8_t opaqCid, DlPduBlock_t *dlpdu, void *extras);
uint32_t opaqDataServiceRegister(uint8_t opaqCid,opaqServiceCallback callback);

#ifdef __cplusplus
}
#endif
#endif

