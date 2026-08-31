#include "alipay_common.h"
#include <stdio.h>
#include <string.h>
#include "ps_lib_api.h"
#include "vendor_device_info.h"
#include "csi_device_info.h"
#include "bsp.h"


#define DEMO_MAC_ADDR 		"1C:EC:06:18:00:C1"
#define DEMO_COMPANY_NAME 	"YiXin"
#define	DEMO_PRODUCT_MODULE "EC_W1"
retval_e alipay_get_mac(uint8_t* buf_did, uint32_t *len_did)
{
	if(!buf_did || !len_did) {
		return ALIPAY_RV_WRONG_PARAM;
	}
	uint32_t devIdLen = *len_did;

	if(strlen(DEMO_MAC_ADDR) > devIdLen)
		return ALIPAY_RV_HAL_ERROR;
	devIdLen = strlen(DEMO_MAC_ADDR);
	memcpy(buf_did,DEMO_MAC_ADDR,devIdLen + 1);
	*len_did = devIdLen;

	return ALIPAY_RV_OK;
}


retval_e alipay_get_sn(uint8_t* buf_sn, uint32_t *len_sn)
{

	if(!buf_sn || !len_sn) {
		return ALIPAY_RV_WRONG_PARAM;
	}

	if(appGetImeiNumSync(buf_sn)) //imei max size is 32Byte
		return ALIPAY_RV_HAL_ERROR;
	*len_sn = strlen(buf_sn);

	return ALIPAY_RV_OK;
}


retval_e alipay_get_imei(uint8_t* buf_imei, uint32_t *len_imei)
{

    if(!buf_imei || !len_imei) {
        return ALIPAY_RV_WRONG_PARAM;
    }

	if(appGetImeiNumSync(buf_imei)) //imei max size is 32Byte
    	return ALIPAY_RV_HAL_ERROR;
    *len_imei = strlen(buf_imei);

    return ALIPAY_RV_OK;
}


/*获取设备类型，根据设备实际情况返回
 * @param 
 * @return 0    成人手表
 *         1    成人手环
 *         2    成人卡片
 *         3    儿童手表
 *         4    儿童手环
 *         5    学生卡
 */
// 默认0, 测试商家聚合码需要设置为3
int g_productType = 0;
uint32_t alipay_get_productType(void) {
    return g_productType;
}

retval_e alipay_get_companyName(uint8_t* buffer, uint32_t* len)
{
	alipay_log_ext("alipay_get_companyName");
	if(!buffer || !len) {
		return ALIPAY_RV_WRONG_PARAM;
	}
	uint32_t company_name_len = *len;

	if(strlen(DEMO_COMPANY_NAME) > company_name_len)
		return ALIPAY_RV_HAL_ERROR;
	company_name_len = strlen(DEMO_COMPANY_NAME);
	memcpy(buffer,DEMO_COMPANY_NAME,company_name_len + 1);
	*len = company_name_len;

	return ALIPAY_RV_OK;
}

retval_e alipay_get_productModel(uint8_t* buffer, uint32_t* len)
{
	if(!buffer || !len) {
		return ALIPAY_RV_WRONG_PARAM;
	}
	uint32_t product_model_len = *len;

	if(strlen(DEMO_PRODUCT_MODULE) > product_model_len)
		return ALIPAY_RV_HAL_ERROR;
	product_model_len = strlen(DEMO_PRODUCT_MODULE);
	memcpy(buffer,DEMO_PRODUCT_MODULE,product_model_len + 1);
	*len = product_model_len;

	return ALIPAY_RV_OK;
}

