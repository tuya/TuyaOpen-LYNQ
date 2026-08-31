/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    kpc_defs.h
 * Description:  ec7xx keypad func code header file
 * History:      Rev1.0   2023-11-22
 *
 ****************************************************************************/
#ifndef  _KPC_DEFS_H
#define  _KPC_DEFS_H
#ifdef __cplusplus
extern "C" {
#endif

#define OK_KEY_ROW                  (1)
#define OK_KEY_COLUMN               (1)
#define OK_KEY_CODE                 (0xF2)

#define CLEAR_KEY_ROW               (2)
#define CLEAR_KEY_COLUMN            (3)
#define CLEAR_KEY_CODE              (0xF0)

#define CANCEL_KEY_ROW              (1)
#define CANCEL_KEY_COLUMN           (3)
#define CANCEL_KEY_CODE             (0xF1)

#define UP_KEY_ROW                  (4)
#define UP_KEY_COLUMN               (0)
#define UP_KEY_CODE                 (0xF3)

#define DOWN_KEY_ROW                (3)
#define DOWN_KEY_COLUMN             (2)
#define DOWN_KEY_CODE               (0xF4)

#define F1_KEY_ROW                  (0)
#define F1_KEY_COLUMN               (1)
#define F1_KEY_CODE                 (0xF5)

#define F2_KEY_ROW                  (0)
#define F2_KEY_COLUMN               (2)
#define F2_KEY_CODE                 (0xF6)

#define NULL_KEY_CODE               (0xFF)

#define FUNC_KEY_ROW                (3)
#define FUNC_KEY_COLUMN             (2)
#define FUNC_KEY_CODE               (0xFE)

#define MENU_KEY_ROW                (3)
#define MENU_KEY_COLUMN             (0)
#define MENU_KEY_CODE               (0xFD)

#define FUNC_CODE_YES               'a'
#define FUNC_CODE_NO                'b'

//在LCD V1.1和手机板上的menu yes按键互换
#define FUNC_CODE_MENU              'm'
#define FUNC_CODE_BACK              'n'
#define FUNC_CODE_DIAL              'C'
#define FUNC_CODE_DIR_U             'U'
#define FUNC_CODE_DIR_D             'D'
#define FUNC_CODE_DIR_L             'L'
#define FUNC_CODE_DIR_R             'R'

#ifdef __cplusplus
}
#endif
#endif