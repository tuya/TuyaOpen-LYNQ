#ifndef __OL_MQTT_API__
#define __OL_MQTT_API__

#ifdef MBTK_OPENCPU_SUPPORT_MQTT

typedef enum
{
    OL_MQTT_SSL_NONE = 0,
    OL_MQTT_SSL_HAVE = 1,
} OL_MQTT_SSL_TYPE;

typedef enum
{
    OL_MQTT_CONNECT_SUCCESS_EVENT = 0,
    OL_MQTT_DISCONNECT_SUCCESS_EVENT = 1,
    OL_MQTT_CONNECT_ABORT_EVENT = 2,
    OL_MQTT_START_RECONNECT_EVENT = 3,
    OL_MQTT_RECONNECT_FAIL_EVENT = 4,
} OL_Mqtt_Connection_Event;

typedef enum
{
    OL_MQTT_RECONNECT_ENABLE = 0,
    OL_MQTT_RECONNECT_DISABLE = 1,
} OL_Mqtt_Reconnect_FLAG;


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set mqtt host, set is a point, if is malloc string,
 *      can not be free until finish
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    host        [IN]        hostname
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_host(MQTTClient* c, char* host);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set mqtt port
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    port        [IN]        port
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_port(MQTTClient* c, unsigned int port);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set mqtt ca crt if use ssl connect need ca crt
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    ca          [IN]        buffer hold ca crt
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_ca(MQTTClient* c, char* ca);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set mqtt client key if use ssl connect need client key
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    cli_key     [IN]        buffer hold client key
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_cli_key(MQTTClient* c, char* cli_key);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set mqtt client crt if use ssl connect need client crt
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    cli_crt     [IN]        buffer hold client crt
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_cli_crt(MQTTClient* c, char* cli_crt);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set keep alive interval
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    interval    [IN]        interval
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_keep_alive_interval(MQTTClient* c, uint16_t keepalive_interval);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set whether it is mqtt ssl connect
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    is_mqtts    [IN]        OL_MQTT_SSL_TYPE
 *    seclevel    [IN]        //0:no verify; 1:verify server; 2:both verify
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_is_mqtts(MQTTClient* c, OL_MQTT_SSL_TYPE is_mqtts, int seclevel);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set whether it is mqtt ssl connect
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    option      [IN]        the point to the struct MQTTPacket_connectData
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_set_connect_data(MQTTClient* c, MQTTPacket_connectData* option);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to connect to mqtt server, this command is block
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *    option      [IN]        the point to the struct MQTTPacket_connectData
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_connect(MQTTClient* c, MQTTPacket_connectData* options);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to disconnect to the mqtt server
 *
 * PARAMETERS
 *    MQTTClient  [IN]        mqtt client
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_disconnect(MQTTClient* c);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to subscribe topic
 *
 * PARAMETERS
 *    MQTTClient          [IN]        mqtt client
 *    topic_filter        [IN]        topic_filter string
 *    qos                 [IN]        qos   [0,1,2]
 *    message_handler_t   [IN]        message handler
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_subscribe(MQTTClient* c, const char* topic_filter, enum QoS qos, messageHandler messageHandler);

/*****************************************************************************
 * DESCRIPTION
 *    This API is to unsubscribe topic
 *
 * PARAMETERS
 *    MQTTClient      [IN]        mqtt client
 *    topic_filter    [IN]        topic_filter string
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_unsubscribe(MQTTClient* c, const char* topic_filter);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to public message
 *
 * PARAMETERS
 *    MQTTClient      [IN]        mqtt client
 *    topic           [IN]        topic string
 *    msg             [IN]         publish message,point to the struct MQTTMessage
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_publish(MQTTClient* c, const char* topic, MQTTMessage* msg);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to set connect event callback handler
 *
 * PARAMETERS
 *    MQTTClient         [IN]        mqtt client
 *    conn_handle        [IN]        the callback to handle connect event
 *    reconn_enbale      [IN]        enable to reconnect
 *
 * RETURN
 *    =0 : success
 *    <0 : error
 *****************************************************************************/
extern int ol_mqtt_register_conn_callback(MQTTClient* c, connect_event_cb_t conn_callback, OL_Mqtt_Reconnect_FLAG reconn_enbale);


#endif //MBTK_OPENCPU_SUPPORT_MQTT
#endif //__OL_MQTT_API__
