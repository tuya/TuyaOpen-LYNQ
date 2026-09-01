/**
 * @file board_audio_pad.c
 * @brief Weak fallback for the codec's speaker-amplifier pin hook.
 *
 * The prebuilt codec driver (es8311, in openlib/libdriver.a) calls pad_hook()
 * to learn which pin drives the external amplifier, and it is linked in
 * whenever the vendor build has AUDIO_ENABLE=true -- including builds of
 * applications that never touch audio, such as a switch demo. The real
 * implementation lives in volume_ctl.c and is compiled only with the OEM audio
 * path (ENABLE_CATX_PLAYER / ENABLE_CATX_CALL); being strong, it overrides the
 * weak definition here.
 *
 * The fallback claims no pin: with no audio in the build there is nothing to
 * amplify, and picking a pin at random would drive a line the application may
 * be using for something else.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"

/**
 * @brief Initialise the amplifier control pin and report its number.
 *
 * @return the pin number, or a negative value when the board exposes none.
 */
__attribute__((weak)) int pad_hook(void)
{
    return -1;
}
