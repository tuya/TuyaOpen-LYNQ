#!/bin/bash
export DST_BIN=./testscript/lfsInDefaultForPhone.bin
export SRC_PATH=./testscript
export LFSUTIL=./testscript/lfsutil
$LFSUTIL -i $DST_BIN -w -f default.info      -F $SRC_PATH/default.info
$LFSUTIL -i $DST_BIN -w -f phoneMenu_cn.json -F $SRC_PATH/fpui_res/phoneMenu_cn.json
$LFSUTIL -i $DST_BIN -w -f callList_cn.json  -F $SRC_PATH/fpui_res/callList_cn.json
$LFSUTIL -i $DST_BIN -w -f contact_cn.json   -F $SRC_PATH/fpui_res/contact_cn.json
$LFSUTIL -i $DST_BIN -w -f contactDb.json    -F $SRC_PATH/feature_db/contactDb.json
$LFSUTIL -i $DST_BIN -w -f contactDb.type    -F $SRC_PATH/feature_db/contactDb.type
$LFSUTIL -i $DST_BIN -w -f callListDb.json   -F $SRC_PATH/feature_db/callListDb.json
$LFSUTIL -i $DST_BIN -w -f callListDb.type   -F $SRC_PATH/feature_db/callListDb.type
$LFSUTIL -i $DST_BIN -w -f displayDb.json    -F $SRC_PATH/feature_db/displayDb.json
$LFSUTIL -i $DST_BIN -w -f displayDb.type    -F $SRC_PATH/feature_db/displayDb.type
$LFSUTIL -i $DST_BIN -w -f profileDb.json    -F $SRC_PATH/feature_db/profileDb.json
$LFSUTIL -i $DST_BIN -w -f profileDb.type    -F $SRC_PATH/feature_db/profileDb.type
$LFSUTIL -i $DST_BIN -w -f ttsDb.json        -F $SRC_PATH/feature_db/ttsDb.json
$LFSUTIL -i $DST_BIN -w -f ttsDb.type        -F $SRC_PATH/feature_db/ttsDb.type
$LFSUTIL -i $DST_BIN -w -f presetSmsDb.json  -F $SRC_PATH/feature_db/presetSmsDb.json
$LFSUTIL -i $DST_BIN -w -f presetSmsDb.type  -F $SRC_PATH/feature_db/presetSmsDb.type
$LFSUTIL -i $DST_BIN -w -f smsInboxDb.json   -F $SRC_PATH/feature_db/smsInboxDb.json
$LFSUTIL -i $DST_BIN -w -f smsInboxDb.type   -F $SRC_PATH/feature_db/smsInboxDb.type
$LFSUTIL -i $DST_BIN -w -f smsOutboxDb.json  -F $SRC_PATH/feature_db/smsOutboxDb.json
$LFSUTIL -i $DST_BIN -w -f smsOutboxDb.type  -F $SRC_PATH/feature_db/smsOutboxDb.type
rm -f ./lfsutil.log
