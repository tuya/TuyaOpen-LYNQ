# FPUI 使用说明

| Page | cost  | Size | average | weight | memory |
| ---- | ----- | ------- | ----- | ----- | ------ |
| Menu | 10ms  | 240x320 | 24fps | 46fps |  pSRAM |


图片转换使用在线网页工具，字体转换使用桌面工具或**lv_font_conv**

* 图片转换：https://lvgl.io/tools/imageconverter
* 字体转换：https://lvgl.io/tools/fontconverter

## 资源占用

当前LVGL分配的MEM空间使用PSRAM,包括系统字库缓存,对应修改**lv_conf.h**

``` bash
#define LV_MEM_SIZE (300U * 1024U)          /*[bytes]*/
```

显示相关初始化**void lv_port_disp_init(void)** 中定义帧数据

``` bash
    lv_disp_draw_buf_init(&draw_buf_dma, s_disp_buf, p_disp_buf,MY_DISP_HOR_RES*MY_DISP_VER_RES);
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.rounder_cb = lcd_rounder_cb;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dma;
    // disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);
```

* 其中 **disp_drv.hor_res**和**disp_drv.ver_res**分别定义LVGL的画布屏幕尺寸（可以小于实际LCD尺寸）
* **s_disp_buf** 和 **p_disp_buf** 是显示双buffer的指针，全屏刷新指向的空间大小为320x240x2Bytes
* MY_DISP_HOR_RES*MY_DISP_VER_RES是每次更新的显示上限值，例如可以是局部刷新320x24像素（非bytes）

## Font使用

目前系统使用了三个字库，大小分别是36,66,16，其中36包括系统所使用的中文字体，66和16主要是数字和符号

相关字库通过lv_font_conv工具生成，命令如下：
```bash
winget install Schniz.fnm
fnm install --lts
fnm ls
npm install -g lv_font_conv
fnm env --use-on-cd | Out-String | Invoke-Expression
```
系统符号字库（66和16高度）
```bash
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/"
```
生成的指令：
```bash
lv_font_conv --size 66 --bpp 2 --format bin --no-compress --no-prefilter --font E:\eigencomm\V017\PLAT\package\fonts\SourceHanSansCN-Light-2.otf --symbols "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/" -o E:\eigencomm\V017\PLAT\package\merge\lv_font_system_66.bin

lv_font_conv --size 16 --bpp 1 --format bin --no-compress --no-prefilter --font E:\eigencomm\V017\PLAT\package\fonts\SourceHanSansCN-Light-2.otf --symbols "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/" -o E:\eigencomm\V017\PLAT\package\merge\lv_font_system_16.bin

lv_font_conv --size 16 --bpp 1 --format lvgl --no-compress --no-prefilter --font E:\eigencomm\V017\PLAT\package\fonts\SourceHanSansCN-Light-2.otf --symbols "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/" -o E:\eigencomm\V017\PLAT\package\merge\lv_font_system_16.c
```

### lv_font_system_36 字库

作为最小系统字库，只包含系统常用字符，统计如下：

```bash
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890确认短信收菜单录音机手机时间设置存储位置联系人导入导出移动增加簿主页呼叫接听免提删除保存测试返回选项挂断捷径退出通话功能开发中来电紧急通话记录结束等待本定下工具箱件年月日历期上午电子书铃声调节闹钟音量警告震动高低开关计算器格式更新管理写垃圾已常用息恢复语音王恢复出厂安全设置许可标志求助连接管理秒表文情景模式认证亲情号码语言草稿黑白显示多媒体播放相册音乐视频播放器请插入无卡列表中 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/"
```

生成内部字库文件的方式:
```bash
lv_font_conv --size 32 --bpp 2 --format lvgl --no-compress --no-prefilter --font D:\eigencomm\gitea\ec718_716_sdk_main\PLAT\testscript\package\fonts\SourceHanSansCN-Light-2.otf --symbols "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890确认短信收菜单录音机手机时间设置存储位置联系人导入导出移动增加簿主页呼叫接听免提删除保存测试返回选项挂断捷径退出通话功能开发中来电紧急通话记录结束等待本定下工具箱件年月日历期上午电子书铃声调节闹钟音量警告震动高低开关计算器格式更新管理写垃圾已常用息恢复语音王恢复出厂安全设置许可标志求助连接管理秒表文情景模式认证亲情号码语言草稿黑白显示多媒体播放相册音乐视频播放器请插入无卡列表中 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/" -o D:\eigencomm\gitea\ec718_716_sdk_main\PLAT\subsys\apphub\fpui\gui\fonts\lv_font_system_36.c
```

生成外部lv_font_light_36.bin字库文件的方式
```bash
lv_font_conv --size 32 --bpp 2 --format bin --no-compress --no-prefilter --font E:\eigencomm\gitea\ec718_716_sdk_main\PLAT\testscript\package\fonts\SourceHanSansCN-Light-2.otf --symbols "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890确认短信收菜单录音机手机时间设置存储位置联系人导入导出移动增加簿主页呼叫接听免提删除保存测试返回选项挂断捷径退出通话功能开发中来电紧急通话记录结束等待本定下工具箱件年月日历期上午电子书铃声调节闹钟音量警告震动高低开关计算器格式更新管理写垃圾已常用息恢复语音王恢复出厂安全设置许可标志求助连接管理秒表文情景模式认证亲情号码语言草稿黑白显示多媒体播放相册音乐视频播放器请插入无卡列表中 #*?!@#$%^&*()_+-=\|[]{};:,.<>?/" -o E:\eigencomm\gitea\ec718_716_sdk_main\PLAT\testscript\package\merge\lv_font_system_36.bin
```

代码中调用 **ui.c**

```bash
    uint32_t ret = lv_font_extern_init();
    if(ret == 0) {
        UI_ERR("Failed load Font free36\r\n");
    }
```

* 全功能字库**lv_font_extern_36.bin**大小750KB，包含常用汉字，主要用于短信等功能
* 使用外部字库 **lv_font_extern_init()** 加载外部flash数据
* 如果外置字库不使用PSRAM缓存，通过SPI裸地址读取速度相对较慢


## 页面

### List 页面

相关数据结构定义如下：
```bash
typedef struct
{
    char  *title;       //标题字符
    uint8_t  select;    //选中行数 0-5
    char *context[6];   //每行内容
    uint8_t  index;     //起始行索引 < 总行数
    uint8_t  total;     //总行数
    void * image;       //icon图片地址
    char (*bottomBtn)[8];    
} PhoneUI_list_t;
```
* 目前icon图片未实现，bottomBtn是通用组件用于显示底部的按键名称（提示作用）
* context用于显示每行内容，现有屏幕的尺寸只实现了6行（36字体），每次调用传递6行数据的指针
* title是标题，index是起始行的索引，total是整个表的总行数，index和total主要用于指示作用
* select是选中行数，主要是0-5，选中行高亮

### Player 页面

在使用lv_obj_clean后增加lv_obj_del_async的目的是将内存完全回收，不然在重复调用的压力测试中会有内存泄漏

```bash
    if(lv_obj_is_valid(_ui_Player) && _ui_Player != NULL){
        lv_obj_clean(_ui_Player);
        lv_obj_del_async(_ui_Player);
        _ui_Player = NULL;
    }
    if(data == NULL) {
        osEventFlagsSet(uiEvtHandle, (1U << PAGE_APP));
        return NULL;
    }
```
在每个主页中都会调用**ui_btn_set**接口设置底部的按键名称，如果底部按键名称为空，则不显示底部按键

```bash
    ui_btn_set(app_btn,lv_color_black(),lv_color_white());
    lv_disp_load_scr(_ui_Player);
    osEventFlagsSet(uiEvtHandle, (1U << PAGE_APP));
```
使用**lv_disp_load_scr**接口加载页面，同时通过**osEventFlagsSet**传递事件给UI线程，UI线程通过**osEventFlagsWait**等待事件会返回


## 组件

### slider使用说明

目前作为list的组成部分，在原有list参数基础上，增加index参数作为显示页的实际起始索引，当前列表的偏移量select，显示高亮行实际行数为index+select，总行数total影响slider显示长度和比例

```bash
typedef struct
{
    char  *title;       //标题字符
    uint8_t  select;    //选中行数 0-5
    char *context[6];   //每行内容
    uint8_t  index;     //起始行索引 (限制 (index+select) < total)
    uint8_t  total;     //总行数
    void * image;       //icon图片地址
    char (*bottomBtn)[8];    
} PhoneUI_list_t;

```

目前针对参数加入了防呆机制，如果输入index+select 超出total，则不会添加slider图案
```bash
if((data->index+data->select) >= data->total){
    SYSLOG_ERR("(%d+%d)/%d",data->index,data->select,data->total);
    return ui_List;
}
```


### bar使用说明

bar修改为横向和纵向两种，修改自定义配置，包括坐标/长宽/颜色，配置数据结构体：
```bash
typedef struct
{  
    uint8_t  value;     //标识值，同时数字标号
    uint8_t  range;     //最大值
    int16_t  offsetX;   //x偏移量 如果为0则居中
    int16_t  offsetY;   //y偏移量 如果为0则居中
    int16_t  width;     //bar宽度
    int16_t  height;    //bar高度
    uint16_t  color;    //bar颜色
} fpui_bar_t;
```

其中width和height为bar的宽高，设定不同的宽高可以实现横向和纵向bar，offsetX和offsetY为bar的偏移量，如果为0则居中，如果为正数则向右或向下偏移，如果为负数则向左或向上偏移。

* 可以将bar视作一个矩形，通过width和height实现形状定义，通过offsetX和offsetY实现位置定义

### message使用说明

* 绘制时间 5.1(≤23)ms

消息弹窗数据结构如下：
```bash
typedef struct
{
    void *object;
    void *screen;
    char *title;        //暂未使用
    char *context;
    bool overlay;       //是否叠加显示,true=不删除原弹窗
    int16_t  offsetX;   //x偏移量 如果为0则居中 -100 ~ 100
    int16_t  offsetY;   //y偏移量 如果为0则居中 -100 ~ 100
    uint16_t width;     //宽度 40 ~ 240
    uint16_t height;    //高度 40 ~ 320
    uint16_t  color;
} fpui_message_t;
```
1. **title**: 弹窗标题，暂时未使用
2. **context**: 弹窗显示的消息内容
3. **offsetX**: 弹窗x偏移量，如果为0则居中 -100 ~ 100
4. **offsetY**: 弹窗y偏移量，如果为0则居中 -100 ~ 100
5. **width**: 弹窗宽度 40 ~ 240
6. **height**: 弹窗高度 40 ~ 320
7. **color**: 弹窗背景色，默认为白色
8. **overlay**: 是否可重叠，如果为true则不删除原弹窗


### listpop使用说明

弹窗列表功能，支持定义长宽和位置，支持颜色调整

* 绘制时间 14.2(≤23)ms

```bash
typedef struct
{
    void    *object;
    void    *screen;
    uint8_t  index;     //起始行索引 < 总行数
    uint8_t  total;     //总行数
    char *context[MAX_LISTPOP_ITEMS];   //每行内容
    uint8_t  select;    //选中行数 0-5
    bool overlay;       //是否叠加显示,true=不删除原弹窗
    int16_t  offsetX;   //x偏移量 如果为0则居中 -100 ~ 100
    int16_t  offsetY;   //y偏移量 如果为0则居中 -100 ~ 100
    uint16_t width;     //宽度,建议220，和内容相关
    uint16_t height;    //高度，支持自适应
    uint16_t color;
} fpui_listpop_t;
```


### ui_file 使用说明

用于显示文件夹的内容

* 绘制时间 44(≤46)ms

```bash
typedef struct
{
    void   *object;
    char   *folder;
    char    *title;
    uint16_t color;
    uint8_t select;
    uint8_t  index;
    uint8_t  total;
    char *context[6];
    char (*bottomBtn)[8];
} fpui_file_t;
```