# SensorHub

通过LDO供电，需要配置对应的电压值，默认为3.0v

IMU存在自校准问题，导致开机数据不准异常输出，多个任务同时执行的情况下，影响自动校准

* 需要注意超时和异常处理机制，在没有设备的情况下，看门狗会复位

| MPU6500 | GPIO | PAD | MuxAlt | M2324 | miniDKB |
| ------- | ------ | ------ | ------ | ------ | ------ |
| SDA | GPIO14 | 29 | ALT3 |  |   | 
| SCL | GPIO15 | 30 | ALT3 |  |   | 
| IRQ | GPIO36 | 42 | ALT0 |  | LSP-SDO | 

* IMU测试软件I2C读取异常

[MPU-6500-Register](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6500-Register-Map2.pdf)

中断唤醒输出的高电平150ms,使用AGPIO作为唤醒，目前在miniDKB上无法测试，因为没有可用的AGPIOWAKE通道
```
Pad0 Wakeup 	                            2023-08-03 09:54:13.095
MPU_INIT	ID 0x70 , INT 0x43 , MOT 0x0 	2023-08-03 09:54:13.095
Enter_Hibernate	Enter Hibernate 	        2023-08-03 09:54:20.199

```
由于唤醒通道无法配置成中断管脚，所以后续需要将其配置成无中断的轮询模式，可以兼容更多模组

运动中断应用所涉及到的相关内部寄存器主要包括：

* 中断使能寄存器：INT_ENABLE ，寄存器地址0x38，为MPU6050所有中断输出的开关寄存器，用于使能运动中断、FIFO溢出中断和数据中断等；
* FIFO控制寄存器：USER_CTRL，寄存器地址0x6A，该寄存器用于使能FIFO，并可控制MPU6050的I2C主机。应用运动中断功能时，应关闭FIFO和I2C主机；
* FIFO使能寄存器：FIFO_EN，寄存器地址0x23，该寄存器用于使能各个FIFO功能，在应用运动中断功能时应关闭；
* 中断引脚配置寄存器：INT_PIN_CFG，寄存器地址0x37，该寄存器用于设置INT中断引脚的电平标准和驱动方式（推挽、开漏）等；
* 运动状态寄存器：MOT_DETECT_STATUS，寄存器地址0x61，该寄存器用于在触发运动中断（自由落体中断、加速度中断和静止中断）时标示中断的类型。
* 自由落体中断阈值寄存器：FF_THR，寄存器地址0x1D，当使能自由落体中断时，该寄存器的值决定中断的触发阈值，数值越高，对自由落体的检测越不灵敏；
* 自由落体中断时间寄存器：FF_DUR，寄存器地址0x1E，当使能自由落体中断时，该寄存器的值决定中断的持续时间阈值，当自由落体的持续时间达到设定阈值时触发中断。数值越高，对自由落体检测的时间阈值越长；
* 加速度中断阈值寄存器：MOT_THR，寄存器地址0x1F，当使能加速度中断时，该寄存器的值决定中断的触发阈值，数值越高，触发中断所需的加速度越大。
* 加速度中断时间寄存器：MOT_DUR，寄存器地址0x20，当使能加速度中断时，该寄存器的值决定中断的持续时间阈值，当加速度值持续时间达到设定阈值时触发中断。数值越高，触发中断所需的加速度持续时间越长；
* 静止中断阈值寄存器：ZRMOT_THR，寄存器地址0x21，当使能静止中断时，若当前加速度计输出的三轴值均小于静止中断阈值，则产生静止中断。数值越高，触发静止中断的要求越宽松。
* 静止中断时间寄存器：ZRMOT_DUR，寄存器地址0x22，当使能静止中断时，该寄存器的值决定触发静止中断所需的持续时间，数值越高，触发中断所需的静止持续时间越长。

在低功耗监控状态下，需要的配置包括：

* INT_ENABLE  0x38 - 0xf0
* USER_CTRL

//DMP使能=0x80,FIFI使能=0x40,辅助I2C总线主机使能=0x20,

## MPU-IMU

想要高频率的获取数据就用MPU中断，不需要使用时关闭中断，等要数据时打开中断，高频率获取数据再滤波。

编译器相关宏配置：

`EMPL_TARGET_EC718`
`EMPL`
`MPL_LOG_NDEBUG = 0/1` 没有用官方python上位机的话设置为0
`MPU6500`   你使用的型号
`USE_DMP`   是否使用DMP
`REMOVE_LOGGING`    是否使用日志

初始化步骤为：

- 硬件初始化（MPU初始化）
- MPL初始化（MPL 库是 InvenSense Motion Apps 专有算法的核⼼，由 Mllite 和 mpl ⽬录组成。）
- 设置MPL与DMP
- 调用函数获取数据

**注意**：DMP传感器融合仅适用于+ -2000dps和Accel + -2G的陀螺仪，大部分情况下我们都需要使用DMP，除非你需要很大的量程，否则没有必要关闭DMP。

* 支持 MPU9250/6500/6050 驱动和姿态解算
* 通过中断输出数据，提供高频操作，通过定时读取，实现低功耗操作

MPU6500的FIFO缓存可以存储512个16位数据。其中，可以同时存储三轴加速度计、三轴陀螺仪和三轴磁力计数据。

由于这些数据不是同时采集的，因此实际上FIFO缓存的可用空间不是完全相同的。

在MPU6500芯片FIFO使用过程中需要注意以下几个方面：

*  1. 在读取FIFO数据之前，要确保MPU6500芯片已经进入FIFO缓存模式，并且已经配置好数据类型和采样频率。
*  2. 在读取FIFO数据前，要清除FIFO溢出标志。
*  3. 在处理FIFO数据时，要考虑到FIFO数据可能会因为溢出而丢失数据，因此要使用合适的算法来处理数据丢失的情况。
*  4. 为了避免FIFO数据被多次读取，建议在读取完FIFO数据后，立即清除FIFO缓存中的数据。

在2Hz的情况下存在大量噪声和误唤醒

### Motion Driver 6.12


```mpu
├── driver
    ├── eMPL
         ├── inv_mpu.c
         ├── inv_mpu_dmp_motion_driver.c
         ├── ...
    ├── include...
    └── vendor...
├── eMPL-hal...
├── mllite...
├── mpl...
└── README.md

```

文件放置路径在 subsys/sensorhub/mpu，其中适配硬件接口适配依赖新增路径 mpu/driver/vendor，其余路径皆是原文件和路径

相关库工程依赖 subsys/sensorhub/mpu/mpl/liblibmplmpu.a ，需要根据实际芯片选择对应架构，修改工程下Makefile，导入预编译库
```
ifeq ($(SUBSYS_SENSORS_ENABLE),y)
PREBUILDLIBS += $(TOP)/PLAT/subsys/sensorhub/mpu/mpl/liblibmplmpu.a
endif
```

主要适配修改的借口包括 mpu/driver/eMPL/inv_mpu.c 和 mpu/driver/eMPL/inv_mpu_dmp_motion_driver.c中有宏定义以供移植，相关定义在mpu/driver/vendor路径下
```
#define i2c_write       ec_i2c_send
#define i2c_read        ec_i2c_read
#define delay_ms        ec_delay
#define get_ms          ec_ticks
```

### 相关定义

传感器相关配置宏定义在subsys/sensorhub/Makefile.inc

```
SUBSYS_SENSORS_DIR  := $(TOP)/PLAT/subsys/sensorhub
CFLAGS += -DFEATURE_SUBSYS_SENSORHUB_ENABLE 
CFLAGS += -DMPU6500
CFLAGS += -DEMPL
CFLAGS += -DEMPL_TARGET_EC718
CFLAGS += -DUSE_DMP

CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/inc
CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/mpu/driver/eMPL
CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/mpu/driver/vendor
CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/mpu/driver/include
CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/mpu/mllite
CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/mpu/mpl
CFLAGS_INC += -I $(SUBSYS_SENSORS_DIR)/mpu/eMPL-hal

SENSORHUB_SRC_DIRS += $(SUBSYS_SENSORS_DIR)/src 
SENSORHUB_SRC_DIRS += $(SUBSYS_SENSORS_DIR)/mpu/mpl
SENSORHUB_SRC_DIRS += $(SUBSYS_SENSORS_DIR)/mpu/mllite
SENSORHUB_SRC_DIRS += $(SUBSYS_SENSORS_DIR)/mpu/driver/eMPL 
SENSORHUB_SRC_DIRS += $(SUBSYS_SENSORS_DIR)/mpu/driver/vendor 
SENSORHUB_SRC_DIRS += $(SUBSYS_SENSORS_DIR)/mpu/eMPL-hal 
```

有三个函数是关于储存校准数据:
```
inv_error_t inv_get_mpl_state_size(size_t *size);
inv_error_t inv_load_mpl_states(const unsigned char *data, size_t len);
inv_error_t inv_save_mpl_states(unsigned char *data, size_t len);

```