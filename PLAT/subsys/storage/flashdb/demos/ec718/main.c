#include <stdio.h>
#include <flashdb.h>


#define FDB_LOG_TAG "[main]"


#ifdef FDB_USING_KVDB
extern void kvdb_basic_sample(fdb_kvdb_t kvdb);
extern void kvdb_type_string_sample(fdb_kvdb_t kvdb);
extern void kvdb_type_blob_sample(fdb_kvdb_t kvdb);


static struct fdb_kvdb kvdb = {0};
#endif


int32_t flashDbTest(void)
{
    int32_t retVal;

#ifdef FDB_USING_KVDB
    retVal = fdb_kvdb_init(&kvdb, ON_CHIP_FLASH_DB, ON_CHIP_FLASH_DB_PARTITION, NULL, NULL);
    // retVal = fdb_kvdb_init(&kvdb, SPI_FLASH_DB, SPI_FLASH_DB_PARTITION, NULL, NULL);
    if (retVal != FDB_NO_ERR)
    {
        FDB_INFO("Failed to init KVDB: retVal=%d\r\n", retVal);
    }
    else
    {
        kvdb_basic_sample(&kvdb);
        kvdb_type_string_sample(&kvdb);
        kvdb_type_blob_sample(&kvdb);
    }
#endif

    return retVal;
}
