
#include "bl_reset.h"

LastResetState_e blResetReasonTranslate(ResetReason_e resetReason)
{
    LastResetState_e ret = LAST_RESET_CLEAR;
    switch(resetReason)
    {
        case RESET_REASON_CLEAR:
        {
            ret = LAST_RESET_SWRESET;
            break;
        }
        case RESET_REASON_HARDFAULT:
        {
            ret = LAST_RESET_HARDFAULT;
            break;
        }
        case RESET_REASON_ASSERT:
        {
            ret = LAST_RESET_ASSERT;
            break;
        }
        case RESET_REASON_WDT:
        {
            ret = LAST_RESET_WDTSW;
            break;
        }
        case RESET_REASON_BAT:
        {
            ret = LAST_RESET_BATLOW;
            break;
        }
        case RESET_REASON_TEMP:
        {
            ret = LAST_RESET_TEMPHI;
            break;
        }
        case RESET_REASON_FOTA:
        {
            ret = LAST_RESET_FOTA;
            break;
        }
        default:
        {
        }
    }

    return ret;
}




