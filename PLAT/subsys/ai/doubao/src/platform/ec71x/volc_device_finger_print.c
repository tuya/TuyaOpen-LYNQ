#include "volc_device_finger_print.h"
#include "radio_device.h"
#include "ccio_base.h"

#include "volc_type.h"

uint32_t volc_get_mac_address(volc_string_t* mac_addr) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(NULL != mac_addr, VOLC_STATUS_INVALID_ARG);

	CcioMacAddr_t macAddr = {{0}};
	radioDevQueryLocalMac(&macAddr);

    volc_string_snprintf(mac_addr,32,"%2X%2X%2X%2X%2X%2X",macAddr.mac[0], macAddr.mac[1], macAddr.mac[2], macAddr.mac[3], macAddr.mac[4], macAddr.mac[5]);


err_out_label:
    return ret;
}

uint32_t volc_get_device_finger_print(volc_string_t* finger) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(NULL != finger, VOLC_STATUS_INVALID_ARG);
    volc_get_mac_address(finger);
err_out_label:
    return ret;
}

uint32_t volc_generate_device_id(volc_string_t* did) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    volc_string_t device_id;
    VOLC_CHK(NULL != did, VOLC_STATUS_INVALID_ARG);
    volc_string_init(&device_id);
    volc_get_mac_address(&device_id);
    volc_string_set(did, volc_string_get(&device_id));
    volc_string_deinit(&device_id);
err_out_label:
    return ret;
}