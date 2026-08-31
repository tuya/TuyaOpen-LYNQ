#!/bin/bash

export DST_BIN=./testscript/lfsInDefaultForPhone96k.bin
export SRC_PATH=./testscript/fpui_res
export LFSUTIL=./testscript/lfsutil
$LFSUTIL -i $DST_BIN -w -f phoneMenu_cn.json -F $SRC_PATH/phoneMenu_cn.json
$LFSUTIL -i $DST_BIN -w -f callList_cn.json  -F $SRC_PATH/callList_cn.json
$LFSUTIL -i $DST_BIN -w -f contact_cn.json   -F $SRC_PATH/contact_cn.json
rm -f ./lfsutil.log
