/*
 * Copyright (C) 2009-2018 Hanwang Technology Company
 * All rights reserved.
 *
 * file name: HWIM_Keyboard.h
 * date created: Nov 1, 2009
 * last modified: Mar 22, 2018
 */

#ifndef __HW_INPUT_METHOD_KEYBOARD__
#define __HW_INPUT_METHOD_KEYBOARD__

//#ifdef WIN32
//#define HWAPI __declspec(dllexport)
//#else
#define HWAPI
//#endif

/* languages */
enum HWLang {
        HWLANG_Unknown           = 0,

        HWLANG_Chinese_Mainland  = 0x1000,
};


/* input modes */
enum HWInputMode {
        /* Chinese */
        HWIM_INPUT_PINYIN            = 101,
};

/* keyboard modes */
enum HWKeyboardMode {
        HWIM_KEYBOARD_FULL       = 1,
        HWIM_KEYBOARD_REDUCE     = 2,
};

enum HWKeyboardCapslockStatus {
        HWIM_KEYBOARD_NORMAL = 0,
        HWIM_KEYBOARD_CAPS_UNLOCK = 1,
        HWIM_KEYBOARD_CAPS_LOCK   = 2,
};


/* fuzzy pinyins */
enum HWFuzzyPinyin {
        HW_FUZZY_Z_ZH     = 0x0001, // z<-->zh
        HW_FUZZY_C_CH     = 0x0002, // c<-->ch
        HW_FUZZY_S_SH     = 0x0004, // s<-->sh
        HW_FUZZY_F_H      = 0x0008, // f<-->h
        HW_FUZZY_K_G      = 0x0010, // k<-->g
        HW_FUZZY_L_N      = 0x0020, // l<-->n
        HW_FUZZY_L_R      = 0x0040, // l<-->r
        HW_FUZZY_AN_ANG   = 0x0080, // an<-->ang
        HW_FUZZY_EN_ENG   = 0x0100, // en<-->eng
        HW_FUZZY_IN_ING   = 0x0200, // in<-->ing
        HW_FUZZY_IAN_IANG = 0x0400, // ian<-->iang
        HW_FUZZY_UAN_UANG = 0x0800, // uan<-->uang
};

/* error codes */
enum HWError {
        HWERR_SUCCESS                  = 0,    //  成功
        HWERR_INVALID_PARAM            = -1,   //  无效参数
        HWERR_NOT_INITED               = -2,   //  未初始化
        HWERR_INSUF_BUF                = -3,   //  内存不足
        HWERR_BUF_IS_FULL              = -4,   //  内存已满
        HWERR_INVALID_INPUT            = -5,   //  无效输入
        HWERR_UNSUP_LANG               = -6,   //  不支持的语言
        HWERR_UNSUP_DIC                = -7,   //  不支持的字典
        HWERR_UNSUP_SM                 = -8,   //  不支持的搜索模式
        HWERR_UNSUP_IM                 = -9,   //  不支持的输入模式
        HWERR_UNSUP_KT                 = -10,  //  不支持的键盘
        HWERR_UNSUP_FUZZY              = -11,  //  不支持模糊音
        HWERR_UNSUP_FUNCTION           = -12,  //  不支持的功能
        HWERR_NO_RAM                   = -13,  //  无内存
        HWERR_NO_LOADER                = -14,  //  未加载
        HWERR_NO_RELEASER              = -15,  //  未释放
        HWERR_NO_SAVER                 = -16,  //  未存储
        HWERR_NO_LDIC                  = -17,  //  无学习字典
        HWERR_NO_USER_DIC              = -18,  //  无用户字典
        HWERR_NO_PHONE_DIC             = -19,  //  无电话字典
        HWERR_ALREADY_EXISTS           = -20,  //  已存在
        HWERR_TOO_LARGE_ENTRY          = -21,  //  输入太多

        HWERR_INDEX_OUT_OF_BOUNDS      = -40,  //  越界

        HWERR_EXPIRED                  = -100,  //  过期
        /* file */
        HWERR_OPEN_FILE_FAIL           = -101,  // 无法打开文档
        HWERR_READ_FILE_FAIL           = -102,  // 无法读取文档
        HWERR_WRITE_FILE_FAIL          = -103,  // 无法写入文档
        HWERR_LOAD_FILE_FAIL           = -104,  // 无法加载文档

        /* memory */
        HWERR_INSUF_MEM                = -201,  // 内存错误
};

/* basic constants*/
enum {
        /* minimum ram size */
        HWIM_KEYBOARD_MIN_RAMSIZE = 1 * 1024 * 1024,

        /* max input length */
        HWIM_MAX_INPUT_LEN   = 64,

        /* syllables buffer size  */
        HWIM_SYLLABLES_BUFSIZE = 1024,

        /* phrase buffer size */
        HWIM_PHRASE_BUFSIZE = 1024,

        /* candidates buffer size */
        HWIM_CANDIDATES_BUFSIZE = 8192,

        /* full input buffer size */
        HWIM_FULL_INPUT_BUFSIZE = 1024,

        /* next valid key buffer size */
        HWIM_NEXT_VALID_KEY_BUFSIZE = 128,
};

/**
 * HWCBK_LoadDict - file loader callback
 * @path  file path
 * @pSize file size pointer
 *
 * return file memory pointer
 */
typedef void *(*HWCBK_LoadDict)(const char *path, unsigned int *pSize);

/**
 * HWCBK_ReleaseDict - memory releaser callback
 * @ptr memory pointer
 *
 * return error code
 */
typedef int (*HWCBK_ReleaseDict)(void *ptr);

/**
 * HWCBK_SaveDict - file saver callback
 * @ptr memory pointer
 * @size memory size
 * @path file path
 *
 * return error code
 */
typedef int (*HWCBK_SaveDict)(const void *ptr , unsigned int size,
                              const char *path);

/* handle structure */
typedef struct tagHWIM_HANDLE {
        unsigned int Handle[128];
} HWIM_HANDLE;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * HWKIM_GetVersion - get SDK version
 *
 * return version string
 */
HWAPI const char *HWKIM_GetVersion(void);

/**
 * HWKIM_Init - initialize handle
 * @pHandle handler pointer
 * @pLoader file loader callback
 * @pReleaser memory releaser callback
 * @pSaver file saver callback
 *
 * return error code
 */
HWAPI int HWKIM_Init(HWIM_HANDLE *pHandle, HWCBK_LoadDict pLoader,
               HWCBK_ReleaseDict pReleaser, HWCBK_SaveDict pSaver);

/**
 * HWKIM_SetWorkspace - set workspace ram
 * @pHandle handle pointer
 * @pRam workspace ram
 * @ramSize ram size, at least HWIM_KEYBOARD_MIN_RAMSIZE
 *
 * return error code
 */
HWAPI int HWKIM_SetWorkspace(HWIM_HANDLE *pHandle, void *pRam, unsigned int ramSize);

/**
 * HWKIM_SetLanguageDict - set language and dictionary
 * @pHandle handle pointer
 * @lang language, see enum HWLang
 * @path language dictionary file path
 *
 * return error code
 */
HWAPI int HWKIM_SetLanguageDict(HWIM_HANDLE *pHandle, int lang, const char *path);


/**
 * HWKIM_ReleaseLanguageDict - release language dictionary memory
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_ReleaseLanguageDict(HWIM_HANDLE *pHandle);

/**
 * HWKIM_GetLanguage - get language
 * @pHandle handle pointer
 *
 * return language or error code
 */
HWAPI int HWKIM_GetLanguage(HWIM_HANDLE *pHandle);

/**
 * HWKIM_SetUserDict - set user dictionary
 * @pHandle handle pointer
 * @path user dictionary path
 *
 * return error code
 */
HWAPI int HWKIM_SetUserDict(HWIM_HANDLE *pHandle, const char *path);

/**
 * HWKIM_ReleaseUserDict - release user dictionary memory
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_ReleaseUserDict(HWIM_HANDLE *pHandle);

/**
 * HWKIM_SaveUserDict - save user dictionary
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_SaveUserDict(HWIM_HANDLE *pHandle);

/**
 * HWKIM_SetUserDictStatus - set user dictionary status, active/deactive
 * @pHandle handle pointer
 * @status 0: off, 1: on
 *
 * return error code
 */
HWAPI int HWKIM_SetUserDictStatus(HWIM_HANDLE *pHandle, int status);

/**
 * HWKIM_GetUserDictStatus - get user dictionary status
 * @pHandle handle pointer
 *
 * return status or error code
 */
HWAPI int HWKIM_GetUserDictStatus(HWIM_HANDLE *pHandle);

/**
 * HWKIM_ResetUserDict - reset user dictionary
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_ResetUserDict(HWIM_HANDLE *pHandle);

/**
 * HWKIM_SetKeyboardMode - set keyboard mode
 * @pHandle handle pointer
 * @keyboardMode keyboard mode, see enum HWKeyboardMode
 *
 * return error code
 */
HWAPI int HWKIM_SetKeyboardMode(HWIM_HANDLE *pHandle, int keyboardMode);

/**
 * HWKIM_GetKeyboardMode - get keyboard mode
 * @pHandle handle pointer
 *
 * return keyboard mode or error code
 */
HWAPI int HWKIM_GetKeyboardMode(HWIM_HANDLE *pHandle);

/**
 * HWKIM_SetInputMode - set input mode
 * @pHandle handle pointer
 * @inputMode input mode, see enum HWInputMode
 *
 * return error code
 */
HWAPI int HWKIM_SetInputMode(HWIM_HANDLE *pHandle, int inputMode);

/**
 * HWKIM_GetInputMode - get input mode
 * @pHandle handle pointer
 *
 * return input mode or error code
 */
HWAPI int HWKIM_GetInputMode(HWIM_HANDLE *pHandle);

/**
 * HWKIM_SetFuzzyPinyin - set fuzzy pinyin
 * @pHandle handle pointer
 * @fuzzy fuzzy pinyin code
 *
 * return error code
 */
HWAPI int HWKIM_SetFuzzy(HWIM_HANDLE *pHandle, int fuzzy);

/**
 * HWKIM_GetFuzzyPinyin - get fuzzy pinyin setting
 * @pHandle handle pointer
 *
 * return fuzzy pinyin setting or error code
 */
HWAPI int HWKIM_GetFuzzy(HWIM_HANDLE *pHandle);


/**
 * HWKIM_SetCorrectionStatus - set correction status
 * @pHandle handle pointer
 * @status 0: off, 1: on
 *
 * return error code
 */
HWAPI int HWKIM_SetCorrectionStatus(HWIM_HANDLE *pHandle, int status);

/**
 * HWKIM_GetCorrectionStatus - get correction status
 * @pHandle handle pointer
 *
 * return status or error code
 */
HWAPI int HWKIM_GetCorrectionStatus(HWIM_HANDLE *pHandle);

/**
* HWKIM_SetPredictionStatus - set prediction status
* @pHandle handle pointer
* @status 0: off, 1: on
*
* return error code
*/
HWAPI int HWKIM_SetPredictionStatus(HWIM_HANDLE *pHandle, int status);

/**
* HWKIM_GetPredictionStatus - get prediction status
* @pHandle handle pointer
*
* return status or error code
*/
HWAPI int HWKIM_GetPredictionStatus(HWIM_HANDLE *pHandle);


/**
 * HWKIM_ClearInput - clear input
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_ClearInput(HWIM_HANDLE *pHandle);


/**
 * HWKIM_AddKey - add a key
 * @pHandle handle pointer
 * @key key label
 *
 * return error code
 *
 * Description:
 *   take Pinyin for example, in full keyboard mode, @key may be "a, b, ...",
 *   in reduced keyboard mode, @key may be "2, 3, 4, ..."
 */
HWAPI int HWKIM_AddKey(HWIM_HANDLE *pHandle, unsigned int key);

/**
 * HWKIM_AddCode - add a code
 * @pHandle handle pointer
 * @key key label
 *
 * return error code
 *
 * Description:
 *   take Pinyin for example, in reduce keyboard mode, @key may be "2",
 *   if the key is '2',then @code may be "a,b,c"
 */
HWAPI int HWKIM_AddCode(HWIM_HANDLE *pHandle, unsigned int code);

/**
 * HWKIM_AddSeparator - add separator
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_AddSeparator(HWIM_HANDLE *pHandle);

/**
 * HWKIM_Backspace - backspace a key, unselect a candidate or unselect a
 *                   syllable
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_Backspace(HWIM_HANDLE *pHandle);


/**
 * HWKIM_GetPhrase - get phrase
 * @pHandle handle pointer
 * @phraseBuf phrase buffer
 * @bufSize phrase buffer size, at least HW_PHRASE_BUFSIZE
 *
 * return phrase length or error code
 *
 * Description:
 *   take Pinyin for example, first input "beijing", then PHRASE would be
 *   "BeiJing", and candidates would be "北京, ...", select "北", then PHRASE
 *   would be "北Jing", and candidates would be "京, ..."
 */
HWAPI int HWKIM_GetPhrase(HWIM_HANDLE *pHandle, unsigned int *phraseBuf,
                    unsigned int bufSize);

/**
 * HWKIM_GetEnterPhrase - get EnterPhrase
 * @pHandle handle pointer
 * @phraseBuf phrase buffer
 * @bufSize phrase buffer size, at least HW_PHRASE_BUFSIZE
 *
 * return phrase length or error code
 *
 * Description:
 *   take Pinyin for example, first input "beijing" and select candidate"北", 
 *   then EnterPhrase would be "北jing"
 */
HWAPI int HWKIM_GetEnterPhrase(HWIM_HANDLE *pHandle, unsigned int *phraseBuf,
                    unsigned int bufSize);

/**
 * HWKIM_GetSpacePhrase - get SpacePhrase
 * @pHandle handle pointer
 * @phraseBuf phrase buffer
 * @bufSize phrase buffer size, at least HW_PHRASE_BUFSIZE
 *
 * return phrase length or error code
 *
 * Description:
 *   take Pinyin for example, first input "beijing" and select candidate"北", 
 *   then SpacePhrase would be "北京"
 */
HWAPI int HWKIM_GetSpacePhrase(HWIM_HANDLE *pHandle, unsigned int *phraseBuf,
	unsigned int bufSize);

/**
 * HWKIM_GetSpelling - get spelling
 * @pHandle handler pointer
 * @spellingBuf spelling buffer
 * @bufSize spelling buffer size
 *
 * return spelling length or error code
 *
 * Description:
 *   take Pinyin for example, first input "beijing", then SPELLING would be
 *   "BeiJing", and candidates would be "北京, ...", select "北", then SPELLING
 *   would be "Jing", and candidates would be "京, ..."
 */
HWAPI int HWKIM_GetSpelling(HWIM_HANDLE *pHandle, unsigned int *spellingBuf,
                      unsigned int bufSize);

/**
 * HWKIM_GetInput - get original input string
 * @pHandle handle pointer
 * @inputBuf input string buffer
 * @bufSize input string buffer size
 *
 * return input string length or error code
 */
HWAPI int HWKIM_GetInput(HWIM_HANDLE *pHandle, unsigned int *inputBuf,
                   unsigned int bufSize);

/**
 * HWKIM_GetSyllables - get first syllables
 * @pHandle handle pointer
 * @syllableBuf syllable buffer
 * @bufSize syllable buffer size, at least HW_SYLLABLES_BUFSIZE
 *
 * return syllable count or error code
 */
HWAPI int HWKIM_GetSyllables(HWIM_HANDLE *pHandle, unsigned int *syllableBuf,
                       unsigned int bufSize);

/**
 * HWKIM_GetCandidates - get candidates
 * @pHandle handle pointer
 * @candBuf candidate buffer
 * @bufSize candidate buffer size, at least HW_CANDIDATES_BUFSIZE, counted in
 *          sizeof(unsigned int)
 *
 * return candidate count or error code
 */
HWAPI int HWKIM_GetCandidates(HWIM_HANDLE *pHandle, unsigned int *candBuf,
                        unsigned int bufSize);

/**
 * HWKIM_SelectSyllable - select syllable
 * @pHandle handle pointer
 * @sylIndex syllable index
 *
 * return error code
 */
HWAPI int HWKIM_SelectSyllable(HWIM_HANDLE *pHandle, unsigned int sylIndex);

/**
 * HWKIM_SelectCandidate - select candidate
 * @pHandle handle pointer
 * @candIndex candidate index
 *
 * return error code
 */
HWAPI int HWKIM_SelectCandidate(HWIM_HANDLE *pHandle, unsigned int candIndex);

/**
 * HWKIM_ConversionIsComplete - conversion is complete
 * @pHandle handle pointer
 *
 * return 1 if Yes, 0 if No, or error code
 */
HWAPI int HWKIM_ConversionIsComplete(HWIM_HANDLE *pHandle);

/**
 * HWKIM_GetFullInput - get full input
 * @pHandle handle pointer
 * @inputBuf full input buffer
 * @bufSize full input buffer size, at least HW_FULL_INPUT_BUFSIZE
 *
 * return full input length, or error code
 */
HWAPI int HWKIM_GetFullInput(HWIM_HANDLE *pHandle, unsigned int *inputBuf,
                       unsigned int bufSize);


/**
 * HWKIM_SetPredictContext - set prediction context
 * @pHandle handle pointer
 * @str context string
 * @strLen context string length
 * @learn_pred boolean type, learn word predictions or not
 * @is_full_pre boolean type, full prediction or not
 * return error code
 */
HWAPI int HWKIM_SetPredictContext(HWIM_HANDLE *pHandle, const unsigned int *str,
                            unsigned int strLen, int learn_pred, int is_full_pre);

/**
 * HWKIM_ClearPredictContext - clear prediction context
 * @pHandle handle pointer
 *
 * return error code
 */
HWAPI int HWKIM_ClearPredictContext(HWIM_HANDLE *pHandle);

/**
 * HWKIM_AddUserWord - add user word
 * @pHandle handle pointer
 * @word user word
 * @len user word length
 */
HWAPI int HWKIM_AddUserWord(HWIM_HANDLE *pHandle, const unsigned int *word,
                      unsigned int len);

/**
* HWKIM_SetCapslockStatus - set caps lock status
* @pHandle handle pointer
* @status 0: normal, 1: unlock, 2: lock
*
* return error code
*/
HWAPI int HWKIM_SetCapslockStatus(HWIM_HANDLE *pHandle, int status);

#ifdef __cplusplus
}
#endif

#endif
