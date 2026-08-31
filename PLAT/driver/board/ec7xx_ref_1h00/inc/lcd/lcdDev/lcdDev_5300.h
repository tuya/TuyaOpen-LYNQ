#ifndef _LCD_CO5300_
#define _LCD_CO5300_
#ifdef __cplusplus
extern "C" {
#endif

#ifndef LCD_BPP_USE
#define LCD_BPP_USE         (16)    // 16:565;   18:666   24:888
#endif
#ifndef LCD_WIDTH
#define LCD_WIDTH           (412)
#endif
#ifndef LCD_HEIGHT
#define LCD_HEIGHT          (502)
#endif
#ifndef LCD_PIXEL
#define LCD_PIXEL           (LCD_HEIGHT*LCD_WIDTH)
#endif
#ifndef LCD_PPI_USE
#define LCD_PPI_USE         (334)
#endif
#ifndef LCD_INTERFACE
#define LCD_INTERFACE       MSPI_4W_II
#endif

#if (defined TYPE_EC718M)
#ifndef LCD_FREQ
#define LCD_FREQ                (50*1024*1024)
#ifndef LCD_TIME_OF_FRAME
    #define LCD_TIME_OF_FRAME   (149356)    // need fix
#endif
#endif
#else
#error "this chip not support co5300"
#endif

#ifndef LCD_X_OFFSET
#define LCD_X_OFFSET        (0)
#endif
#ifndef LCD_Y_OFFSET
#define LCD_Y_OFFSET        (0)
#endif
#ifndef LCD_TE_CYCLE
#define LCD_TE_CYCLE        (16742)     // us
#endif
#ifndef LCD_TE_WAIT_TIME
#define LCD_TE_WAIT_TIME    (623)       // us
#endif

#define DEFAULT_INST        (0x2)

#ifdef __cplusplus
}
#endif
#endif
