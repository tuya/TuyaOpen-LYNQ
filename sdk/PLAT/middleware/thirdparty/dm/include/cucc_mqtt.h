/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    cucc_mqtt.h
 * Description:  EC618  entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/

#define CUCC_DM_MQTT_CLIENTID_LEN       100
#define CUCC_DM_MQTT_USERNAME_LEN       100
#define CUCC_DM_MQTT_PASSWORD_LEN       100
#define CUCC_DM_MQTT_DEVICE_SECRET_LEN  100

#define CUCC_DM_MQTT_RECV_DEVICE_SECRET_LEN  166

int mqttCuccNoPreIdentity(const char *productKey, const char *InDeviceId, const char *deviceKey, char *clientId, char *username);
int mqttCuccDynNoPreRegIdentity(const char *productKey, const char *InDeviceId, const char *deviceKey, const char *productSecret, char *clientId, char *username, char *password);
int mqttCuccIdentity(const char *productKey, const char *deviceKey, const char *deviceSecret, char *clientId, char *username, char *password);
int mqttCuccDynRegIdentity(const char *productKey, const char *deviceKey, const char *productSecret, char *clientId, char *username, char *password);


