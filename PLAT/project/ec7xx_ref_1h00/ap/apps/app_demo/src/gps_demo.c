/****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include "ol_log.h"
#include "cmsis_os2.h"
#include "ol_gps_api.h"


static void mbtk_gps_nmea_cb(char *nmea)
{
	OL_LOG_PRINTF("%s", nmea);
}





void gps_demo(void)
{
  int ret;
  mbtk_gps_info *info;
  int status = 0;

  ol_gps_power(MBTK_GNSS_POWERON);

  info = ol_get_gps_info();
  if (!info){
      OL_LOG_PRINTF("gps not run");
      return;
  }


	ol_set_gps_nmea_cb(mbtk_gps_nmea_cb);
	ol_gps_operation(MBTK_GNSS_CMD_PARSE_BY_USER);
	ol_gps_set_pos_sys_type(3);


	//ol_gps_operation不设置MBTK_GNSS_CMD_PARSE_BY_USER或者设置MBTK_GNSS_CMD_PARSE_BY_M，则会进行内部解析
  while (1)
  {
  
		OL_LOG_PRINTF("Locate:valid states %d,UTC time %d/%d/%d %d:%d:%d",
			info->nmea.valid_states,info->nmea.utc.year,info->nmea.utc.month,info->nmea.utc.date,
			info->nmea.utc.hour,info->nmea.utc.min,info->nmea.utc.sec);

  		OL_LOG_PRINTF("Locate:%c %d, %c %d\n", 
			info->nmea.nshemi,info->nmea.latitude,info->nmea.ewhemi,info->nmea.longitude);

		OL_LOG_PRINTF("Locate:speed %d,altitude %d,cog %d,azi %d\n", 
			info->nmea.speed,info->nmea.altitude,info->nmea.ewhemi,info->nmea.longitude);	

		OL_LOG_PRINTF("Locate:gps states %d,fixmode %d,pdop %d,hdop %d,vdop %d\n", 
			info->nmea.gpssta,info->nmea.fixmode,info->nmea.cog,info->nmea.azi);

       OL_LOG_PRINTF("Locate:gps serial %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", 
				info->nmea.possl[0],info->nmea.possl[1],info->nmea.possl[2],info->nmea.possl[3],
        info->nmea.possl[4],info->nmea.possl[5],info->nmea.possl[6],info->nmea.possl[7],
        info->nmea.possl[8],info->nmea.possl[9],info->nmea.possl[10],info->nmea.possl[11]);
			
       OL_LOG_PRINTF("BeiDou index 0 num %d\n", info->nmea.beidou_slmsg[0].beidou_num);
     
  	  osDelay(1000);
  }
    return;
}



