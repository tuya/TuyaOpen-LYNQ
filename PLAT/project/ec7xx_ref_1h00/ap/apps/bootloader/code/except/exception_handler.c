#include <stdio.h>
#include <stdarg.h>
#include <string.h>
//#include "gpr.h"
#include "wdt.h"
#include "bl_bsp.h"

#ifndef T_USART
#define T_USART USART_TypeDef
#endif


#ifndef USART_0
#define USART_0   ((T_USART*)USART0_BASE_ADDR)
#endif

#ifndef USART_1
#define USART_1   ((T_USART*)USART1_BASE_ADDR)
#endif

#define EXCEPT_AUTO_REBOOT_FLAG 1

#ifndef EXCEPT_AUTO_REBOOT_FLAG
#define EXCEPT_AUTO_REBOOT_FLAG 0
#endif

#define USART_LSR_TX_EMPTY         ((uint32_t)0x00000040)

void execpt_usart_divide_latch(T_USART *usart,unsigned int val)
{
#if 0
	unsigned int rd_data = 0;
	rd_data = usart->LCR;
	rd_data = (rd_data&0xffffff7f )|(val <<7);
	usart->LCR = rd_data;
#endif
}

void except_usart_divided(T_USART *usart,unsigned int dlh,unsigned int dll)
{
#if 0
	execpt_usart_divide_latch(usart,1);
	usart->DLH = dlh;
	usart->DLL = dll;
	execpt_usart_divide_latch(usart,0);
#endif
}

void except_usart_init(T_USART *usart)
{
#if 0
	//uart pimux set up
	//618 pad is same with 617, so also need to pull up, config 510
	*((uint32_t *)0x4d06007C)=0x00000510;//RX, padAddr:31
	*((uint32_t *)0x4d060080)=0x00000010;//TX, padAddr:32

	except_usart_divided(usart,0,27);
	//usart->EFCR = 0x700;

	usart->IER = 0x0f;
	usart->FCR = 0x51;
	usart->LCR = 0x3;
	usart->MFCR = 0x1;
#endif
}

void except_usart_send(T_USART *usart,uint16_t Data)
{
#if 0
	usart->THR = (Data & (uint16_t)0x01FF);
#endif
}

int except_usart_is_txrdy(T_USART *usart)
{
#if 0
	int bitstatus = 0;
	if ( (usart->LSR & USART_LSR_TX_EMPTY) != 0)
	{
		bitstatus = 1;
	}
	else
	{
		bitstatus = 0;
	}

	return bitstatus;
#endif
    return 0;
}

int except_fputc(int ch, FILE *f)
{
#if 0
    unsigned int cnt  =0;
    except_usart_send(USART_0,(uint8_t) ch);

    while (1)
    {
        if (except_usart_is_txrdy(USART_0) != 0)
        {
            //tx finish
            break;
        }
        if (cnt > 70000)
        {
            //when timeout just return
            break;
        }
        cnt++;
    };
#endif
    return (ch);

}

void except_trace(char*log,int len)
{
#if 0

    int i;
    for(i=0;i<len;i++)
    {
        except_fputc(*((char *)log+i),NULL);
    }
#endif
}

void except_config_wdt(void)
{
#if 0
    //when rx data received, do not reboot
    #if (EXCEPT_AUTO_REBOOT_FLAG !=0)
        //config wdt
        BSP_WdtInit();
        WDT_start();
    #else
        WDT_stop();
    #endif
#endif
}


void exception_init(void)
{
#if 0
    int i;

    //reset uart0, 51M, 115200
    #if 0
    GPR_clockEnable(PCLK_UART0) ;
    GPR_clockDisable(FCLK_UART0);
    GPR_setClockSrc(FCLK_UART0, FCLK_UART0_SEL_51M);
    GPR_clockEnable(FCLK_UART0);
    GPR_swReset(RST_FCLK_UART0);
	#endif

    for(i=0;i<70000;i++)
    {
    }
    except_usart_init(USART_0);

    except_config_wdt();
    except_trace("BL Exception called!\r\n", sizeof("BL Exception called!\r\n"));

#if (EXCEPT_AUTO_REBOOT_FLAG !=0)
    except_trace("Waiting wdg reset!\r\n", sizeof("Waiting wdg reset!\r\n"));
#endif
#endif
}

#if defined(__CC_ARM)

__asm void CommonFault_Handler(void)
{
        extern exception_init

        PRESERVE8
        push {lr}
        ldr r0, =exception_init
        blx  r0
        pop {lr}
        bx lr
}

__asm void HardFault_Hook_Handler(void)
{
        PRESERVE8
        push {lr}
        ldr r0, =CommonFault_Handler
        blx r0
        b .
        nop
        nop
}

#elif defined(__GNUC__)

// todo: fix me
void CommonFault_Handler(void)
{
    asm volatile(
        ".extern exception_init\n\t"
        "push {lr}\n\t"
        "ldr r0, =exception_init\n\t"
        "blx  r0\n\t"
        "pop {lr}\n\t"
        "bx lr\n\t"
    );

}

void HardFault_Hook_Handler(void)
{
    asm volatile(
        "push {lr}\n\t"
        "ldr r0, =CommonFault_Handler\n\t"
        "blx  r0\n\t"
        "b .\n\t"
        "bx lr\n\t"

    );

}
#endif

