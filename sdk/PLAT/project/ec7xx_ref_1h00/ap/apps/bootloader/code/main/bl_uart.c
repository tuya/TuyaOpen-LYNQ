#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ec7xx.h"
#include "clock.h"
#include "bl_uart.h"
#include "bsp.h"
#include "sctdef.h"

extern void delay_us(uint32_t us);

static USART_TypeDef* const gUartBases[] = USART_INSTANCE_ARRAY;

static const ClockId_e g_uartClocks[] = UART_CLOCK_VECTOR;

static const ClockResetVector_t g_uartResetVectors[] = UART_RESET_VECTORS;

PLAT_BL_CIRAM_FLASH_TEXT void UART_setBaudrate(uint32_t instance, uint32_t baudrate)
{
    uint32_t uart_clock = 0;
    uint32_t div;

    uart_clock = CLOCK_getClockFreq(g_uartClocks[instance*2+1]);

    /*
     * formula to calculate baudrate, baudrate = clock_in / divisor_value,
     * where divisor_value = DIVR_INT.DIVR_FRAC(4 digits)
     */
    // round to nearest value
    div = ((uart_clock << 4) + (baudrate >> 1)) / baudrate;

    // Integer part of divisor value shall not be zero, otherwise, the result is invalid
    if ((div >> 4) == 0)
        return;

    // Disable uart first
    gUartBases[instance]->ENR = 0;
    gUartBases[instance]->DIVR = div;
}


PLAT_BL_CIRAM_FLASH_TEXT void UART_init(uint32_t instance, uartPortFrameFormat_t* format, uint32_t baudrate)
{
    if(instance == PORT_USART_INVALID) return;

    // Reset uart
    GPR_swResetModule(&g_uartResetVectors[instance]);

    // enable clock
    GPR_clockEnable(g_uartClocks[instance*2]);
    GPR_clockEnable(g_uartClocks[instance*2+1]);

    uint32_t lcr = gUartBases[instance]->LCR;

    UART_setBaudrate(instance, baudrate);

    switch(format->dataBits)
    {
        case 7:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 2U;
            break;
        default:
            // 8 bits
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 3U;
            break;
    }

    switch(format->parity)
    {
        case 1:
            // ODD
            lcr |= USART_LCR_PARITY_EN_Msk;
            lcr &= ~USART_LCR_EVEN_PARITY_Msk;
            break;
        case 2:
            // EVEN
            lcr |= (USART_LCR_PARITY_EN_Msk | USART_LCR_EVEN_PARITY_Msk);
            break;
        default:
            // NONE
            lcr &= ~USART_LCR_PARITY_EN_Msk;
            break;
    }

    switch(format->stopBits)
    {
        case 2:
            // 2 bits
            lcr &=~ USART_LCR_STOP_BIT_NUM_Msk;
            lcr |= EIGEN_VAL2FLD(USART_LCR_STOP_BIT_NUM, 2);
            break;
        default:
            // 1 bit
            lcr &=~ USART_LCR_STOP_BIT_NUM_Msk;
            break;
    }

    switch(format->flowControl)
    {
        case 3:
            // rts & cts, set RTS pin to low
            gUartBases[instance]->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
            gUartBases[instance]->MCR |= (USART_MCR_AUTO_FLOW_CTS_EN_Msk);
            break;
        case 2:
            // cts only
            gUartBases[instance]->MCR |= USART_MCR_AUTO_FLOW_CTS_EN_Msk;
            break;
        case 1:
            // set RTS pin to low
            gUartBases[instance]->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
            break;
        case 0:
        default:
            break;
    }

    gUartBases[instance]->LCR = lcr;

    gUartBases[instance]->ENR = USART_ENR_TX_EN_Msk | USART_ENR_RX_EN_Msk;

}

PLAT_BL_CIRAM_FLASH_TEXT void UART_Deinit(uint32_t instance)
{
    if(instance == PORT_USART_INVALID) return;

    // Reset uart
    GPR_swResetModule(&g_uartResetVectors[instance]);

    // enable clock
    GPR_clockDisable(g_uartClocks[instance*2]);
    GPR_clockDisable(g_uartClocks[instance*2+1]);
}

PLAT_BL_CIRAM_FLASH_TEXT void UART_flush(uint32_t instance)
{
    //write 1 and dummy read twice and clear
    gUartBases[instance]->FCR0 |= USART_FCR0_TXFIFO_FLUSH_Msk | USART_FCR0_RXFIFO_FLUSH_Msk;
    (void) gUartBases[instance]->FCR0;
    (void) gUartBases[instance]->FCR0;
    gUartBases[instance]->FCR0 &= ~(USART_FCR0_TXFIFO_FLUSH_Msk | USART_FCR0_RXFIFO_FLUSH_Msk);
}

PLAT_BL_CIRAM_FLASH_TEXT uint32_t UART_send(uint32_t instance, const uint8_t *data, uint32_t num, uint32_t timeout_us)
{
    for(uint32_t i = 0; i < num; i++)
    {
        while((EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, gUartBases[instance]->FSR) != 0) && timeout_us)
        {
            delay_us(1);
            timeout_us--;
        }

        if(timeout_us == 0)
            return i;

        // wait until tx is empty
        while((UART_readLSR(&(gUartBases[instance]->LSR)) & USART_LSR_TX_BUSY_Msk) && timeout_us)
        {
            delay_us(1);
            timeout_us--;
        }

        if(timeout_us == 0)
            return i;

        gUartBases[instance]->TDR = data[i];
    }

    return num;
}

PLAT_BL_CIRAM_FLASH_TEXT uint32_t UART_receive(uint32_t instance, uint8_t *data, uint32_t num, uint32_t timeout_us)
{
    uint32_t timeoutValue = timeout_us;

    for(uint32_t i = 0; i < num; i++)
    {
        //wait until receive data is ready
        while(((EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, gUartBases[instance]->FSR)) == 0) && timeout_us)
        {
            delay_us(1);
            timeout_us--;
        }

        if(timeout_us == 0)
            return i;
        //read data
        timeout_us = timeoutValue;
        data[i] = gUartBases[instance]->RDR;
    }

    return num;
}




