#ifndef __USB_NET_ADAPT_CUST_H__
#define __USB_NET_ADAPT_CUST_H__


#if 1


void UsbNetATAdaptInitCb(void);
uint8_t UsbNetATAdaptEnabledCb(void);

void UsbNetATAdaptRstIsrCb(void);
void UsbNetATAdaptAddrIsrCb(void);
void UsbNetATAdaptDevDescIsrCb(void);

void UsbNetATAdaptCfgDescIsrCb(uint32_t ReqMsgLen);


void UsbNetATAdaptSetCfgIsrCb(void);
void UsbNetATAdaptSwtPreDiscReqCb(void);
void UsbNetATAdaptSWTPendSuspCb(void);
void UsbNetATAdaptSwtReEnumReqCb(void);

void UsbNetATAGetSwtExpectType(uint8_t *pSwtUsbNetTypePending, uint8_t *pExpectedUsbNetType);
uint8_t UsbNetATAUpdGetRealUsedType(uint8_t need_update, uint8_t *RealUsedUsbNetTypeCur);
uint8_t UsbNetOrgUpdGetRealUsedType(void);


#endif


typedef struct {
    void (*init)(void);
    uint8_t (*get_adapt_cfg_enabled)(void);
    
    void (*rstisr_cb)(void);
    void (*addrisr_cb)(void);
    void (*devdesc_cb)(void);
    void (*cfgdesc_cb)(uint32_t ReqMsgLen);
    void (*setcfgdesc_cb)(void);
//    void (*suspend_cb)(void);


    void (*get_adapt_swt_type)(uint8_t* pSwtUsbNetTypePending, uint8_t* pExpectedUsbNetType);
    uint8_t (*upd_get_real_used_type)(uint8_t NeedUpd, uint8_t *RealUsedUsbNetTypeCur);

} usbnet_adapt_ops;


extern void USBDynamicInstInit(void);

extern uint8_t UsbNetATATrySwtEntInstMutex(void);
extern void UsbNetATASwtExitInstUnMutex(void);

extern uint8_t UsbVBUSTrySwtEntInstMutex(void);
extern void UsbVBUSwtExitInstUnMutex(void);

typedef struct {
    void (*usb_dyn_inst_mgr_init)(void);
    uint8_t (*usb_net_ata_swt_try_enter_inst_mutex)(void);
    void (*usb_net_ataswt_exit_inst_unmutex)(void);

    uint8_t (*vbus_swt_try_enter_inst_mutex)(void);
    void (*vbus_swt_exit_inst_unmutex)(void);
}usb_dynamic_inst_mgr_ops;



extern const usbnet_adapt_ops t_usbnet_adapt;
extern const usb_dynamic_inst_mgr_ops t_usb_dynamic_inst_mgr_ops;

#endif


