#!/bin/bash

export OUT_DIR=${BUILD_PRO_TARGET}_download
export bootloader_dir=gccout/$OUTPUT_NAME/ap/bootloader
export project_dir=gccout/$OUTPUT_NAME/ap/$PROJECT_NAME

echo --------- Package dowmload ---------
echo BUILD_PRO_NAME=$BUILD_PRO_NAME
echo BUILD_PRO_TARGET=$BUILD_PRO_TARGET
echo PROJECT_NAME=$PROJECT_NAME
echo project_dir=$project_dir
echo bootloader_dir=$bootloader_dir

if [ -e $OUT_DIR ]; then
    rm -rf $OUT_DIR
fi
mkdir $OUT_DIR

cp  $bootloader_dir/ap_bootloader.bin                   $OUT_DIR/
cp  $bootloader_dir/ap_bootloader.elf                   $OUT_DIR/
cp  $bootloader_dir/ap_bootloader.map                   $OUT_DIR/

cp  $project_dir/comdb.txt                              $OUT_DIR/
cp  $project_dir/ap_$PROJECT_NAME.bin                   $OUT_DIR/
cp  $project_dir/ap_$PROJECT_NAME.elf                   $OUT_DIR/
cp  $project_dir/ap_$PROJECT_NAME.map                   $OUT_DIR/
cp  $project_dir/$PROJECT_NAME.binpkg                   $OUT_DIR/$BUILD_PRO_NAME.binpkg

cp  ./mbtk/releasepack/$BUILD_PRO_NAME/config_*.ini      $OUT_DIR/
cp  ./mbtk/releasepack/$BUILD_PRO_NAME/*.bin             $OUT_DIR/

echo ---------- Package end ------------