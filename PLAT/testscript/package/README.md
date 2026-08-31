# 文件打包

外置文件系统总大小2MB


如果指定路径下是已经排序的bin文件，则按照命名排序合并为一个统一的bin，可以指定输入路径、输出路径和文件名称
``` bash
python merge.py .\merge ..\..\project\ec7xx_ref_1h00\ap\apps\phone\lfs merged
```

* 目前裸地址访问只有1M空间，只能打包两张图片，需要先修改文件布局才能放入所有图片