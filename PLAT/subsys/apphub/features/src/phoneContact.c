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
#include "phoneContact.h"


#define CONTACT_JSON_PATH           "D:/contactDb.json"
#define CONTACT_TYPE_PATH           "D:/contactDb.type"


static char *getContactJsonPath(void)
{
    return CONTACT_JSON_PATH;
}

static char *getContactTypePath(void)
{
    return CONTACT_TYPE_PATH;
}

int32_t addContact(PhoneContactT *contact, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (contact == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            if (jsonDbGetTotalCount(getContactJsonPath(), getContactTypePath()) >= CONTACT_CAPACITY)
            {
                SYSLOG_DEBUG("The contact on the phone is full.\r\n");
                goto labelEnd;
            }
            retVal = jsonDbAddItem(getContactJsonPath(), getContactTypePath(), contact);
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

int32_t deleteContact(uint32_t index, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbDeleteItem(getContactJsonPath(), getContactTypePath(), index);
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

int32_t editContact(uint32_t index, PhoneContactT *contact, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (contact == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbUpdateItem(getContactJsonPath(), getContactTypePath(), index, contact);
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

int32_t getContact(uint32_t index, PhoneContactT *contact, PhoneLocationT location)
{
    int32_t        retVal = PHONE_STATUS_ERROR;
    PhoneContactT *item   = NULL; 

    if (contact == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
            retVal = jsonDbGetItemById(getContactJsonPath(), getContactTypePath(), index, (void **)&item);
#endif
            if (item != NULL)
            {
                memcpy(contact, item, sizeof(PhoneContactT));
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(item);
#else
                free(item);
#endif
                item = NULL;
            }
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

int32_t getContactCapacity(uint32_t *capacity, PhoneLocationT location)
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
            *capacity = CONTACT_CAPACITY;
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

int32_t getContactUsedCount(uint32_t *count, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (count == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    *count = 0;

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            *count = jsonDbGetTotalCount(getContactJsonPath(), getContactTypePath());
            retVal = PHONE_STATUS_OK;
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

int32_t getContactByKey(char *key, PhoneContactT **contact, uint32_t **ids, uint32_t *count, PhoneLocationT location)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if ((key == NULL) || (contact == NULL) || (ids == NULL) || (count == NULL))
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    switch (location)
    {
        case PHONE_LOCATION_PHONE:
            *count = jsonDbGetMatchedItemsByStr(getContactJsonPath(), getContactTypePath(), key, (void **)contact, ids);
            retVal = PHONE_STATUS_OK;
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
