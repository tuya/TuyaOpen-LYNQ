#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "ui.h"
#include "mode_config.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#include "rng.h"
#include "storage.h"
#include "cmsis_os2.h"
#include "mm_jpeg_if.h"
#include "servicemanager.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lv_port_disp.h"
#include "lv_port_fs.h"

extern uint32_t lv_font_extern_init(void);
extern void *ui_recorder_set(fpui_recorder_t *data);

lv_disp_t *dispp = NULL;
lv_theme_t *theme = NULL;
osEventFlagsId_t uiEvtHandle = NULL; 
osThreadId_t guiTaskHandle = NULL;
// lv_font_t *pLight36 = NULL;
lv_font_t *pSystem16 = NULL;
lv_font_t *pSystem66 = NULL;
osMessageQueueId_t GuiMsgQ = NULL;

static uint8_t testItem = 0;
#define EPAT_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_TEST, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 
/**
  \fn
  \brief
  \return
*/
void guiMsgQInit(void)
{
    // memset(&gMsgQ, 0, sizeof(gMsgQ));
	GuiMsgQ = osMessageQueueNew(GUI_MSG_QUEUE_SIZE, sizeof(GuiMsgT), NULL);
    if (GuiMsgQ == NULL)
    {
        UI_ERR("Failed to create queue for ApphubQ.\r\n");
    }
}
/**
  \fn
  \brief
  \return
*/
int32_t guiSendMsg(GuiMsgT* msgPtr)
{
	osMessageQueuePut(GuiMsgQ, msgPtr, 0, 0);
    return 0;
}

/**
  \fn
  \brief
  \return
*/
const char *fpui_bat_arr[] = {
    LV_IMAGE_22,
    LV_IMAGE_23,
    LV_IMAGE_24,
    LV_IMAGE_25,
    LV_IMAGE_26,
    LV_IMAGE_27,
    LV_IMAGE_13,//充电图标
    LV_IMAGE_14,
    LV_IMAGE_15,
    LV_IMAGE_16,
    LV_IMAGE_17,
    LV_IMAGE_18,
};
const char *fpui_sig_arr[] = {
    LV_IMAGE_35,
    LV_IMAGE_36,
    LV_IMAGE_37,
    LV_IMAGE_38,
    LV_IMAGE_39,
};
const char *fpui_menu_arr[] = {
    "D:/MM_01.jpg",
    "D:/MM_02.jpg",
    "D:/MM_03.jpg",
    "D:/MM_04.jpg",
    "D:/MM_05.jpg",
    "D:/MM_06.jpg",
};
/**
  \fn
  \brief
  \return
*/
uint32_t conv_jpeg_rgb565(char *name,lv_img_dsc_t *img)
{
    if ((name == NULL) || (img == NULL)){
        UI_LOG("Param error\r\n");
        return 0;
    }
    FILE *file = file_fopen(name, "r");
    if (file == NULL){
        UI_LOG("Failed to open the file \"%s\"\r\n", name);
        return 0;
    }
    struct stat  buf = {0};
    file_fstat((int)file, &buf);
    #if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    void *jpeg = pvPortMalloc_Psram(buf.st_size);
    #else 
    //if no psram
    void *jpeg = malloc(buf.st_size);
    #endif
    file_fread(jpeg, buf.st_size, 1, file);
    if (file != NULL){
        file_fclose(file);
    }
    // UI_LOG("read the file \"%s\" %d\r\n",name,buf.st_size);
    JPEG_INFO info;
    void *pdec = JpegD_Create();
    int32_t res = JpegD_DecodeInfo(pdec,jpeg,buf.st_size,&info);
    res = res;
    JPEG_IMAGE_BUF ibuf;
    ibuf.eFmt = JPEG_COLOR_FMT_RGB565;
    ibuf.pData[0] = (void *)img->data;
    ibuf.uWidth = info.uWidth;
    ibuf.uHeight = info.uHeight;
    img->header.cf = LV_IMG_CF_TRUE_COLOR;
    img->header.w = info.uWidth;
    img->header.h = info.uHeight;
    img->data_size = info.uWidth*info.uHeight*2;
    img->header.always_zero = 0;
    img->header.reserved = 0;
    // UI_LOG("JpegD_DecodeImage %d %d,%d\r\n",res,info.uWidth,info.uHeight);
    res = JpegD_DecodeImage(pdec,&ibuf);
    if(pdec != NULL){
        // UI_LOG("JpegD_Destroy %d 0x%x\r\n",res,pdec);
        JpegD_Destroy(pdec);
    }
    
    if(jpeg != NULL){
    #if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(jpeg);
    #else 
    //if no psram
        free(jpeg);
    #endif
    }

    return buf.st_size;
}
/**
  \fn
  \brief
  \return
*/
uint8_t ui_auto_update(lv_obj_t * ui)
{
    if(ui == ui_Home && lv_obj_is_valid(ui_Home))
    {
        return ui_home_auto_update();
    }
    return 0;
}

/**
  \fn
  \brief
  \return
*/
PLAT_FPSRAM_ZI_CUST uint32_t MesStep[MES_STEP_ALL];
void timeMes(TimeMesIndex_e index)
{
    if(index == MES_START){
        for(uint8_t i=0;i<MES_STEP_ALL;i++){
            // MesStep[i] = millis();
            MesStep[i] = 0;
        }
        MesStep[0] = millis();
    } 
    else if(index < MES_STEP_ALL){
        // MesStep[index] =  millis() - MesStep[index];
        MesStep[index] =  millis();
    }
    else if(index == MES_END){
        if(MesStep[0]) UI_LOG("\n\rtimeMes[%d]:",(millis() - MesStep[0])); 
        for(uint8_t i=1;i<MES_STEP_ALL;i++){
            if(MesStep[i]){
                UI_LOG("[%d]%dms ",i,MesStep[i]-MesStep[0]);
                MesStep[i] = 0;
            }
        }
        // UI_LOG("\n\r"); 
    }  
}
void memoryMes(uint32_t *sram,uint32_t *psram)
{
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
    uint32_t total_heap = xPortGetTotalHeapSize();
    uint32_t curr_free_heap = xPortGetFreeHeapSize();
    uint32_t min_free_heap=xPortGetMinimumEverFreeHeapSize();
    uint32_t max_free_block=xPortGetMaximumFreeBlockSize();
    uint32_t total_heap_Psram =xPortGetTotalHeapSize_Psram();
    uint32_t curr_free_heap_Psram=xPortGetFreeHeapSize_Psram();
    uint32_t min_free_heap_Psram=xPortGetMinimumEverFreeHeapSize_Psram();
    uint32_t max_free_block_Psram=xPortGetMaximumFreeBlockSize_Psram();
    UI_LOG("%d/%d,[%d,%d],%d/%d,[%d,%d]",curr_free_heap/1024,total_heap/1024,min_free_heap,\
    max_free_block,curr_free_heap_Psram/1024,total_heap_Psram/1024,min_free_heap_Psram,max_free_block_Psram);
#endif
}
/**
  \fn
  \brief
  \return
*/
static uint8_t used_pct_max = 0;
static uint8_t frag_pct_max = 0;
static uint32_t used_mem_max = 0;
static uint32_t used_time_max = 0;
static uint32_t task_time_max = 0;
static uint32_t gui_used_time = 0;
static void guiTask(void *argument)
{
    uint32_t ret = lv_font_extern_init();
    if(ret == 0) {
        UI_ERR("Failed load extern Font\r\n");
    }
    pSystem66 = lv_font_load(LV_FONT_SYSTEM_66);
    if(pSystem66 == NULL) {
        UI_ERR("Failed to load Font System66\r\n");
    }
    pSystem16 = lv_font_load(LV_FONT_SYSTEM_16);
    if(pSystem16 == NULL) {
        UI_ERR("Failed to load Font System16\r\n");
    }
    GuiMsgT msg;
    uint32_t _runcnt = 0;
    uint32_t used_mem = 0;
    lv_mem_monitor_t mon;
    uint32_t round = 0;
    uint32_t used_time = 0;
    uint32_t taskms = 0;
    gui_used_time = 0;
    volatile uint32_t costms = 0;
    uint32_t timeout = osWaitForever;
    UI_LOG("Task ready!\t%d/%d,%s,%s",millis(),osKernelGetTickCount(),__DATE__,__TIME__);
    while (1)
    {
        uint16_t addmore = 0;
        uint8_t printflag = 0;
        memset(&msg, 0, sizeof(GuiMsgT));
        osStatus_t stat = osMessageQueueGet(GuiMsgQ, &msg, 0, timeout);
        if (stat == osOK){
            #if UI_TEST_ITEMS || UI_TEST_ITEM
            UI_LOG("[%d-%d],%dms,%d(≤%d)ms,%d(≤%d)ms,%dKB,%d%%,%d%%,0x%x",round,testItem,(gui_used_time),(used_time),\
                    used_time_max,(taskms),task_time_max,used_mem_max/1024,used_pct_max,frag_pct_max,lv_scr_act());
            #endif
            timeout = (msg.refresh_ms > 3) ? msg.refresh_ms : 10;
            costms = millis();
            if(msg.ui_set != NULL){
                msg.ui_set(msg.ui_data);
                round ++;
                used_time = (millis()-costms);
                if(used_time_max < used_time) {
                    addmore = used_time-used_time_max;
                    UI_LOG("[%d-%d],%dms,%d->%d(+%d)ms,%dKB,%d%%",round,testItem,gui_used_time,\
                            used_time_max,used_time,addmore,used_mem_max/1024,used_pct_max);
                    used_time_max = used_time;
                    printflag |= 0x10;
                }
                gui_used_time += used_time;
                osDelay(5);
            }
        }
        if((_runcnt++)%100 == 0 ){
            osDelay(5);
            costms = millis();
            uint8_t ret = ui_auto_update(lv_scr_act());
            if(ret){
                EPAT_TRACE(auto_update, 8, "ret 0x%X,round %d,now %d,max %d ms,%d Bytes,frag %d,pct %d",ret,round, \
                    (millis()/1000),used_time_max,task_time_max,used_mem_max,frag_pct_max,used_pct_max);
            } 
        }
        if (lv_is_initialized()) {
            osDelay(5);
            costms = millis();
            lv_task_handler();
            taskms = millis()-costms;
            gui_used_time += taskms;
            if(task_time_max < taskms) {
                addmore = taskms-task_time_max;
                printflag |= 0x20;
                UI_LOG("[%d-%d],%d/%d,%d(≤%d)ms,%d->%d(+%d)ms",round,testItem,gui_used_time/1000,\
                        millis()/1000,used_time,used_time_max,task_time_max,taskms,addmore);
                task_time_max = taskms;
            }
        }
        if (stat == osOK){
            if(msg.ui_del != NULL){
                msg.ui_del(msg.ui_data);
            }
        }
        lv_mem_monitor(&mon);
        used_mem = mon.total_size - mon.free_size;
        if(used_pct_max < mon.used_pct) {
            addmore = mon.used_pct - used_pct_max;
            // UI_LOG("%d[%d],%dms(+%d),\t%d Bytes(<%d),pct %d%%->%d%%(+%d%%),frag %d%%(<%d)",round,testItem,gui_used_time,(used_time+taskms),used_mem,used_mem_max, used_pct_max,mon.used_pct,addmore, mon.frag_pct,frag_pct_max);
            used_pct_max = mon.used_pct;
            printflag |= 0x1;
        }
        if(frag_pct_max < mon.frag_pct) {
            addmore = mon.frag_pct - frag_pct_max;
            UI_LOG("[%d-%d],%d(+%d)ms,%d KB(≤%d),pct %d%%(≤%d),frag %d%%->%d%%(+%d%%)",round,testItem,gui_used_time,\
                (used_time+taskms),used_mem/1024,used_mem_max/1024,mon.used_pct,used_pct_max,frag_pct_max,mon.frag_pct,addmore);
            frag_pct_max = mon.frag_pct;
            printflag |= 0x2;
        }
        if(used_mem_max < used_mem) {
            printflag |= 0x4;
            addmore = used_mem - used_mem_max;
            if(addmore > 200){
                UI_WRN("[%d-%d],%d(+%d)ms,%dKB(+%dB),pct %d%%(≤%d),frag %d%%(≤%d)",round,testItem,gui_used_time,\
                    (used_time+taskms),used_mem/1024,addmore,mon.used_pct,used_pct_max,mon.frag_pct,frag_pct_max);  
            }else{
                UI_LOG("[%d-%d],%d(+%d)ms,%dKB(+%dB),pct %d%%(≤%d),frag %d%%(≤%d)",round,testItem,gui_used_time,\
                    (used_time+taskms),used_mem/1024,addmore,mon.used_pct,used_pct_max,mon.frag_pct,frag_pct_max);  
            }
            used_mem_max = used_mem;
        }
        if(printflag){
            EPAT_TRACE(guiTask, 8, "flag 0x%X,round %d,%d ms,usedmax %d ms,taskmax %d ms,%d Bytes,frag %d,pct %d", \
                printflag,round,(used_time+taskms),used_time_max,task_time_max,used_mem_max,frag_pct_max,used_pct_max);
        }
    }
}
/**
  \fn
  \brief
  \return
*/
#if UI_TEST_ITEMS
static void guiTest1(void *argument)
{
    osDelay(500);
    static uint8_t info[100] = {0};
    uint8_t testItems[] = UI_TEST_ARRAY;
    uint32_t testCount[UI_TEST_ITEMS] = {0};
    uint8_t testItemNum = sizeof(testItems)/sizeof(testItems[0]);
    UI_LOG("Task ready!\t%d,%d/%d,%d,%s,%s",LV_LOG_LEVEL,millis(),osKernelGetTickCount(),testItemNum,__DATE__,__TIME__);
    uint8_t Rand[24] = {0};
    uint32_t costTime = 0;
    uint32_t testRound = 0;
    uint32_t testItemCnt = 0;
    uint32_t testCostms = millis();
    while (1)
    {
        rngGenRandom(Rand);
        testItem = testItems[Rand[0]%testItemNum];
        switch(testItem) {
            case 0:
                osDelay(Rand[0]);
                ui_home_test(fpui_bat_arr[Rand[1]%12],fpui_sig_arr[Rand[2]%5],LV_IMAGE_09);
                break;
            case 1:
                osDelay(Rand[0]);
                char menu_title[16] = {0};
                sprintf(menu_title,"测试-%04X",testRound);
                ui_menu_test(menu_title,fpui_menu_arr[Rand[1]%6]);
                break;
            case 2:
                osDelay(Rand[0]);
                osDelay(Rand[0]);
                char list_title[16] = {0};
                sprintf(list_title,"测试%04d",testRound);
                uint8_t index = Rand[1]%100;
                uint8_t total = index + Rand[3]%100 + 1;
                uint8_t select = total - index;
                ui_list_test(index,select%MAX_LIST_ITEMS,total,list_title,NULL);
                break;
            case 3:
                osDelay(Rand[0]);
                char call_numstr[16] = {0};
                sprintf(call_numstr,"%d%d%d%d",Rand[1],Rand[2],Rand[3],Rand[4]);
                ui_call_test(NULL,Rand[1]%5,call_numstr);
                break;
            case 4:
                osDelay(Rand[0]);
                char play_title[10] = {0};
                sprintf(play_title,"%d.mp3",testRound);
                ui_player_test(play_title,LV_IMAGE_40);
                break;
            case 5:
                osDelay(Rand[0]);
                char test_title[20] = {0};
                sprintf(test_title,"紧急呼叫%04d",testRound);
                ui_headinfo_set(test_title);
                break;
            case 6:
                osDelay(Rand[0]);
                if(testRound%2 == 0){
                    ui_bar_test(Rand[1]%100,((Rand[2])%100+20),(20-Rand[4]%40),(80-Rand[5]%160),(16),(160),Rand[9]*Rand[10]);
                }
                else{
                    ui_bar_test(Rand[1]%100,((Rand[2])%100+20),(20-Rand[4]%40),(80-Rand[5]%160),(160),(16),Rand[9]*Rand[10]);
                }
                break;
            case 7:
                osDelay(Rand[0]);
                if(testItemCnt%POP_KEEP_NUM == 0){
                    ui_listpop_set(NULL);
                }
                else{
                    testItemCnt++;
                }
                break;
            case 8:
                osDelay(Rand[0]);
                if(testItemCnt%POP_KEEP_NUM == 0){
                    ui_message_set(NULL);
                }
                else{
                    ui_message_test("弹窗消息",(Rand[1]-Rand[5])%20,(Rand[2]-Rand[6])%20,\
                            (Rand[7])%100+140,(Rand[8])%100+40,Rand[9]*Rand[10],Rand[11]%2);
                    testItemCnt++;
                }
                break;
            case 9:
                osDelay(Rand[0]);
                char file_title[16] = {0};
                sprintf(file_title,"File-%04X",testRound);
                // uint8_t start = Rand[1]%10;
                // uint8_t sel = Rand[2]%MAX_LIST_ITEMS;
                if(testRound%2 == 0)
                {
                    ui_file_test(file_title,"C:/",Rand[1]%6,Rand[2]%MAX_LIST_ITEMS,Rand[9]*Rand[10]);
                }
                else {
                    ui_file_test(file_title,"D:/",Rand[1]%31,Rand[2]%MAX_LIST_ITEMS,Rand[9]*Rand[10]);
                }
                break;
            case 10:
                osDelay(Rand[0]);
                if(Rand[1]%4 == 0){
                    ui_recorder_test(NULL,0);
                    // osDelay(3000);
                }
                else{
                    char recorder_name[MAX_TITLE_LENGTH] = {0};
                    sprintf(recorder_name,"0x%04X.wav",testRound);
                    ui_recorder_test(recorder_name,0);
                    // osDelay(3000);
                }
                break;
            default:
                osDelay(1000);
                break;
        }
        osDelay(Rand[10]);
        testCount[testItem]++;
        testRound++;
        if(testRound%100 == 0){
            memset(info,0,sizeof(info));
            sprintf(info,"{[%d]",UI_TEST_ITEMS);
            for(int i = 0;i<UI_TEST_ITEMS; i++){
                char temp[8] = {0};
                sprintf(temp,",%d",testCount[i]);
                strcat(info,temp);
            }
            costTime = gui_used_time - costTime;
            testCostms = (millis()-testCostms);
            UI_LOG("%d,%ds,%d/%d,%s}",testRound,millis()/1000,costTime,testCostms,info);
            memoryMes(NULL,NULL);
            costTime = gui_used_time;
            testCostms = millis();
        }
    }
}
#endif
/**
  \fn
  \brief
  \return
*/
#ifdef UI_TEST_ITEM
static void guiTest2(void *argument)
{
    osDelay(500);
    UI_LOG("Task ready!\t%d,%d/%d,%s,%s",LV_LOG_LEVEL,millis(),osKernelGetTickCount(),__DATE__,__TIME__);
    uint8_t onlyItem = UI_TEST_ITEM;
    uint32_t testRound = 0;
    uint32_t testCostms = millis();
    uint8_t Rand[24] = {0};
    #ifndef UI_TEST_ITEMS
    testItem = UI_TEST_ITEM;
    #endif
    uint32_t testItemCnt = 0;
    uint32_t costTime = 0;
    ui_home_test(fpui_bat_arr[1],fpui_sig_arr[2],LV_IMAGE_09);
    while (1)
    {
        rngGenRandom(Rand);
        switch(onlyItem) {
            case 0:
                osDelay(Rand[0]);
                ui_home_test(fpui_bat_arr[Rand[1]%12],fpui_sig_arr[Rand[2]%5],LV_IMAGE_09);
                break;
            case 1:
                osDelay(Rand[0]);
                char menu_title[16] = {0};
                sprintf(menu_title,"MENU-%04X",testRound);
                ui_menu_test(menu_title,fpui_menu_arr[Rand[1]%6]);
                break;
            case 2:
                osDelay(Rand[0]);
                char list_title[16] = {0};
                sprintf(list_title,"List-%04X",testRound);
                uint8_t index = Rand[1]%100;
                uint8_t total = index + Rand[3]%100 + 1;
                uint8_t select = total - index;
                ui_list_test(index,select%MAX_LIST_ITEMS,total,list_title,NULL);
                break;
            case 3:
                osDelay(Rand[0]);
                char call_numstr[16] = {0};
                sprintf(call_numstr,"%d%d%d%d",Rand[1],Rand[2],Rand[3],Rand[4]);
                ui_call_test(NULL,Rand[1]%5,call_numstr);
                break;
            case 4:
                osDelay(Rand[0]);
                char play_title[10] = {0};
                sprintf(play_title,"%d.mp3",testRound);
                ui_player_test(play_title,LV_IMAGE_40);
                break;
            case 5:
                osDelay(Rand[0]);
                char test_title[20] = {0};
                sprintf(test_title,"紧急呼叫%04d",testRound);
                ui_headinfo_set(test_title);
                break;
            case 6:
                osDelay(Rand[0]);
                ui_bar_test(Rand[1]%100,((Rand[2])%100+20),(20-Rand[4]%40),(80-Rand[5]%160),(160),(16));
                break;
            case 7:
                osDelay(Rand[0]);
                testItemCnt++;
                if(testItemCnt%10){
                    // ui_listpop_test(4,Rand[2]%4,(testItemCnt%10)*16-80,(testItemCnt%10)*16-80,140,80,Rand[9]*Rand[10],Rand[11]%2);
                    osDelay(400);
                }
                else{
                    ui_listpop_set(NULL);
                    osDelay(1000);
                }
                break;
            case 8:
                osDelay(Rand[0]);
                testItemCnt++;
                if(testItemCnt%10){
                    ui_message_test("弹窗消息",(testItemCnt%10)*16-80,(testItemCnt%10)*16-80,140,80,Rand[9]*Rand[10],Rand[11]%2);
                    osDelay(400);
                }
                else{
                    ui_message_set(NULL);
                    osDelay(1000);
                }
                break;
            case 9:
                osDelay(Rand[0]);
                char file_title[16] = {0};
                sprintf(file_title,"File-%04X",testRound);
                if(testRound%2 == 0)
                {
                    ui_file_test(file_title,"C:/",1,testRound%MAX_LIST_ITEMS,0x0072);
                }
                else {
                    ui_file_test("根目录",".",0,1,Rand[9]*Rand[10]);
                }
                osDelay(1000);
                break;
            case 10:
                osDelay(Rand[0]);
                char recorder_name[MAX_TITLE_LENGTH] = {0};
                if(Rand[1]%4 == 0){
                    ui_recorder_test(NULL,0x0072);
                    osDelay(3000);
                }
                else if(Rand[1]%4 == 1){
                    ui_recorder_set(NULL);
                }
                else if(Rand[1]%4 == 2){
                    sprintf(recorder_name,"0x%04X.wav",testRound);
                    ui_recorder_test(recorder_name,0x0072);
                    osDelay(3000);
                }
                else{
                    sprintf(recorder_name,"keep.wav");
                    ui_recorder_test(recorder_name,0x0072);
                    osDelay(3000);
                }
                break;
            default:
                osDelay(1000);
                break;
        }
        testRound++;
        if(testRound%100 == 0){
            costTime = gui_used_time - costTime;
            testCostms = (millis()-testCostms);
            UI_WRN("[%d],%ds,%d/%d",testRound,millis()/1000,costTime,testCostms);
            memoryMes(NULL,NULL);
            costTime = gui_used_time;
            testCostms = millis();
        }
    }
}
#endif

void guiInit(void)
{
    guiMsgQInit();
    uiEvtHandle = osEventFlagsNew(NULL);
    lv_init();
    lv_port_disp_init();
    lv_port_fs_init();
    dispp = lv_disp_get_default();
    theme = lv_theme_basic_init(dispp);
    lv_disp_set_theme(dispp, theme);
    osThreadAttr_t gui_attr = {0};
    gui_attr.name = "guiTask";
    gui_attr.stack_size = SUBSYS_GUI_TASK_STACK_SIZE;
    gui_attr.priority = osPriorityNormal;
    gui_attr.stack_mem = subsys_gui_task_stack;
    gui_attr.cb_mem = &subsys_gui_task;
    gui_attr.cb_size = sizeof(StaticTask_t);
    memset(subsys_gui_task_stack, 0xA5, SUBSYS_GUI_TASK_STACK_SIZE);
#if 0
    guiTaskHandle = osThreadNew(guiTask, NULL, &gui_attr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", gui_attr.name);
    Service_reg(serviceName, guiTask, NULL, gui_attr.cb_mem, gui_attr.cb_size, gui_attr.stack_mem, gui_attr.stack_size, gui_attr.priority);
    guiTaskHandle = (osThreadId_t)Service_start(serviceName);
#endif
#ifdef UI_TEST_ITEMS
    osThreadAttr_t threadAttr = {0};
    threadAttr.name       = "guiTest1";
    threadAttr.stack_size = 2 * 1024;
    threadAttr.priority   = osPriorityNormal;
#if 0
    osThreadNew(guiTest1, NULL, &threadAttr); 
#else
    char serviceName2[32] = {0};
    snprintf(serviceName2, sizeof(serviceName2), "service:/%s", threadAttr.name);
    Service_reg(serviceName2, guiTest2, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    Service_start(serviceName2);
#endif
#endif
#ifdef UI_TEST_ITEM
    osThreadAttr_t thread2Attr = {0};
    thread2Attr.name       = "guiTest2";
    thread2Attr.stack_size = 2 * 1024;
    thread2Attr.priority   = osPriorityNormal;
#if 0
    osThreadNew(guiTest2, NULL, &thread2Attr);
#else
    char serviceName3[32] = {0};
    snprintf(serviceName3, sizeof(serviceName3), "service:/%s", thread2Attr.name);
    Service_reg(serviceName3, guiTest2, NULL, thread2Attr.cb_mem, thread2Attr.cb_size, thread2Attr.stack_mem, thread2Attr.stack_size, thread2Attr.priority);
    Service_start(serviceName3);
#endif
#endif
}
