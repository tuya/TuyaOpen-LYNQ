@echo off
setlocal

set LFS_TOOL=..\testscript\lfsutil.exe
set PARAMETERS=%1
cd /d "%~dp0"

set DEST_DIR=..\project\%BOARD_NAME%\ap\apps\%PROJECT_NAME%\lfs\

:prepack_action
if exist %DEST_DIR%\ex_lfs.bin (
    del %DEST_DIR%\ex_lfs.bin
)
copy .\ext_lfs_2M5.bin .\ex_lfs.bin

set WATCH_RES=.\watch_res\
for /r %WATCH_RES% %%i in (*) do (
    %LFS_TOOL% -i .\ex_lfs.bin -w -f %%~nxi -F %%i
)
for /r %DEST_DIR% %%i in (*) do (
    %LFS_TOOL% -i .\ex_lfs.bin -w -f %%~nxi -F %%i
)

%LFS_TOOL% -i .\ex_lfs.bin -s
%LFS_TOOL% -i .\ex_lfs.bin -l -d /

move ex_lfs.bin %DEST_DIR%
del lfsutil.log
endlocal