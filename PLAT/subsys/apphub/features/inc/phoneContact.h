#ifndef __PHONE_CONTACT_H__
#define __PHONE_CONTACT_H__


#include <stdint.h>
#include "phone.h"


#define CONTACT_CAPACITY            500


typedef struct
{
    char name[CONTACT_NAME_LENGTH + 1];                 //phone, 30 Chinese characters. SIM, 6 Chinese characters.
    char mobileNumber[CONTACT_NUMBER_LENGTH + 1];
    char officeNumber[CONTACT_NUMBER_LENGTH + 1];       //phone only
} PhoneContactT;


int32_t addContact(PhoneContactT *contact, PhoneLocationT location);
int32_t deleteContact(uint32_t index, PhoneLocationT location);
int32_t editContact(uint32_t index, PhoneContactT *contact, PhoneLocationT location);
int32_t getContact(uint32_t index, PhoneContactT *contact, PhoneLocationT location);
int32_t getContactCapacity(uint32_t *capacity, PhoneLocationT location);
int32_t getContactUsedCount(uint32_t *count, PhoneLocationT location);
int32_t getContactByKey(char *key, PhoneContactT **contact, uint32_t **ids, uint32_t *count, PhoneLocationT location);


#endif
