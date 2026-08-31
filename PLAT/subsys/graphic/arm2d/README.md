## ARM-2D


### usage



在配置使用ARM-2D时，需要修改flash.ld文件
```
  .text 0x882000:
  {
```
解决资源跨区域问题：
```
  .text :
  {
```

同时需要修改 ***core_cm3.h***  文件，注释掉头文件

```
#include "ec7xx.h"
```


针对不同的LCD修改长宽尺寸：

***arm_2d_disp_adapter_0.h*** 
```
#   define __DISP0_CFG_SCEEN_WIDTH__                               240
#   define __DISP0_CFG_SCEEN_HEIGHT__                              320
```
***arm_2d_cfg.h*** 
```
#   define __GLCD_CFG_SCEEN_WIDTH__                               240
#   define __GLCD_CFG_SCEEN_HEIGHT__                              320
```
针对不同的内存占用，设定 ***PFB block size*** 

```
#   define __DISP0_CFG_PFB_BLOCK_WIDTH__                           240
#   define __DISP0_CFG_PFB_BLOCK_HEIGHT__                          32
```

配置不同的demo,需要定义相关的编译宏

```
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Controls
CFLAGS += -DRTE_Acceleration_Arm_2D_Helper_Disp_Adapter0
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Benchmark
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Benchmark_Watch_Panel
```

使用Benchmark_Generic需要定义如下宏

```
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Benchmark
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Benchmark_Generic
```

使用DMA+ISR方式使能相关宏定义
```
#   define __DISP0_CFG_ENABLE_ASYNC_FLUSHING__                     1
```
针对横屏和竖屏的修改，不止要修改.h头文件，还需要对应修改LCD Fill相关参数