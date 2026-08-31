# openHAL Device

openHAL针对对Device单独管理，便于扩展和分包维护

| openHAL | startup | create | query | open | apply | ioctl | pmctl | close | delete |  
| ------ | ----- | ----- | ------ | ---- | ----- | ----- | ----- | ------ | ------ | 
| [BTN](#btn)  |  √  | √  |  √  |  √  |  √  |  √  |  √  | √  |  √  | 


## btn

作为示例项目，用于验证相关基础接口完备性

* startup()接口用于上电初始化，主要包括用于初始化boot/wakeup等按键，文件配置


## lcd

整理上下层LCD调用关系

## tp

整理tp相关接口，适配不同器件