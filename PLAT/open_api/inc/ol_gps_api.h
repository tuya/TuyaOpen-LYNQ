#ifndef _MBTK_AT_GPS_H_
#define _MBTK_AT_GPS_H_
#include "Driver_USART.h"
#include "bsp_usart.h"

#define GPS_TASK_STACK_SIZE    (6144)//(4096)

//#define MBTK_GPS_OPEN_CMD "$PAIR002*38\r\n"
//#define MBTK_GPS_CLOSE_CMD "$PAIR003*39\r\n"
#define MBTK_GPS_HOT_START "F1D906400100034A24"
#define MBTK_GPS_WARM_START "F1D906400100024923"
#define MBTK_GPS_COLD_START "F1D906400100014822"
#define MBTK_GPS_GPS_ONLY "F1D9060C04000100000017A0"
#define MBTK_GPS_BDS_ONLY "F1D9060C0400044000005A6C"
#define MBTK_GPS_GPS_AND_BDS "F1D9060C0400254000007BF0"
#define MBTK_GPS_RATE_1HZ "F1D9064410000030010001000000E80300000000000077E3"
#define MBTK_GPS_RATE_2HZ "F1D9064410000000010002000000F4010000000000005271"
#define MBTK_GPS_RATE_5HZ "F1D9064410000000010005000000C800000000000000282E"
#define MBTK_GPS_RATE_10HZ "F1D906441000000001000A0000006400000000000000C94A"




//#define EPO_DOWNLOAD_TIME "EPO_download_time.txt"

typedef enum
{
    MBTK_GNSS_POWEROFF = 0,
    MBTK_GNSS_POWERON, // 1
    MBTK_GNSS_OPER_MAX ,
}Mbtk_gnss_oper_enum;



////gps 相关
 typedef struct
{
	uint8_t num;		
	//卫星编号
	uint8_t eledeg;	
	//卫星仰角
	uint16_t azideg;	
	//卫星方位角
	uint8_t sn;		
	//信噪比		   
}nmea_satellitemsg;
//北斗 NMEA-0183协议重要参数结构体定义 
//卫星信息
 typedef struct
{
	 uint8_t beidou_num;		
	 //卫星编号
	uint8_t beidou_eledeg;	
	 //卫星仰角
	uint16_t beidou_azideg;	
	 //卫星方位角
	uint8_t beidou_sn;		
	 //信噪比		   
}beidou_nmea_satellitemsg;

//UTC时间信息
 typedef struct
{
	uint8_t year;	//年份
	uint8_t month;	//月份
	uint8_t date;	//日期
	uint8_t hour; 	//小时
	uint8_t min; 	//分钟
	uint8_t sec; 	//秒钟
}nmea_utc_time;

 typedef struct nmea_msg
{
	uint8_t svnum;					
	//可见GPS卫星数
	uint8_t beidou_svnum;			
	//可见GPS卫星数
	nmea_satellitemsg slmsg[12];		
	//最多12颗GPS卫星
	beidou_nmea_satellitemsg beidou_slmsg[12];		
	//暂且算最多12颗北斗卫星
	nmea_utc_time utc;			
	//UTC时间
	uint8_t valid_states;
	//状态
	uint32_t latitude;				
	//纬度 分扩大100000倍,实际要除以100000
	uint8_t nshemi;					
	//北纬/南纬,N:北纬;S:南纬				  
	uint32_t longitude;			   
	//经度 分扩大100000倍,实际要除以100000
	uint8_t ewhemi;					
	//东经/西经,E:东经;W:西经
	uint8_t gpssta;					
	//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
	uint8_t posslnum;				
	//用于定位的卫星数,0~12.(GPS和BD一共的卫星数)
	uint8_t possl[12];				
	//用于定位的gps卫星编号
	uint8_t posslbd[12];				
	//用于定位的BD卫星编号	
	uint8_t fixmode;					
	//定位类型:1,没有定位;2,2D定位;3,3D定位
	uint16_t pdop;					
	//位置精度因子 0~500,对应实际值0~50.0
	uint16_t hdop;					
	//水平精度因子 0~500,对应实际值0~50.0
	uint16_t vdop;					
	//垂直精度因子 0~500,对应实际值0~50.0 
	uint16_t azi;
	//方位角,放大了100倍,实际除以100
	int altitude;			 	
	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	uint32_t speed;					
	//地面速率,放大了1000倍,实际除以1000.单位:0.001公里/小时
	uint16_t cog;
	//地面方位,放大了100倍,实际除以100,对应实际值000.00~359.99
}nmea_msg;
 //m^n函数
 //返回值:m^n次方.

 typedef void(*mbtk_gps_nmea)(char *nmea);




 typedef enum 
 {
	 mbtk_gps_err = -1,
	 mbtk_gps_success = 0
 }mbtk_gps_return_enum;

 

  typedef enum
  {
	  MBTK_GNSS_CMD_NONE = 0,
	  MBTK_GNSS_CMD_COLDSTART,      //冷启
	  MBTK_GNSS_CMD_WARMSTART,      //温启
      MBTK_GNSS_CMD_HOTSTART,       //热启
	  MBTK_GNSS_CMD_PARSE_BY_USER,  //nmea数据如果设置了ol_gps_set_gps_nmea_cb，可以设置此项，此项可以关闭内部解析nmea数据，也就是导致ol_get_gps_info数据不会再更新
	  MBTK_GNSS_CMD_PARSE_BY_M,     //模块解析nmea数据，和MBTK_GNSS_CMD_PARSE_BY_USER相反
	  MBTK_GNSS_CMD_POS_SYS_TYPE,   //定位系统选择
	  MBTK_GNSS_CMD_GPS_RATE,       //
      MBTK_GNSS_CMD_MAX
  }Mbtk_gnss_cmd_enum;



typedef struct
{
	nmea_msg  nmea;
	int state;
}mbtk_gps_info;


typedef enum 
{
	MBTK_E,NONE,
	MBTK_E_PARAM = -1,
	MBTK_E_ALREADY_STARTED = -2,
	MBTK_E_ERROR = -3,
	MBTK_E_NOT_RUNING = -4,
}mbtk_gps_error;


//mbtk_gps_info 实际返回mbtk_gps_info
void *ol_get_gps_info(void);
int ol_gps_power(int on_off);
int ol_gps_operation(int cmd);
int ol_set_gps_nmea_cb(mbtk_gps_nmea nmea_cb);
void ol_prepare_gps_data(void *pbuf, uint32_t len);
int ol_gps_set_pos_sys_type(INT32 naviSys);
int ol_gps_set_gps_rate(INT32 naviRate);


#endif
