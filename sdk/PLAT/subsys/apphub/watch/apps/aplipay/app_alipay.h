#ifndef __APP_ALIPAY_H__
#define __APP_ALIPAY_H__

typedef struct _queueAlipay{
	uint8_t bindStatus;
	uint8_t paymentStatus;
	uint8_t paymentResult;
}queueAlipay;

typedef enum _appAliBindingStat{
	E_APP_ALI_UNBOUND,
	E_APP_ALI_BINDING,
	E_APP_ALI_BINDING_SUCCESS,
	E_APP_ALI_BINDING_FAIL,
	E_APP_ALI_BINDING_EXIT
}appAliBindingStat;

void appAliPayUiInit(void);
#endif
