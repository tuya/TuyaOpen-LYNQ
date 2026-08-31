#!/bin/bash

export DST_BIN=./project/ec7xx_ref_1h00/ap/apps/phone/lfs/ext_lfs_2M5.bin
export SRC_PATH=./testscript
export LFSUTIL=./testscript/lfsutil
$LFSUTIL -i $DST_BIN -w -f default.info      -F $SRC_PATH/default1.info
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
$LFSUTIL -i $DST_BIN -w -f MM_01.jpg  -F $SRC_PATH/package/phone/MM_01.jpg
$LFSUTIL -i $DST_BIN -w -f MM_02.jpg  -F $SRC_PATH/package/phone/MM_02.jpg
$LFSUTIL -i $DST_BIN -w -f MM_03.jpg  -F $SRC_PATH/package/phone/MM_03.jpg
$LFSUTIL -i $DST_BIN -w -f MM_04.jpg  -F $SRC_PATH/package/phone/MM_04.jpg
$LFSUTIL -i $DST_BIN -w -f MM_05.jpg  -F $SRC_PATH/package/phone/MM_05.jpg
$LFSUTIL -i $DST_BIN -w -f MM_06.jpg  -F $SRC_PATH/package/phone/MM_06.jpg
rm -f ./lfsutil.log
