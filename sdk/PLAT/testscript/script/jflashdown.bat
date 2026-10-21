::JFlash.exe -openprj"D:\Jenkins\script\ec.jflash" -open"D:\Jenkins\workspace\EC718_OPEN\PLAT\gccout\ec718_ref_1h00\ap\bootloader\ap_bootloader.bin,0x00803000" -auto -exit
::JFlash.exe -openprj"D:\Jenkins\script\ec.jflash" -open"D:\EC718_Firmware\RCV\ap_bootloader.bin,0x00803000" -auto -exit
::JFlash.exe  -startapp -hide -exit
JFlash.exe -openprj"D:\Jenkins\script\ec.jflash" -open"D:\Jenkins\workspace\EC718_OPEN\PLAT\gccout\ec718_ref_1h00\ap\ref_app\ap_ref_app.bin,0x008A3000" -auto -exit
JFlash.exe  -startapp -hide -exit
::JFlash.exe -openprj"D:\Jenkins\script\ec.jflash" -open"D:\Jenkins\script\opentest\fs\lfs.fs,0x00E03000" -auto -exit
::JFlash.exe  -startapp -hide -exit
::JFlash.exe -openprj"D:\Jenkins\script\ec.jflash" -open"D:\Jenkins\script\opentest\rfCaliTb\EC718P_M2324_CN_V2A.bin,0x00F4B000" -auto -exit
::JFlash.exe  -startapp -hide -exit
