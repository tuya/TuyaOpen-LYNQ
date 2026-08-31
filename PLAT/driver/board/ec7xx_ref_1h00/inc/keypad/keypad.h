/****************************************************************************
 *
 * Copy right:   2020-, Copyrigths of EigenComm Ltd.
 * File name:    tp.h
 * Description:  EC618 tp driver file
 * History:      Rev1.0   2020-12-17
 *
 ****************************************************************************/

#ifndef _KEYPAD_EC718_H
#define _KEYPAD_EC718_H

#include "ec7xx.h"
#include "Driver_Common.h"

#include "kpc.h"
#include "kpc_defs.h"
/**
  \addtogroup tp_interface_gr
  \{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** \brief KEYPAD location */
// ROW PIN
/* GPIO27 */
#define KEYPAD_ROW_0_PAD_INDEX      (52)
#define KEYPAD_ROW_0_ALT_FUNC       (PAD_MUX_ALT6)

/* GPIO26 */
#define KEYPAD_ROW_1_PAD_INDEX      (51)
#define KEYPAD_ROW_1_ALT_FUNC       (PAD_MUX_ALT6)

/* GPIO25 */
#define KEYPAD_ROW_2_PAD_INDEX      (50)
#define KEYPAD_ROW_2_ALT_FUNC       (PAD_MUX_ALT6)

/* GPIO24 */
#define KEYPAD_ROW_3_PAD_INDEX      (49)
#define KEYPAD_ROW_3_ALT_FUNC       (PAD_MUX_ALT6)

/* GPIO23-AGPIO3 */
#define KEYPAD_ROW_4_PAD_INDEX      (48)
#define KEYPAD_ROW_4_ALT_FUNC       (PAD_MUX_ALT6)


// COLUMNS PIN
/* GPIO13 */
#define KEYPAD_COLUMN_0_PAD_INDEX   (28)
#define KEYPAD_COLUMN_0_ALT_FUNC    (PAD_MUX_ALT6)

/* GPIO12 */
#define KEYPAD_COLUMN_1_PAD_INDEX   (27)
#define KEYPAD_COLUMN_1_ALT_FUNC    (PAD_MUX_ALT6)

/* GPIO20 */
#define KEYPAD_COLUMN_2_PAD_INDEX   (45)
#define KEYPAD_COLUMN_2_ALT_FUNC    (PAD_MUX_ALT6)

/* GPIO21 */
#define KEYPAD_COLUMN_3_PAD_INDEX   (46)
#define KEYPAD_COLUMN_3_ALT_FUNC    (PAD_MUX_ALT6)

#define ROWS                        (5)
#define COLUMNS                     (4)


/*******************************************************************************
 * API
 ******************************************************************************/

 #ifdef __cplusplus
 extern "C" {
#endif
void keypadInit(void);
void keypadScan(void);
void keypadLoop(void);
int32_t KPC_eventQueueRead(KpcReportEvent_t *eventPtr);

#ifdef __cplusplus
}
#endif

#endif
