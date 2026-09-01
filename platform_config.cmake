##
# @file platform_config.cmake
#/

####################################################
# By configuring the variable PLATFORM_PUBINC,
# the header file in the platform is provided to TuyaOpen for use.
####################################################

list_subdirectories(PLATFORM_PUBINC ${PLATFORM_PATH}/tuyaos/tuyaos_adapter)


####################################################
# Same story as littlefs above for cJSON: TuyaOpen builds its own, and the
# vendor SDK links a prebuilt libcjson.a that other vendor libraries call --
# including extensions (cJSON_GetObjectStringItemByValue) TuyaOpen's copy does
# not have, so the vendor archive has to stay. Renaming TuyaOpen's symbols lets
# both live in one image, each used by the code it was built with.
####################################################
set(TUYAOPEN_CJSON_SYMBOLS
    cJSON_AddArrayToObject
    cJSON_AddBoolToObject
    cJSON_AddFalseToObject
    cJSON_AddItemReferenceToArray
    cJSON_AddItemReferenceToObject
    cJSON_AddItemToArray
    cJSON_AddItemToObject
    cJSON_AddItemToObjectCS
    cJSON_AddNullToObject
    cJSON_AddNumberToObject
    cJSON_AddObjectToObject
    cJSON_AddRawToObject
    cJSON_AddStringToObject
    cJSON_AddTrueToObject
    cJSON_Compare
    cJSON_CreateArray
    cJSON_CreateArrayReference
    cJSON_CreateBool
    cJSON_CreateDoubleArray
    cJSON_CreateFalse
    cJSON_CreateFloatArray
    cJSON_CreateIntArray
    cJSON_CreateNull
    cJSON_CreateNumber
    cJSON_CreateObject
    cJSON_CreateObjectReference
    cJSON_CreateRaw
    cJSON_CreateString
    cJSON_CreateStringArray
    cJSON_CreateStringReference
    cJSON_CreateTrue
    cJSON_Delete
    cJSON_DeleteItemFromArray
    cJSON_DeleteItemFromObject
    cJSON_DeleteItemFromObjectCaseSensitive
    cJSON_DetachItemFromArray
    cJSON_DetachItemFromObject
    cJSON_DetachItemFromObjectCaseSensitive
    cJSON_DetachItemViaPointer
    cJSON_Duplicate
    cJSON_GetArrayItem
    cJSON_GetArraySize
    cJSON_GetErrorPtr
    cJSON_GetNumberValue
    cJSON_GetObjectItem
    cJSON_GetObjectItemCaseSensitive
    cJSON_GetStringValue
    cJSON_HasObjectItem
    cJSON_InitHooks
    cJSON_InsertItemInArray
    cJSON_IsArray
    cJSON_IsBool
    cJSON_IsFalse
    cJSON_IsInvalid
    cJSON_IsNull
    cJSON_IsNumber
    cJSON_IsObject
    cJSON_IsRaw
    cJSON_IsString
    cJSON_IsTrue
    cJSON_Minify
    cJSON_Parse
    cJSON_ParseWithLength
    cJSON_ParseWithLengthOpts
    cJSON_ParseWithOpts
    cJSON_Print
    cJSON_PrintBuffered
    cJSON_PrintPreallocated
    cJSON_PrintUnformatted
    cJSON_ReplaceItemInArray
    cJSON_ReplaceItemInObject
    cJSON_ReplaceItemInObjectCaseSensitive
    cJSON_ReplaceItemViaPointer
    cJSON_SetNumberHelper
    cJSON_SetValuestring
    cJSON_Version
    cJSON_free
    cJSON_malloc
    )
foreach(cjson_sym ${TUYAOPEN_CJSON_SYMBOLS})
    add_definitions(-D${cjson_sym}=tuyaopen_${cjson_sym})
endforeach()
