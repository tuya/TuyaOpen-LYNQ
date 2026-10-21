#include <stdlib.h>
#include "tkl_ota.h"
#include "cmsis_os2.h"
#include "osasys.h"
#include "ol_fota_api.h"
#include "vlog.h"

unsigned int offset = 0;

/*************************variable define********************/
OPERATE_RET tkl_ota_get_ability(UINT_T* image_size, TUYA_OTA_TYPE_E* type)
{
    int freesize = 512 * 1024;

    if (image_size) {
        *image_size = freesize;
    }

    if (type) {
        *type = TUYA_OTA_DIFF;
    }

    LOGI("get ability, size/%d, type/%d", image_size, *type);
    return OPRT_OK;
}

/**
* @brief ota start notify
*
* @param[in] image_size:  image size
* @param[in] type:        ota type
* @param[in] path:        ota path
*
* @note This API is used for ota start notify
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_ota_start_notify(UINT_T image_size, TUYA_OTA_TYPE_E type, TUYA_OTA_PATH_E path)
{
	int ret;
	static bool nvm_init_flag = false;
    LOGI("tkl_ota_start_notify file size/%d", image_size);

	if(!nvm_init_flag) {
		ret = ol_fota_nvm_init();
		if (0 != ret) {
			LOGE("ol_fota_nvm_init failed: %d", ret);
			return OPRT_COM_ERROR;
		}
		nvm_init_flag = true;
	}

	offset = 0;
    return OPRT_OK;
}

/**
* @brief ota data process
*
* @param[in] pack:       point to ota pack
* @param[in] remain_len: ota pack remain len
*
* @note This API is used for ota data process
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_ota_data_process(TUYA_OTA_DATA_T* pack, UINT_T* remain_len)
{
	int ret;
    if (NULL == pack) {
        LOGE("data process error, null data");
        return OPRT_INVALID_PARM;
    }

	ret = ol_fota_nvm_write(offset, pack->data, pack->len);
	if (0 != ret) {
		LOGE("ota data write to nvm failed: %d", ret);
		return OPRT_COM_ERROR;
	}

	offset += pack->len;
	*remain_len = 0;

    return OPRT_OK;
}

/**
* @brief ota end notify
*
* @param[in] reset:  ota reset
*
* @note This API is used for ota end notify
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_ota_end_notify(BOOL_T reset)
{
	int ret;

	LOGI("tkl ota end notify");

	if (0 == offset) {
		LOGE("image size null");
		return OPRT_COM_ERROR;
	}

	ret = ol_fota_nvm_check();
	if (0 != ret) {
		LOGE("check fota nvm failed: %d", ret);
		return OPRT_COM_ERROR;
	}

	ol_fota_reboot_upgrade();

    return OPRT_OK;
}

/**
* @brief get old firmware info
*
* @param[out] image_size:  max image size
* @param[out] type:        ota type
*
* @note This API is used for old firmware info, and only used in resumes transmission at break-points
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_ota_get_old_firmware_info(TUYA_OTA_FIRMWARE_INFO_T** info)
{
    LOGI("ota get old fw info not supported");
    return OPRT_NOT_SUPPORTED;
}
