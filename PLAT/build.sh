#!/bin/bash
echo $PATH
# clear

export BUILD_ENV=linux
export PROJECT_NAME=tuyaopen
export BOARD_NAME=ec7xx_0h00
export CHIP_NAME=ec7xx
export CHIP_TYPE=ec716e
export CORE_NAME=ap
export EUTRAN_MODE=cat_mode
export BUILD_OPTION=merge
export UNILOG=false
export TOOLCHAIN_NAME=GCC
export CUST=common
export SDKREL=false
export OPENCPU=true
export LTO_ENABLE=false
export GCF_ENABLE=false
export RAM_ENBALE=true
export ROM_ENBALE=false
export LESS_LOG=false
export PWR_TEST=false
export BUILD_HEADBIN=false
export GCCLIB_PATH=`realpath "../../tools/gcc-arm-none-eabi-10-2020-q4-major"`
export COMDBLIB_PATH="./prebuild/PLAT/lib/gcc/$CHIP_TYPE/ram"
export CPBIN_SUBPATH=ram
export PKG_MAPDEF=pkg_716e_mapdef
export PKG_PRODUCT="EC716E_PRD"
export OUTPUT_NAME=$BOARD_NAME"_"$CHIP_TYPE
export JOBNUMBER=j8

export CUR_PATH=$('pwd')

echo $0 version $(date "+%Y%m%d")
echo GCCLIB_PATH: $GCCLIB_PATH
echo Chip Type: $CHIP_TYPE

echo OUTPUT_NAME: $OUTPUT_NAME

if [ ! -e $GCCLIB_PATH ]
then
	echo ERROR:Please check GCCLIB_PATH setting,exit!!!
	exit 1
fi

if [ ! -e "gccout" ]; then
mkdir gccout
fi
PARAMETERS=$1
echo "PARAMETERS：$PARAMETERS"
# -------------------------------------------------------
# mbtk opencpu platform config
# -------------------------------------------------------
export BUILD_PRO_BOARD=EC716E
export BUILD_PRO_CHIP=L511C_Y6
export BUILD_PRO_NAME=L511C_Y6E
export BUILD_PRO_TARGET=L511C_Y6E

# mbtk add for copy comdb_cust_lib to tools
cp -rf ./openlib/$BUILD_PRO_TARGET/comdb_cust_lib.txt  ./tools

#set build_config.inc file
echo MBTK_PRODUCT_BOARD=$BUILD_PRO_BOARD > ./middleware/developed/build_config.inc
echo MBTK_PRODUCT_CHIP=$BUILD_PRO_CHIP >> ./middleware/developed/build_config.inc
echo MBTK_PRODUCT_NAME=$BUILD_PRO_NAME >> ./middleware/developed/build_config.inc

# set build factory feature
fac=$1
if [ "$fac"x == "-f"x ]; then
    echo CFLAGS += -DFACTORY_TEST_MODE>>./middleware/developed/build_config.inc
fi

echo curr Board   is: $BOARD_NAME
echo curr Chip    is: $CHIP_NAME
echo curr Type    is: $CHIP_TYPE
echo curr Project is: $PROJECT_NAME
echo curr core    is: $CORE_NAME
echo curr option  is: $BUILD_OPTION
echo cp bin subpath:  $CPBIN_SUBPATH

failHandle()
{
	echo fail
	echo "#######################################################################"
	echo "##                                                                   ##"
	echo "##                    ########    ###     ####  ##                   ##"
	echo "##                    ##         ## ##     ##   ##                   ##"
	echo "##                    ##        ##   ##    ##   ##                   ##"
	echo "##                    ######   ##     ##   ##   ##                   ##"
	echo "##                    ##       #########   ##   ##                   ##"
	echo "##                    ##       ##     ##   ##   ##                   ##"
	echo "##                    ##       ##     ##  ####  ########             ##"
	echo "##                                                                   ##"
	echo "#######################################################################"
	exit 1
}

clean()
{
	make -$JOBNUMBER  clean-gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME
	exit 0
}

build_lib()
{
	echo start loggpress
	make -j1 build-unilog TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true
	# copy log file
	cp ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/debug_log_ap.h ./middleware/developed/debug/inc/

	if [ $? -eq 0 ]; then
		echo "debug_log_ap.h exit"
	else
		echo "debug_log_ap.h error"
		exit 1
	fi

	if [ -e "./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h" ]; then
		cp -rf ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h  ./middleware/developed/debug/inc/
	fi

	echo "start to build tuyaos adapter..."
	cd project/$BOARD_NAME/ap/apps/$PROJECT_NAME/GCC
	make -$JOBNUMBER tuyaos_adapter TYPE=$CHIP_TYPE TARGET=$BOARD_NAME EUTRAN_MODE=$EUTRAN_MODE PROJECT=$PROJECT_NAME CORE=ap BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true MANUFACTURER=$MANUFACTURER

	if [ ${PIPESTATUS[0]} -gt 0 ]; then
		echo "build fail"
		exit 1
	fi
	echo "build successfully"
	exit 0
}

build_elf()
{
	echo start loggpress
	make -j1 build-unilog TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true
	# copy log file
	cp ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/debug_log_ap.h ./middleware/developed/debug/inc/
	if [ -e "./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h" ]; then
		cp -rf ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h  ./middleware/developed/debug/inc/
	fi
	
	make -$JOBNUMBER gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME EUTRAN_MODE=$EUTRAN_MODE PROJECT=$PROJECT_NAME CORE=ap BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true MANUFACTURER=$MANUFACTURER | tee ./gccout/$OUTPUT_NAME/ap/outbuildlog.txt
	if [ ${PIPESTATUS[0]} -gt 0 ]
	then
		failHandle
	fi

	# ./tools/fcelf -T -bin ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -size AP_PKGIMG_LIMIT_SIZE \
	# 								-bin ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -size CP_PKGIMG_LIMIT_SIZE \
	# 								-bin ./openlib/$BUILD_PRO_TARGET/bin/ap_bootloader.bin -size BOOTLOADER_PKGIMG_LIMIT_SIZE \
	# 								-h ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt
	# if [ $? -ne 0 ]
	# then
	# 	failHandle
	# fi

	./tools/fcelf -M  -input ./openlib/$BUILD_PRO_TARGET/bin/ap_bootloader.bin -addrname BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE  \
	-input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -addrname AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
	-input ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
	-input ./mbtk/releasepack/$BUILD_PRO_NAME/$CHIP_TYPE"_"PrMgrCfg.json.bin -addrname XPKGDPRM_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
		-input ./mbtk/releasepack/$BUILD_PRO_NAME/format_$CHIP_TYPE.json.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
		-input ./mbtk/releasepack/$BUILD_PRO_NAME/config_$CHIP_TYPE"_"prd_uart.baseini.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
		-input ./mbtk/releasepack/$BUILD_PRO_NAME/config_$CHIP_TYPE"_"prd_usb.baseini.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
	-pkgmode 1  \
	-banoldtool 1 \
	-productname $PKG_PRODUCT \
	-def ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt \
	-outfile ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/$PROJECT_NAME.binpkg

	if [ $? -ne 0 ]
	then
		failHandle
	fi

	if [  -f ./tools/UpdateDBPattern.txt ]
	then
		rm -f ./tools/UpdateDBPattern.txt
	fi

	echo "build successfully"
	./output_download.sh
	exit 0
}

if [ $# -ge 1 ]; then
    if [ "$1" = "clean" ]; then
		clean
	else
		echo "unknown command $@"
		exit 1
	fi
else
    # build_lib
	build_elf
fi


end()
{
	echo "--end--"
	exit 1
}

if [ -n "$(echo $PARAMETERS|grep 'clall')" ]
then
	make -$JOBNUMBER  clean-gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME
    rm -rf ./gccout/*
	echo "clean all done ok..."
	end
fi

if [ -n "$(echo $PARAMETERS|grep 'clean')" ]
then
	make -$JOBNUMBER  clean-gcc TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME
	echo "clean done ok..."
	end
fi

completeHandle()
{
endtime=$(date "+%Y/%m/%d %H:%M:%S")
echo "Start time:" $starttime
echo "End time:" $endtime

echo "#######################################################################"
echo "##                                                                   ##"
echo "##                 ########     ###     ######   ######              ##"
echo "##                 ##     ##   ## ##   ##    ## ##    ##             ##"
echo "##                 ##     ##  ##   ##  ##       ##                   ##"
echo "##                 ########  ##     ##  ######   ######              ##"
echo "##                 ##        #########       ##       ##             ##"
echo "##                 ##        ##     ## ##    ## ##    ##             ##"
echo "##                 ##        ##     ##  ######   ######              ##"
echo "##                                                                   ##"
echo "#######################################################################"
	echo build successfully

#--------------------------------------------------------------------
# mbtk add to copy download package
./output_download.sh


exit 1

}



starttime=$(date "+%Y/%m/%d %H:%M:%S")
echo "Start time:" $starttime

if [ "$BUILD_OPTION" == "merge" ]
then
	if [ -f './tools/comdblib.txt' ]
	then
		# rm -rf ./gccout/*
		echo start loggpress
		make -$JOBNUMBER build-unilog TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true
		# copy log file
		cp ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/debug_log_ap.h ./middleware/developed/debug/inc/
		if [ -e "./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h" ]; then
			cp -rf ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h  ./middleware/developed/debug/inc/
		fi

		# echo "start to build tuyaos adapter..."
		# cd project/$BOARD_NAME/ap/apps/$PROJECT_NAME/GCC
		# # pwd
		# make -$JOBNUMBER tuyaos_adapter TYPE=$CHIP_TYPE TARGET=$BOARD_NAME EUTRAN_MODE=$EUTRAN_MODE PROJECT=$PROJECT_NAME CORE=ap BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true MANUFACTURER=$MANUFACTURER
		# exit

		# build ap
		make -$JOBNUMBER gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME EUTRAN_MODE=$EUTRAN_MODE PROJECT=$PROJECT_NAME CORE=ap BUILD_UNILOG=true BUILD_CUST=$CUST SDK=true MANUFACTURER=$MANUFACTURER | tee ./gccout/$OUTPUT_NAME/ap/outbuildlog.txt

		if [ ${PIPESTATUS[0]} -gt 0 ]
		then
			failHandle
		fi

		./tools/fcelf -T -bin ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -size AP_PKGIMG_LIMIT_SIZE \
									-bin ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -size CP_PKGIMG_LIMIT_SIZE \
									-bin ./openlib/$BUILD_PRO_TARGET/bin/ap_bootloader.bin -size BOOTLOADER_PKGIMG_LIMIT_SIZE \
									-h ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt
		if [ $? -ne 0 ]
		then
			failHandle
		fi
		if [ "$BUILD_HEADBIN" == "true" ]
		then
		echo general headers 3
		./tools/ecsecure APIMAGE=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin \
								 CPIMAGE=./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin \
								 BLIMAGE=./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.bin \
								 BLHASH=1 SYSHASH=1 \
								 HEAD1=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/bl_sec_header.bin \
								 HEAD2=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/sys_sec_header.bin \
								 BLPEM=./project/$BOARD_NAME/ap/apps/bootloader/code/main/pub_key_bl.pem \
								 SYSPEM=./project/$BOARD_NAME/ap/apps/bootloader/code/main/pub_key_sys.pem \
								  ADRBASE=AP_FLASH_XIP_ADDR \
								  APADR=AP_FLASH_LOAD_ADDR \
								  CFGDEF=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt
		if [ $? -ne 0 ]
		then
		   failHandle
		fi

	        ./tools/fcelf -M  -input ./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.bin -addrname  BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE \
								  -input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -addrname  AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
								  -input ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
								  -input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/bl_sec_header.bin -addrname  BLS_SEC_HAED_ADDR -flashsize BLS_FLASH_LOAD_SIZE \
								  -input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/sys_sec_header.bin -addrname  SYS_SEC_HAED_ADDR -flashsize SYS_FLASH_LOAD_SIZE \
									-pkgmode 1  \
									-banoldtool 1 \
									-productname $PKG_PRODUCT \
									-def ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt \
								  -outfile ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/$PROJECT_NAME.binpkg

			if [ $? -ne 0 ]
			then
			   failHandle
			fi
		else
			./tools/fcelf -M  -input ./openlib/$BUILD_PRO_TARGET/bin/ap_bootloader.bin -addrname BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE  \
			-input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -addrname AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
			-input ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
			-input ./mbtk/releasepack/$BUILD_PRO_NAME/$CHIP_TYPE"_"PrMgrCfg.json.bin -addrname XPKGDPRM_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
		        -input ./mbtk/releasepack/$BUILD_PRO_NAME/format_$CHIP_TYPE.json.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
		        -input ./mbtk/releasepack/$BUILD_PRO_NAME/config_$CHIP_TYPE"_"prd_uart.baseini.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
		        -input ./mbtk/releasepack/$BUILD_PRO_NAME/config_$CHIP_TYPE"_"prd_usb.baseini.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
			-pkgmode 1  \
			-banoldtool 1 \
			-productname $PKG_PRODUCT \
			-def ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt \
			-outfile ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/$PROJECT_NAME.binpkg

			if [ $? -ne 0 ]
			then
				failHandle
			fi

		fi

		if [ "$FIBOCOM_PKG_OPENSDK" == "true" ];
		then
			if [ -e "opensdk.py" ]; then
				python3 opensdk.py $PROJECT_NAME $CHIP_TYPE
				./opensrc.sh
			fi
		fi

		if [  -f ./tools/UpdateDBPattern.txt ]
		then
			rm -f ./tools/UpdateDBPattern.txt
		fi
		completeHandle
	else
		make -$JOBNUMBER gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE EUTRAN_MODE=$EUTRAN_MODE PROJECT=bootloader CORE=$CORE_NAME SDK_REL=$SDKREL BUILD_UNILOG=false BUILD_CUST=$CUST | tee ./gccout/$OUTPUT_NAME/$CORE_NAME/outbuildlog.txt
		if [ ${PIPESTATUS[0]} -gt 0 ]
		then
			failHandle
		fi
		echo start logprepass b22
		if [ -e './tools/UpdateDBPattern.txt' ]
		then
			rm -f ./tools/UpdateDBPattern.txt
		fi

		make -$JOBNUMBER build-unilog TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE PROJECT=cp_project CORE=cp SDK_REL=$SDKREL BUILD_UNILOG=true BUILD_CUST=$CUST | tee ./gccout/$OUTPUT_NAME/cp/outbuildlog.txt
		if [ ${PIPESTATUS[0]} -gt 0 ]
		then
			failHandle
		fi
		make -$JOBNUMBER gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE EUTRAN_MODE=$EUTRAN_MODE PROJECT=cp_project CORE=cp SDK_REL=$SDKREL BUILD_UNILOG=true BUILD_CUST=$CUST | tee ./gccout/$OUTPUT_NAME/cp/outbuildlog.txt
		if [ ${PIPESTATUS[0]} -gt 0 ]
		then
			failHandle
		fi
		make -$JOBNUMBER build-unilog TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE PROJECT=$PROJECT_NAME CORE=ap SDK_REL=$SDKREL BUILD_UNILOG=true BUILD_CUST=$CUST | tee ./gccout/$OUTPUT_NAME/ap/outbuildlog.txt
		if [ ${PIPESTATUS[0]} -gt 0 ]
		then
			failHandle
		fi
		# copy log file
		cp ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/debug_log_ap.h ./middleware/developed/debug/inc/
		if [ -e "./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h" ]; then
			cp -rf ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/dbversion.h  ./middleware/developed/debug/inc/
		fi

		# build ap
		make -$JOBNUMBER gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE EUTRAN_MODE=$EUTRAN_MODE PROJECT=$PROJECT_NAME CORE=ap SED_REL=$SDKREL BUILD_UNILOG=true BUILD_CUST=$CUST | tee ./gccout/$OUTPUT_NAME/ap/outbuildlog.txt
		if [ ${PIPESTATUS[0]} -gt 0 ]
		then
			failHandle
		fi
		./tools/fcelf -T -bin ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -size AP_PKGIMG_LIMIT_SIZE \
									-bin ./gccout/$OUTPUT_NAME/cp/cp_project/cp-demo-flash.bin -size CP_PKGIMG_LIMIT_SIZE \
									-bin ./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.bin -size BOOTLOADER_PKGIMG_LIMIT_SIZE \
									-h ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt
		if [ $? -ne 0 ]
		then
			failHandle
		fi

		if [ "$BUILD_HEADBIN" == "true" ]
		then
		echo general headers 4
		./tools/ecsecure APIMAGE=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin \
								 CPIMAGE=./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin \
								 BLIMAGE=./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.bin \
								 BLHASH=1 SYSHASH=1 \
								 HEAD1=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/bl_sec_header.bin \
								 HEAD2=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/sys_sec_header.bin \
								 BLPEM=./project/$BOARD_NAME/ap/apps/bootloader/code/main/pub_key_bl.pem \
								 SYSPEM=./project/$BOARD_NAME/ap/apps/bootloader/code/main/pub_key_sys.pem \
								  ADRBASE=AP_FLASH_XIP_ADDR \
								  APADR=AP_FLASH_LOAD_ADDR \
								  CFGDEF=./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt
		if [ $? -ne 0 ]
		then
		   failHandle
		fi
			./tools/fcelf -M  -input ./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.bin -addrname  BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE \
								  -input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -addrname  AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
								  -input ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
								  -input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/bl_sec_header.bin -addrname  BLS_SEC_HAED_ADDR -flashsize BLS_FLASH_LOAD_SIZE \
								  -input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/sys_sec_header.bin -addrname  SYS_SEC_HAED_ADDR -flashsize SYS_FLASH_LOAD_SIZE \
									-pkgmode 1  \
									-banoldtool 1 \
									-productname $PKG_PRODUCT \
									-def ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt \
								  -outfile ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/$PROJECT_NAME.binpkg
			if [ $? -ne 0 ]
			then
			   failHandle
			fi
		else
			./tools/fcelf -M  -input ./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.bin -addrname BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE  \
			-input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.bin -addrname AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
			-input ./prebuild/FW/lib/gcc/$CHIP_TYPE/$CPBIN_SUBPATH/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
			-pkgmode 1  \
			-banoldtool 1 \
			-productname $PKG_PRODUCT \
			-def ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt \
			-outfile ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/$PROJECT_NAME.binpkg

			if [ $? -ne 0 ]
			then
				failHandle
			fi
		fi

		./tools/fcelf -m  -input ./gccout/$OUTPUT_NAME/ap/bootloader/ap_bootloader.elf -addrname BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE  \
				-input ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/ap_$PROJECT_NAME.elf -addrname  AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
				-input ./gccout/$OUTPUT_NAME/cp/cp_project/cp-demo-flash.elf -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
				-pkgmode 1  \
				-banoldtool 1 \
				-productname $PKG_PRODUCT \
				-def ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/mem_map.txt \
				-outfile ./gccout/$OUTPUT_NAME/ap/$PROJECT_NAME/$PROJECT_NAME.elf

		if [ $? -ne 0 ]
		then
			failHandle
		fi

		if [  -f ./tools/UpdateDBPattern.txt ]
		then
		rm -f ./tools/UpdateDBPattern.txt
		fi
		completeHandle
	fi
fi
buildimage()
{
echo "buildimage"

( make -$JOBNUMBER gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE EUTRAN_MODE=$EUTRAN_MODE PROJECT=$PROJECT_NAME CORE=$CORE_NAME BUILD_UNILOG=$UNILOG BUILD_CUST=$CUST ) | tee ./gccout/$OUTPUT_NAME/$CORE_NAME/outbuildlog.txt

if [ ${PIPESTATUS[0]} -gt 0 ]
then
   failHandle
else
	completeHandle
fi

}

if [ -n  "$(echo $PROJECT_NAME|grep 'bootloader')" ]
then
	buildimage
fi

if [ -n  "$(echo $PROJECT_NAME|grep 'driver_example')" ]
then
	buildimage
fi

export UNILOG=true
echo "start logprepass b2"
if [  -f ./tools/UpdateDBPattern.txt ]
then
	rm -f ./tools/UpdateDBPattern.txt
fi

 make -$JOBNUMBER build-unilog TYPE=$CHIP_TYPE TARGET=$BOARD_NAME V=$VERBOSE PROJECT=$PROJECT_NAME CORE=$CORE_NAME TOOLCHAIN_NAME=$TOOLCHAIN_NAME BUILD_UNILOG=true BUILD_CUST=$CUST | tee ./gccout/$OUTPUT_NAME/$CORE_NAME/outbuildlog.txt

if [ ${PIPESTATUS[0]} -gt 0 ]
then
   failHandle
fi

#this header file will compile with device code
cp -f ./gccout/$OUTPUT_NAME/$CORE_NAME/$PROJECT_NAME/debug_log_$CORE_NAME.h ./middleware/developed/debug/inc/
if [ $? -ne 0 ]
then
   failHandle
fi

if [ -f ./gccout/$OUTPUT_NAME/$CORE_NAME/$PROJECT_NAME/dbversion.h ]
then
  	cp -f ./gccout/$OUTPUT_NAME/$CORE_NAME/$PROJECT_NAME/dbversion.h ./middleware/developed/debug/inc/
	if [ $? -ne 0 ]
	then
		failHandle
	fi
fi

buildimage

