/****************************************************************************
 *
 * Description:  speak entry header file
 *
 ****************************************************************************/
#ifndef __TTS_API_H__
#define __TTS_API_H__


#include <stdint.h>
#include <stdbool.h>


typedef enum {
    TTS_TYPE_GBK,        /* GBK (default) + GB2312 + GB18030*/
    TTS_TYPE_UTF8,       /* UTF-8 */
    TTS_TYPE_UTF16LE,    /* UTF-16 little-endian */
    TTS_TYPE_UTF16BE,    /* UTF-16 big-endian */
}tts_decode_enum;

typedef struct
{
    tts_decode_enum  decode;
    char            *p_buffer;
    uint32_t         p_length;
} tts_queue_t;


void tts_init(void);
void tts_play(char* p_text, uint32_t p_length, tts_decode_enum decode);


#endif /*__TTS_API_H__*/