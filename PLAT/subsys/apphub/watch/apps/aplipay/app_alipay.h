#ifndef __APP_ALIPAY_H__
#define __APP_ALIPAY_H__

typedef enum _appAliBindingStat{
	E_APP_ALI_UNBOUND,
	E_APP_ALI_BINDING,
	E_APP_ALI_BINDING_SUCCESS,
	E_APP_ALI_BINDING_FAIL,
	E_APP_ALI_BINDING_EXIT
}appAliBindingStat;

void ui_alipay_init(void);
void ui_binding_stat_show(uint8_t status);
void ui_paymnet_state_show(uint8_t result, uint8_t status);
#endif
