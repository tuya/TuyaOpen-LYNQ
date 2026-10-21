/*******************************************************************************
 *
 * Copyright (c) 2014 Bosch Software Innovations GmbH, Germany.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Bosch Software Innovations GmbH - Please refer to git log
 *
 *******************************************************************************/
/*
 * lwm2mclient.h
 *
 *  General functions of lwm2m test client.
 *
 *  Created on: 22.01.2015
 *  Author: Achim Kraus
 *  Copyright (c) 2015 Bosch Software Innovations GmbH, Germany. All rights reserved.
 */

#ifndef CT_LWM2MCLIENT_H_
#define CT_LWM2MCLIENT_H_

#include "liblwm2m.h"

/*
 * object_device.c
 */
lwm2m_object_t * ct_get_object_device(void);
void ct_free_object_device(lwm2m_object_t * objectP);
uint8_t ct_device_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
void ct_display_device_object(lwm2m_object_t * objectP);
/*
 * object_firmware.c
 */

#ifdef WITH_FOTA
lwm2m_object_t * ct_get_object_firmware(void);
void ct_free_object_firmware(lwm2m_object_t * objectP);
void ct_display_firmware_object(lwm2m_object_t * objectP);
void report_exec_fota_result(void);
#endif

/*
 * object_server.c
 */
lwm2m_object_t * ct_get_server_object(int serverId, const char* binding, int lifetime, bool storing);
void ct_clean_server_object(lwm2m_object_t * object);
void display_server_object(lwm2m_object_t * objectP);
void copy_server_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_connectivity_moni.c
 */
lwm2m_object_t * ct_get_object_conn_m(void);
void ct_free_object_conn_m(lwm2m_object_t * objectP);
uint8_t ct_connectivity_moni_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);

/*
 * object_security.c
 */
lwm2m_object_t * get_security_object(int serverId, const char* serverUri, char * bsPskId, char * psk, uint16_t pskLen, bool isBootstrap);
//lwm2m_object_t * ct_get_security_object(int serverId, const char* serverUri, char * bsPskId, char * psk, uint16_t pskLen, bool isBootstrap);
void clean_security_object(lwm2m_object_t * objectP);
char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
void display_security_object(lwm2m_object_t * objectP);
void copy_security_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 *  object_data_report.c
*/
lwm2m_object_t * get_data_report_object(void);
void free_data_report_object(lwm2m_object_t * object);

#endif /* LWM2MCLIENT_H_ */
