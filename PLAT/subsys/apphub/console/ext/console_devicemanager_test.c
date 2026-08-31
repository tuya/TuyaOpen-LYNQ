/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_device_manager.c
 * Description:  EC718
 * History:      Rev1.0   2025-09-15
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "string.h"
#include "devicemanager.h"
#include "api_cam.h"
#include "api_scr.h"
#include "api_tp.h"
#include <stdbool.h>

extern int skip_atoi(const char **s);

int cmd_devicemanager(int argc, char **argv)
{
    char *sub_cmd = argv[1];
    // devicemanager list
    if(strcmp(sub_cmd, "list") == 0)
    {
        device_info_t *dev_info = malloc(sizeof(device_info_t) * 64);
        int device_num = Devices_list(dev_info, 64);
        for(size_t i = 0; i < device_num; i++)
        {
            printf("device_path: %s\r\n", dev_info[i].device_path);
            printf("device_type: %d\r\n", dev_info[i].device_type);
            printf("device_id: %d\r\n", dev_info[i].device_id);
            printf("state: %d\r\n", dev_info[i].state);
            printf("power_mode: %d\r\n", dev_info[i].power_mode);
            printf("reference_count: %d\r\n", dev_info[i].reference_count);
            printf("last_access_time: %d\r\n", dev_info[i].last_access_time);
        }
        free(dev_info);
    }
    // devicemanager open dev:/camera
    else if(strcmp(sub_cmd, "open") == 0)
    {
        if(strcmp(argv[2], "dev:/camera") == 0)
        {
            CamExtPwrCfg_t ext_pwr_cfg = {.enable = true,
                                          .pad_num = 11,
                                          .mux = PAD_MUX_ALT4,
                                          .io_num = 16};
            CamRstPinCfg_t rst_pin_cfg = {.pad_num = 20,
                                          .mux = PAD_MUX_ALT0,
                                          .io_num = 5,
                                          .reset_level = 1};
            CamType_e type = api_cam_probe_type(1, &ext_pwr_cfg, &rst_pin_cfg);
            CamCfg_t cfg;
            api_cam_default(type, &cfg);
            int ret = Device_open(argv[2], &cfg, 1000, NULL);
            printf("ret = %d\r\n", ret);
        }
        else if(strcmp(argv[2], "dev:/lcd") == 0)
        {
        }
        else if(strcmp(argv[2], "dev:/tp") == 0)
        {
        }
    }
    else if(strcmp(sub_cmd, "close") == 0)
    {
        Device_close(argv[2]);
    }
    // devicemanager set_power dev:/camera 0
    else if(strcmp(sub_cmd, "set_power") == 0)
    {
        Devices_set_power_mode(argv[2], skip_atoi(&argv[3]));
    }
    else if(strcmp(sub_cmd, "get_power") == 0)
    {
        device_power_mode_t power_mode;
        Devices_get_power_mode(argv[2], &power_mode);
        printf("power_mode = %d\r\n", power_mode);
    }
    else if(strcmp(sub_cmd, "suspend") == 0)
    {
        Devices_suspend(argv[2]);
    }
    else if(strcmp(sub_cmd, "resume") == 0)
    {
        Devices_resume(argv[2]);
    }
    else if(strcmp(sub_cmd, "any_active") == 0)
    {
        bool ret = Device_any_active();
        printf("ret = %d\r\n", ret);
    }
    // devicemanager enable_power_mode /dev/camera 0
    else if(strcmp(sub_cmd, "enable_power_mode") == 0)
    {
        Device_enable_power_mode(argv[2], skip_atoi(&argv[3]));
    }
    // devicemanager disable_power_mode /dev/camera 0
    else if(strcmp(sub_cmd, "disable_power_mode") == 0)
    {
        Device_disable_power_mode(argv[2], skip_atoi(&argv[3]));
    }
    // devicemanager clock_enable dev:/camera
    else if(strcmp(sub_cmd, "clock_enable") == 0)
    {
        Device_bus_clock_enable(argv[2]);
    }
    // devicemanager clock_disable dev:/camera
    else if(strcmp(sub_cmd, "clock_disable") == 0)
    {
        Device_bus_clock_disable(argv[2]);
    }
    // devicemanager ioctl dev:/camera 2 240(0xf0)/241(0xf1)
    else if(strcmp(sub_cmd, "ioctl") == 0)
    {
        if(strcmp(argv[2], "dev:/camera") == 0)
        {
            uint8_t para = 0;
            Device_ioctl(argv[2], skip_atoi(&argv[3]), &para);
            printf("para = %d\r\n", para);
        }
        else if(strcmp(argv[2], "dev:/lcd") == 0)
        {
        }
        else if(strcmp(argv[2], "dev:/tp") == 0)
        {
        }
    }
    else
    {
        rt_kprintlnf("Usage: devicemanager [options]");
        rt_kprintlnf("[options]:");
        rt_kprintlnf("    %-10s - get device list", "list");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_devicemanager, devicemanager, devicemanager test);

#endif  // FEATURE_SUBSYS_CONSOLE_ENABLE