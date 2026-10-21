# openHAL - screen 

和driver中lcd驱动的差异说明：

* openhal中的驱动更新频率高，功能接口丰富
* openhal前期直接调用现有接口，后期优化直接配置相应寄存器

| screen | param1 | param2 | param3 | return | brief | 
| ------- | ------ | ------ | ------ | ------ | ----- |
| api_lcd_startup | √ | - | - | - |  上电初始化
| api_lcd_query  | index  |  - | -  | enum | 传输状态查询
| api_lcd_create | index  | cfg | dma cb | handle | 创建设备端口
| api_lcd_open   | handle | register | lspi cb | usrId | 申请接口
| api_lcd_apply  | usrId  |  | - |  申请接口
| api_lcd_ioctl  |   |   |   | 传输接口 
| api_lcd_pmctl  |   |   |   | 功耗模式 
| api_lcd_close  | usrId |    | enum |  关闭接口 
| api_lcd_delete | usrId |    | enum |  删除接口 
| api_test_lcd   | usrId |    |    |  测试接口 


* api_lcd_default可同步修改api_lcd_startup的数据




