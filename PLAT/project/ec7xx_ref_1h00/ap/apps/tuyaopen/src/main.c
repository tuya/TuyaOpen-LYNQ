/****************************************************************************
 *
 * File name:    app.c
 * Description:  open app demo entry source file
 *
 ****************************************************************************/
#include "string.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "os_common.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "plat_config.h"
#include "ps_lib_api.h"
#include "ps_event_callback.h"
#include "slpman.h"
#include "reset.h"
#include "ol_sys_api.h"
#include "vlog.h"
#include "ol_fs_api.h"
#include "ol_fs_ex_api.h"



#define MODULE_NAME_BUF_SIZE    16

static char module_name[MODULE_NAME_BUF_SIZE] = {0};

/**
 * @brief Whether chip is EC718PVM (module name carries N and V suffixes).
 * @return 1 if chipid is EC718PVM, otherwise 0
 */
static int __is_pvm_chip(void)
{
    char *chipid = ol_get_chipid();

    if ((chipid == NULL) || (chipid[0] == '\0')) {
        return 0;
    }
    return (strcmp(chipid, "EC718PVM") == 0) ? 1 : 0;
}

/**
 * @brief Map rfCustomerMd prefix to product module name (date suffix ignored).
 *        N/V in module name follows ol_get_chipid(): EC718PVM -> *N-Y7PVM, else *-Y7PM.
 */
static void __set_module_name_from_rf(const char *rf_customer_md)
{
    int is_pvm = 0;

    if ((rf_customer_md == NULL) || (rf_customer_md[0] == '\0')) {
        return;
    }

    memset(module_name, 0, sizeof(module_name));
    is_pvm = __is_pvm_chip();

    /* L511G-Y7PM / L511G-Y7PVM */
    if (strstr(rf_customer_md, "L511G") != NULL) {
        strncpy(module_name, is_pvm ? "L511G-Y7PVM" : "L511G-Y7PM", sizeof(module_name) - 1);
        return;
    }

    /* L511C-Y7PM / L511CN-Y7PVM  <- L511C-Y7P_M_V2_20250311 */
    if (strstr(rf_customer_md, "L511C") != NULL) {
        strncpy(module_name, is_pvm ? "L511CN-Y7PVM" : "L511C-Y7PM", sizeof(module_name) - 1);
        return;
    }

    /* L511LA-Y7PM / L511LAN-Y7PVM <- L511LA-Y7B_V1_20260318 */
    if (strstr(rf_customer_md, "L511LA") != NULL) {
        strncpy(module_name, is_pvm ? "L511LAN-Y7PVM" : "L511LA-Y7PM", sizeof(module_name) - 1);
        return;
    }

    /* L511E-Y7PM / L511EN-Y7PVM share the same RF prefix */
    if (strstr(rf_customer_md, "L511E") != NULL) {
        strncpy(module_name, is_pvm ? "L511EN-Y7PVM" : "L511E-Y7PM", sizeof(module_name) - 1);
        return;
    }

    /* Fallback: copy L511... segment before first '_' */
    {
        char *ptr = strstr((char *)rf_customer_md, "L511");
        char *underscore = NULL;
        size_t name_len = 0;

        if (ptr == NULL) {
            return;
        }
        underscore = strstr(ptr, "_");
        if (underscore == NULL) {
            return;
        }
        name_len = (size_t)(underscore - ptr);
        if (name_len >= sizeof(module_name)) {
            name_len = sizeof(module_name) - 1;
        }
        memcpy(module_name, ptr, name_len);
        module_name[name_len] = '\0';
    }
}

/* Both live in libdriver_private.a and the package ships no header for
 * them; declare them here rather than calling through an implicit int(). */
extern void ShareInfoWakeupCP4Version(void);
extern void ShareInfoAPGetRfCustomerPrjNameInfo(UINT8 **rfCustomerMd, UINT8 **rfPaMd,
                                                UINT8 **rfAsmMd, UINT8 **rfCalcTime);

void check_RF_version(void)
{
    UINT8 *pRfCustomerMd = PNULL, *pRfPaMd = PNULL, *pRfAsmMd = PNULL, *pRfCalcTime = PNULL;
    UINT8 rfCustomerMd[32];

    ShareInfoWakeupCP4Version();
    delay_us(150000);// delay 150ms for cp boot

    ShareInfoAPGetRfCustomerPrjNameInfo(&pRfCustomerMd, &pRfPaMd, &pRfAsmMd, &pRfCalcTime);
    memset(rfCustomerMd, 0, sizeof(rfCustomerMd));
    if (PNULL == pRfCustomerMd) {
        /* CP not up yet, or no RF info stored: keep going, the module name
         * is informational. */
        LOGE("rf customer info not available");
        return;
    }
    memcpy(rfCustomerMd, pRfCustomerMd, sizeof(rfCustomerMd) - 1);

    LOGI("rfCustomerMd: %s", rfCustomerMd);
    __set_module_name_from_rf((const char *)rfCustomerMd);
    if (module_name[0] != '\0') {
        LOGI("module_name: %s", module_name);
    }
}

void LYNQ_MODULE_GET_NAME(char *name)
{
    memcpy(name, module_name, sizeof(module_name));
}

/*
  App init entry function. Customer do init or create tasks based on their needs
*/
static void app_init(void *arg)
{
    LastResetState_e ap_state;
    LastResetState_e cp_state;
    slpManSlpState_t slpstate;

    /* First thing the application logs: seeing this tells you the AP got
     * this far and that the log database matches the firmware. */
    LOGD("app_init enter");
    osDelay(500);

    check_RF_version();

    LOGD("App demo init: %s, chipid: %s", ol_get_sw_version(), ol_get_chipid());

    ResetStateGet(&ap_state, &cp_state);
    LOGD("Check last reset reason: ap_state = %d, cp_state = %d",ap_state, cp_state);

    //check wakeup from deep sleep or power on
    slpstate = slpManGetLastSlpState();
    LOGD("Check last sleep state = %d",slpstate);

    ol_set_dump_flag(OL_DUMP_PRINT_RESET);
	// ol_set_dump_flag(OL_DUMP_FLASH_EPAT_LOOP);

    /* The Tuya KV database lives in this filesystem, and the region is small
     * enough that a full one shows up as write failures -- worth one line. */
	int t_size = ol_fs_get_totalspacesize(FS_TYPE_INTERNAL);
    int u_size = ol_fs_get_usedspacesize(FS_TYPE_INTERNAL);
    int f_size = ol_fs_get_freespacesize(FS_TYPE_INTERNAL);
    LOGD("check fs: total/%d, used/%d, free/%d", t_size, u_size, f_size);
    LOGD("FreeHeapSize:%d", xPortGetFreeHeapSize());
    LOGD("Build time %s %s", __DATE__, __TIME__);

    /* TuyaOpen apps start here: tuya_main.c spawns the application thread
     * from tuya_app_main(), there is no main() on an RTOS build. */
    extern void tuya_app_main(void);
    tuya_app_main();
}

/**
  \fn          int main_entry(void)
  \brief       main entry function.
  \return
*/
void main_entry(void)
{
    BSP_CommonInit();
    slpManAONIOPowerOn();
    mbtkInitENGpio();

    //IO voltage default is 1.8V
    //Modify the IO voltage to 3.3V or other voltage define in IOVoltageSel_t as follows
    #if defined(PMU_AON_GPIO_LEVEL) && (PMU_AON_GPIO_LEVEL == 1)
    slpManAONIOVoltSet(IOVOLT_3_30V);
    #endif
    #if defined(PMU_NORMAL_GPIO_LEVEL) && (PMU_NORMAL_GPIO_LEVEL == 1)
    slpManNormalIOVoltSet(IOVOLT_3_30V);
    #endif

    osKernelInitialize();
    registerAppEntry(app_init, NULL);
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }
    while(1);
}
