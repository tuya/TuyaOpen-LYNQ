@echo off
setlocal

@rem -------------------------------------------------------
@rem run cmd:  .\lfs_add_cmd.bat lfs_mp3.bin low_power.mp3
@rem -------------------------------------------------------

set LFS_TOOL=.\lfsutil.exe
set SRC_BIN=%1
set SRC_FILE=%2

%LFS_TOOL% -i .\%SRC_BIN% -r %SRC_FILE%
%LFS_TOOL% -i .\%SRC_BIN% -s
%LFS_TOOL% -i .\%SRC_BIN% -l -d /

%LFS_TOOL% -i .\%SRC_BIN% -w -f %SRC_FILE% -F %SRC_FILE%

%LFS_TOOL% -i .\%SRC_BIN% -s
%LFS_TOOL% -i .\%SRC_BIN% -l -d /

del lfsutil.log
endlocal