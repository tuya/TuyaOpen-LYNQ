#ifndef __PHONE_H__
#define __PHONE_H__


#ifndef MIN
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#endif

#define PHONE_CONFIG_FILE                                   "D:/phoneConfig.ini"
#define PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT     "contactSettingStorageSelect"

#define CONTACT_NAME_LENGTH                                 60
#define CONTACT_NUMBER_LENGTH                               20


typedef enum
{
    PHONE_STATUS_OK    = 0,
    PHONE_STATUS_ERROR = -1,
} PhoneStatusT;

typedef enum 
{
    PHONE_LOCATION_PHONE = 0,
    PHONE_LOCATION_SIM1,
    PHONE_LOCATION_SIM2,
} PhoneLocationT;

typedef enum
{
    PHONE_CALL_TYPE_INCOMMING = 0,
    PHONE_CALL_TYPE_DIALLING,
    PHONE_CALL_TYPE_INCOMMING_UNANSWERED,
    PHONE_CALL_TYPE_DIALLING_UNANSWERED,
} PhoneCallTypeT;

typedef enum
{
    PHONE_CALL_STATE_RING = 0,
    PHONE_CALL_STATE_ATA,
    PHONE_CALL_STATE_ATD,
    PHONE_CALL_STATE_COLP,
} PhoneCallStateT;

typedef enum
{
    PHONE_SIM_1 = 0,
    PHONE_SIM_2,
} PhoneSimT;


#endif
