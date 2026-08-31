
#ifndef __SENSOR_MPU_H__
#define __SENSOR_MPU_H__
#ifdef __cplusplus
extern "C" {
#endif
//如果AD0脚(9脚)接地,IIC地址为0X68(不包含最低位).
//如果接V3.3,则IIC地址为0X69(不包含最低位).
#define MPU_ADDR                0X68    

#define MPU6050_WHO_AM_I        0x68 // value of WHO_AM_I register of mpu6050
#define MPU6500_WHO_AM_I        0x70 // value of WHO_AM_I register of mpu6500
#define MPU9250_WHO_AM_I        0x71 // value of WHO_AM_I register of mpu9250
#define ICM20608G_WHO_AM_I      0xAF // value of WHO_AM_I register of icm20608G
#define ICM20608D_WHO_AM_I      0xAE // value of WHO_AM_I register of icm20608D

//MPU9250内部封装了一个AK8963磁力计,地址和ID如下:
#define AK8963_ADDR             0X0C    //AK8963的I2C地址
#define AK8963_ID               0X48    //AK8963的器件ID

//AK8963的内部寄存器
#define AK8963_WHO_AM_I         0x00 // should return 0x48
#define AK8963_INFO             0x01
#define AK8963_ST1              0x02  // data ready status bit 0
#define AK8963_XOUT_L           0x03   
#define AK8963_XOUT_H           0x04
#define AK8963_YOUT_L           0x05
#define AK8963_YOUT_H           0x06
#define AK8963_ZOUT_L           0x07
#define AK8963_ZOUT_H           0x08
#define AK8963_ST2              0x09  // Data overflow bit 3 and data read error status bit 2
#define AK8963_CNTL             0x0A  // Power down (0000), single-measurement (0001), self-test (1000) and Fuse ROM (1111) modes on bits 3:0
#define AK8963_ASTC             0x0C  // Self test control
#define AK8963_I2CDIS           0x0F  // I2C disable
#define AK8963_ASAX             0x10  // Fuse ROM x-axis sensitivity adjustment value
#define AK8963_ASAY             0x11  // Fuse ROM y-axis sensitivity adjustment value
#define AK8963_ASAZ             0x12  // Fuse ROM z-axis sensitivity adjustment value


//MPU6500的内部寄存器
#define MPU_SELF_TESTX_REG      0X0D    //自检寄存器X
#define MPU_SELF_TESTY_REG      0X0E    //自检寄存器Y
#define MPU_SELF_TESTZ_REG      0X0F    //自检寄存器Z
#define MPU_SELF_TESTA_REG      0X10    //自检寄存器A
#define MPU_SAMPLE_RATE_REG     0X19    //采样频率分频器
#define MPU_CFG_REG             0X1A    //配置寄存器
#define MPU_GYRO_CFG_REG        0X1B    //陀螺仪配置寄存器
#define MPU_ACCEL_CFG_REG       0X1C    //加速度计配置寄存器
#define MPU_ACCEL_CFG2_REG      0X1D    //29:加速度计配置寄存器
#define MPU_LP_ACCEL_ODR        0X1E    //30:唤醒频率0.24-500Hz 时间阈值ms
#define MPU_MOTION_DET_REG      0X1F    //运动检测阀值设置寄存器
#define MPU_FIFO_EN_REG         0X23    //FIFO使能寄存器
#define MPU_I2CMST_CTRL_REG     0X24    //IIC主机控制寄存器
#define MPU_I2CSLV0_ADDR_REG    0X25    //IIC从机0器件地址寄存器
#define MPU_I2CSLV0_REG         0X26    //IIC从机0数据地址寄存器
#define MPU_I2CSLV0_CTRL_REG    0X27    //IIC从机0控制寄存器
#define MPU_I2CSLV1_ADDR_REG    0X28    //IIC从机1器件地址寄存器
#define MPU_I2CSLV1_REG         0X29    //IIC从机1数据地址寄存器
#define MPU_I2CSLV1_CTRL_REG    0X2A    //IIC从机1控制寄存器
#define MPU_I2CSLV2_ADDR_REG    0X2B    //IIC从机2器件地址寄存器
#define MPU_I2CSLV2_REG         0X2C    //IIC从机2数据地址寄存器
#define MPU_I2CSLV2_CTRL_REG    0X2D    //IIC从机2控制寄存器
#define MPU_I2CSLV3_ADDR_REG    0X2E    //IIC从机3器件地址寄存器
#define MPU_I2CSLV3_REG         0X2F    //IIC从机3数据地址寄存器
#define MPU_I2CSLV3_CTRL_REG    0X30    //IIC从机3控制寄存器
#define MPU_I2CSLV4_ADDR_REG    0X31    //IIC从机4器件地址寄存器
#define MPU_I2CSLV4_REG         0X32    //IIC从机4数据地址寄存器
#define MPU_I2CSLV4_DO_REG      0X33    //IIC从机4写数据寄存器
#define MPU_I2CSLV4_CTRL_REG    0X34    //IIC从机4控制寄存器
#define MPU_I2CSLV4_DI_REG      0X35    //IIC从机4读数据寄存器
#define MPU_I2CMST_STA_REG      0X36    //IIC主机状态寄存器
#define MPU_INTBP_CFG_REG       0X37    //55:中断/旁路设置寄存器
#define MPU_INT_EN_REG          0X38    //中断使能寄存器
#define MPU_INT_STA_REG         0X3A    //58:中断状态寄存器,BIT6 WOM_INT
#define MPU_ACCEL_XOUTH_REG     0X3B    //加速度值,X轴高8位寄存器
#define MPU_ACCEL_XOUTL_REG     0X3C    //加速度值,X轴低8位寄存器
#define MPU_ACCEL_YOUTH_REG     0X3D    //加速度值,Y轴高8位寄存器
#define MPU_ACCEL_YOUTL_REG     0X3E    //加速度值,Y轴低8位寄存器
#define MPU_ACCEL_ZOUTH_REG     0X3F    //加速度值,Z轴高8位寄存器
#define MPU_ACCEL_ZOUTL_REG     0X40    //加速度值,Z轴低8位寄存器
#define MPU_TEMP_OUTH_REG       0X41    //温度值高八位寄存器
#define MPU_TEMP_OUTL_REG       0X42    //温度值低8位寄存器
#define MPU_GYRO_XOUTH_REG      0X43    //陀螺仪值,X轴高8位寄存器
#define MPU_GYRO_XOUTL_REG      0X44    //陀螺仪值,X轴低8位寄存器
#define MPU_GYRO_YOUTH_REG      0X45    //陀螺仪值,Y轴高8位寄存器
#define MPU_GYRO_YOUTL_REG      0X46    //陀螺仪值,Y轴低8位寄存器
#define MPU_GYRO_ZOUTH_REG      0X47    //陀螺仪值,Z轴高8位寄存器
#define MPU_GYRO_ZOUTL_REG      0X48    //陀螺仪值,Z轴低8位寄存器

#define MPU_I2CSLV0_DO_REG      0X63    //IIC从机0数据寄存器
#define MPU_I2CSLV1_DO_REG      0X64    //IIC从机1数据寄存器
#define MPU_I2CSLV2_DO_REG      0X65    //IIC从机2数据寄存器
#define MPU_I2CSLV3_DO_REG      0X66    //IIC从机3数据寄存器
#define MPU_I2CMST_DELAY_REG    0X67    //IIC主机延时管理寄存器
#define MPU_SIGPATH_RST_REG     0X68    //信号通道复位寄存器
#define MPU_MDETECT_CTRL_REG    0X69    //运动检测控制寄存器
#define MPU_USER_CTRL_REG       0X6A    //用户控制寄存器
#define MPU_PWR_MGMT1_REG       0X6B    //电源管理寄存器1 0x49 - 睡眠
#define MPU_PWR_MGMT2_REG       0X6C    //电源管理寄存器2
#define MPU_FIFO_CNTH_REG       0X72    //FIFO计数寄存器高八位
#define MPU_FIFO_CNTL_REG       0X73    //FIFO计数寄存器低八位
#define MPU_FIFO_RW_REG         0X74    //FIFO读写寄存器
#define MPU_DEVICE_ID_REG       0X75    //器件ID寄存器


#define FF_THR                  0X1D    //自由落体中断阈值寄存器
#define FF_DUR                  0X1E    //自由落体中断时间寄存器
#define MOT_THR                 0X1F    //加速度中断阈值寄存器,1LSB = 2mg,加速度计的任意一个轴的数据超过该阈值时就会触发自由落体中断计时，若超过阈值的加速度持续时间超过自由落体中断计时时间，就会产生自由落体中断
#define MOT_DUR                 0X20    //加速度中断时间寄存器
#define ZRMOT_THR               0X21    //静止中断阈值寄存器,1LSB = 2mg,加速度计的三个轴的数据均小于该阈值时就会触发静止中断计时，若静止中断计时结束后加速度计值均未超过阈值，就会产生静止中断
#define ZRMOT_DUR               0X22    //静止中断时间寄存器,1 LSB = 64 ms
#define MOT_DETECT_STATUS       0X61    //运动状态寄存器

typedef struct
{
    int8_t accuracy;    //磁力计没校准accuracy为0，校准后为3
    int16_t temp;
    int16_t yaw;       //偏航角,单位度
    int16_t roll;      //翻滚角,单位度
    int16_t pitch;     //俯仰角,单位度
    long euler[3];  
} imu_data_t;



uint8_t sensor_mpu_init(void);
uint8_t sensor_mpu_wake(void);
uint8_t sensor_mpu_loop(imu_data_t *data);

#ifdef __cplusplus
}
#endif
#endif
