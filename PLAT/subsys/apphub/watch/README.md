# watch-UI

* 图标制作：https://www.pgyer.com/tools/appIcon
* 图片转换：https://lvgl.io/tools/imageconverter
* 尺寸转换：https://www.gaitubao.com/
* 中文字体：https://lvgl.io/tools/fontconverter
* [地图下载链接生成工具](https://github.com/W-Mai/XLocateDownloader/releases/download/1.0.0/MapDownloader.zip)
* [地图下载器](https://github.com/W-Mai/XLocateDownloader)
* [LVGL图片转换器(Python版)](https://github.com/W-Mai/lvgl_image_converter)
* [批量文件重命名工具](https://github.com/W-Mai/filename_renamer)

图片转换请参考脚本：faces\smart_resized\assets\b.py 和 faces\smart_resized\assets\conv.py

.\tools\lfsutil.exe -i project\ec7xx_ref_1h00\ap\apps\watch\lfs\ex_lfs.bin -l -d /

### 模拟TP输入

* 通过访问特定测试函数,可以传入TP值触发: tp_irq_set手动注入中断,tp_scan_test手动注入坐标;

## 中英文

目前多语言实现，典型中英文对译

### 主页列表

const char *days[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
对应中文 {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期天"};

"Back"
"返回"

"Close"
"关闭"

"Error"
"错误"

"updated at\n--:--"
 "更新于\n--:--"

"Weather information has not yet been synced. Make sure to enable it in the app settings."
"天气信息未同步. 请确保已经打开了相关配置."


### 二级菜单

"message" - "短信"
"Weather" - "天气"
"AppInfo" - "设备信息"
"Apps" - "应用"
"Watchfaces" - "表盘"
"alipay" - "支付宝"
"DialPad" - "拨号"
"Settings" - "设置"

### 三级菜单

"Change Current" = "预制表盘"
"App Info" = "设备信息"

## UI APP task

本文主要说明带UI的APP开发，如何维持多个APP的数据处理和数据显示安全更新，不同的APP可能会要求使用task，后台task的类型主要分如下情况：
1. 需要保持在后台持续运行，拥有可定义优先级；（独立运行的task）
2. 只在该APP的UI调度到前台激活的情况下才会运行；（和多个APP共享coTask）
3. 需要周期执行且运行一次的时间不超过20ms；（共用guitask，可自由添加lvgl操作）
4. 实时性周期性调度，但运行时间短，采用osTimer/lv_timer，回调函数操作；

针对第一种独立运行的task，和UI进行数据同步的情况，使用事件回调机制：
- 注册应用的自定义事件 - lv_event_register_id()
- 使用事件回调实现同步 - lv_event_send(obj, LV_EVENT_MINE, &some_data)

```bash
void event_cb(lv_event_t * e) {
    if (e->code == EVENT_UPDATE_LABEL_DISTANCE) {
        int *currentScore = (int *)lv_event_get_param(e);// 通过该接口获取param数据
        int *data = lv_obj_get_user_data(e->target);     //使用该接口接收手动传入数据
        lv_label_set_text_fmt(ui_distanceLabel, "%d", *currentScore);  // 更新标签文本
    }
}
```
注意事件数据和用户数据的传入参数不同，一个是事件e，一个是对象obj