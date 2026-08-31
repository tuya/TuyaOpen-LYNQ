/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		comm_test.c
 Author:			  liuch
 Version:			  V1.0
 Date:			    2020-04-24
 Description:	  comm_test interface definition
 History:

******************************************************************************/

#include "hed_private.h"
#include "port_ec71x_i2c.h"
//#include "vendor_api.h"
#include "vendor_se_v2.h"

#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE

se_error_t enter_lpm_test(void)
{
    uint8_t in_buf_len = 0x05;
    uint8_t in_buf[15] = {0};
    uint8_t out_buf[20] = {0};
    uint32_t out_buf_en = 0;
    uint8_t exp_outbuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


    se_error_t ret = 0;

    csi_enter_lpm(0);

    memcpy(in_buf, "\x80\xC8\x01\x00\x08", in_buf_len);

    ret = api_transceive(in_buf, in_buf_len, out_buf, &out_buf_en);
    if (ret == SE_SUCCESS)
    {
        return ret;
    }

    ret = memcmp(out_buf, exp_outbuf, 8);
    if (ret != 0)
    {
        return ret;
    }

    return SE_SUCCESS;
}

se_error_t exit_lpm_test(void)
{
    uint8_t in_buf_len = 0x05;
    uint8_t in_buf[15] = {0};
    uint8_t out_buf[20] = {0};
    uint32_t out_buf_en = 0;
    uint8_t exp_outbuf[8] = {0x34, 0x70, 0x04, 0x10, 0x00, 0x00, 0x00, 0x00};
    se_error_t ret = 0;

    csi_exit_lpm(0);

    memcpy(in_buf, "\x80\xC8\x01\x00\x08", in_buf_len);

    ret = api_transceive(in_buf, in_buf_len, out_buf, &out_buf_en);
    if (ret != SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, exit_lpm_test0, P_SIG, "exit lpm failed:%d", ret);
        return ret;
    }
    ret = memcmp(out_buf, exp_outbuf, 8);
    if (ret != 0)
    {
        ECPLAT_DUMP(UNILOG_PLA_APP, exit_lpm_test1, P_SIG, "exit lpm dump:",8,out_buf);
        return ret;
    }

    return SE_SUCCESS;
}

/*************************************************
Function:	   comm_test
Description: 通信测试
Input:	     no
Output:	     no
Return:   	 参见error.h
Others:	     no
*************************************************/
int32_t comm_test(void)
{

    uint8_t com_outbuf[300] = {0};
    uint32_t outlen = 0;
    uint8_t in_buf[15] = {0};
    uint32_t in_buf_len = 0;
    uint8_t out_buf[1024] = {0};
    uint32_t out_buf_en = 0;
    uint8_t atr[30] = {0};
    uint32_t atr_len = 0;
    int32_t ret = 0;

    /****************************************SE注册、选择和连接*****************************************/
    HED_IIC_Init();
#ifdef HED_I2C_SE0
    //---- 1. 注册设备----
    ret = api_register(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
    if (ret != SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test0, P_WARNING, "failed to i2c api_register");
        return ret;
    }
    //---- 2. 选择设备 ----
    ret = api_select(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
    if (ret != SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test1, P_WARNING, "failed to i2c api_select");
        return ret;
    }
#endif

    //---- 3. 连接，获取atr ----
    ret = api_connect(com_outbuf, &outlen);
    if (ret != SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test2, P_WARNING, "failed to api_connect");
        return ret;
    }
    ECPLAT_DUMP(UNILOG_PLA_APP, comm_test3, P_SIG, "api_connect atr:", outlen, com_outbuf);
    /****************************************SE发送接收*****************************************/

    // 测试结束
    //---- 4. 获取证书 ----
    in_buf_len = 0x05;
    memcpy(in_buf, "\xA0\xD3\x01\x01\x09", in_buf_len);

    ret = api_transceive(in_buf, in_buf_len, out_buf, &out_buf_en);
    if (ret != SE_SUCCESS)
    {
        return ret;
    }
    ECPLAT_DUMP(UNILOG_PLA_APP, comm_test4, P_SIG, "api_transceive out_buf:", 16, out_buf);
    /****************************************SE热复位，接收ATR*****************************************/
    atr_len = 0;

    ret = api_reset(atr, &atr_len);
    if (ret != SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test5, P_WARNING, "failed to api_reset");
        return ret;
    }
    /****************************************low power************************************************/
    ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test5_0, P_WARNING, "low power test");
    ret = enter_lpm_test();
    if(ret !=SE_SUCCESS )
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test5_1, P_WARNING, "failed to enter low power mode");
        return ret;
    }
    osDelay(5000);
    ret = exit_lpm_test();
    if(ret!=SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test5_2, P_WARNING, "failed to exit low power mode");
        return ret;
    }
    /****************************************SE断开连接*****************************************/

    ret = api_disconnect();
    if (ret != SE_SUCCESS)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test6, P_WARNING, "failed to api_disconnect");
        return ret;
    }

    ECPLAT_PRINTF(UNILOG_PLA_APP, comm_test7, P_WARNING, "test SE_SUCCESS");

    return SE_SUCCESS;
}


