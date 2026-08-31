#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#include "phoneSms.h"


#define SMS_INBOX_JSON_PATH         "D:/smsInboxDb.json"
#define SMS_INBOX_TYPE_PATH         "D:/smsInboxDb.type"
#define SMS_OUTBOX_JSON_PATH        "D:/smsOutboxDb.json"
#define SMS_OUTBOX_TYPE_PATH        "D:/smsOutboxDb.type"
#define SMS_DRAFT_BOX_JSON_PATH     "D:/smsDraftBoxDb.json"
#define SMS_DRAFT_BOX_TYPE_PATH     "D:/smsDraftBoxDb.type"
#define SMS_COMMON_BOX_JSON_PATH    "D:/smsCommonBoxDb.json"
#define SMS_COMMOM_BOX_TYPE_PATH    "D:/smsCommonBoxDb.type"
#define SMS_COFIG_PATH              "D:/smsConfigDb.json"


static char *getInboxSmsJsonPath(void)
{
    return SMS_INBOX_JSON_PATH;
}

static char *getInboxSmsTypePath(void)
{
    return SMS_INBOX_TYPE_PATH;
}

static char *getOutboxSmsJsonPath(void)
{
    return SMS_OUTBOX_JSON_PATH;
}

static char *getOutboxSmsTypePath(void)
{
    return SMS_OUTBOX_TYPE_PATH;
}

static char *getDraftBoxSmsJsonPath(void)
{
    return SMS_DRAFT_BOX_JSON_PATH;
}

static char *getDraftBoxSmsTypePath(void)
{
    return SMS_DRAFT_BOX_TYPE_PATH;
}

static char *getCommonBoxSmsJsonPath(void)
{
    return SMS_COMMON_BOX_JSON_PATH;
}

static char *getCommonBoxSmsTypePath(void)
{
    return SMS_COMMOM_BOX_TYPE_PATH;
}

int32_t addSmsToDb(char *fileName, char *typeFileName, void *sms)
{
    int32_t  retVal    = PHONE_STATUS_ERROR;
    uint32_t usedCount = 0;

    if ((fileName == NULL) || (typeFileName == NULL) || (sms == NULL))
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    if (getSmsUsedCount(&usedCount, PHONE_LOCATION_PHONE) != PHONE_STATUS_OK)
    {
        SYSLOG_DEBUG("Failed to get SMS used count.\r\n");
        goto labelEnd;
    }

    if (usedCount >= SMS_CAPACITY)
    {
        SYSLOG_DEBUG("The SMS on the phone is full.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbAddItem(fileName, typeFileName, sms);
#endif

labelEnd:
    return retVal;
}

int32_t getSmsFromDb(char *fileName, char *typeFileName, uint32_t index, PhoneSmsT *sms, uint32_t size)
{
    int32_t    retVal = PHONE_STATUS_ERROR;
    PhoneSmsT *item   = NULL; 

    if ((fileName == NULL) || (typeFileName == NULL) || (sms == NULL))
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    if ((jsonDbGetItemById(fileName, typeFileName, index, (void **)&item) == 1) && (item != NULL))
    {
        memcpy(sms, item, size);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(item);
#else
        free(item);
#endif
        item   = NULL;
        retVal = PHONE_STATUS_OK;
    }
#endif

labelEnd:
    return retVal;
}

int32_t addInboxSms(PhoneSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            addSmsToDb(getInboxSmsJsonPath(), getInboxSmsTypePath(), sms);
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t deleteInboxSms(uint32_t index, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbDeleteItem(getInboxSmsJsonPath(), getInboxSmsTypePath(), index);
#endif
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t getInboxSms(uint32_t index, PhoneSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            retVal = getSmsFromDb(getInboxSmsJsonPath(), getInboxSmsTypePath(), index, sms, sizeof(PhoneSmsT));
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t addOutboxSms(PhoneSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            addSmsToDb(getOutboxSmsJsonPath(), getOutboxSmsTypePath(), sms);
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t deleteOutboxSms(uint32_t index, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbDeleteItem(getOutboxSmsJsonPath(), getOutboxSmsTypePath(), index);
#endif
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t getOutboxSms(uint32_t index, PhoneSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            retVal = getSmsFromDb(getOutboxSmsJsonPath(), getOutboxSmsTypePath(), index, sms, sizeof(PhoneSmsT));
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t addDraftBoxSms(PhoneDraftBoxSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            addSmsToDb(getDraftBoxSmsJsonPath(), getDraftBoxSmsTypePath(), sms);
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t deleteDraftBoxSms(uint32_t index, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbDeleteItem(getDraftBoxSmsJsonPath(), getDraftBoxSmsTypePath(), index);
#endif
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t editDraftBoxSms(uint32_t index, PhoneDraftBoxSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            retVal = jsonDbUpdateItem(getDraftBoxSmsJsonPath(), getDraftBoxSmsTypePath(), index, sms);
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t getDraftBoxSms(uint32_t index, PhoneDraftBoxSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            retVal = getSmsFromDb(getDraftBoxSmsJsonPath(), getDraftBoxSmsTypePath(), index, (PhoneSmsT *)sms, sizeof(PhoneDraftBoxSmsT));
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t addCommonBoxSms(PhoneCommonBoxSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            addSmsToDb(getCommonBoxSmsJsonPath(), getCommonBoxSmsTypePath(), sms);
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t deleteCommonBoxSms(uint32_t index, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbDeleteItem(getCommonBoxSmsJsonPath(), getCommonBoxSmsTypePath(), index);
#endif
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t editCommonBoxSms(uint32_t index, PhoneCommonBoxSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            retVal = jsonDbUpdateItem(getCommonBoxSmsJsonPath(), getCommonBoxSmsTypePath(), index, sms);
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t getCommonBoxSms(uint32_t index, PhoneCommonBoxSmsT *sms, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (sms == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            retVal = getSmsFromDb(getCommonBoxSmsJsonPath(), getCommonBoxSmsTypePath(), index, (PhoneSmsT *)sms, sizeof(PhoneCommonBoxSmsT));
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t getSmsCapacity(uint32_t *capacity, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (capacity == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            *capacity = SMS_CAPACITY;
            retVal    = PHONE_STATUS_OK;
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t getSmsUsedCount(uint32_t *count, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (count == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            *count  = jsonDbGetTotalCount(SMS_INBOX_JSON_PATH,     SMS_INBOX_TYPE_PATH);
            *count += jsonDbGetTotalCount(SMS_OUTBOX_JSON_PATH,    SMS_OUTBOX_TYPE_PATH);
            *count += jsonDbGetTotalCount(SMS_DRAFT_BOX_JSON_PATH, SMS_DRAFT_BOX_TYPE_PATH);
            retVal  = PHONE_STATUS_OK;
#endif
            break;

        case PHONE_LOCATION_SIM1:
            break;

        case PHONE_LOCATION_SIM2:
            break;

        default:
            SYSLOG_DEBUG("location out of range\r\n");
            goto labelEnd;
            break;
    }

labelEnd:
    return retVal;
}

int32_t editSmsConfig(PhoneSmsConfigT *config)
{
    return 0;
}

int32_t getSmsConfig(PhoneSmsConfigT *config)
{
    return 0;
}
