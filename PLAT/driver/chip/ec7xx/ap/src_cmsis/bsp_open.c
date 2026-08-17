#ifdef MBTK_OPENCPU_SUPPORT

#include "Driver_USART.h"
#include "Driver_SPI.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"


#if (RTE_UART0)
extern ARM_DRIVER_USART Driver_USART0;
#endif

#if (RTE_UART1)
extern ARM_DRIVER_USART Driver_USART1;
#endif

#if (RTE_UART2)
extern ARM_DRIVER_USART Driver_USART2;
#endif

#if (RTE_UART3)
extern ARM_DRIVER_USART Driver_USART3;
#endif

#if(RTE_I2C0)
extern ARM_DRIVER_I2C Driver_I2C0;
#endif

#if(RTE_I2C1)
extern ARM_DRIVER_I2C Driver_I2C1;
#endif

#if(RTE_SPI0)
extern ARM_DRIVER_SPI Driver_SPI0;
#endif

#if(RTE_SPI1)
extern ARM_DRIVER_SPI Driver_SPI1;
#endif


ARM_DRIVER_USART* Mbtk_Get_Driver_USARTx(int dev_id)
{
    if(0 == dev_id)
    {
        #if (RTE_UART0)
        return &Driver_USART0;
        #endif
    }
    else if(1 == dev_id)
    {
        #if (RTE_UART1)
		// tuya use lpusart1, it can be wakeup at 9600 if it is in lowpower mode
        extern ARM_DRIVER_USART Driver_LPUSART1;
        return &Driver_LPUSART1;
        #endif
    }
    else if(2 == dev_id)
    {
        #if (RTE_UART2)
        return &Driver_USART2;
        #endif
    }
    else if(3 == dev_id)
    {
        #if (RTE_UART3)
        return &Driver_USART3;
        #endif
    }

    return NULL;
}

ARM_DRIVER_I2C* Mbtk_Get_Driver_I2Cx(int dev_id)
{
    if(0 == dev_id)
    {
        #if (RTE_I2C0)
        return &Driver_I2C0;
        #endif
    }
    else if(1 == dev_id)
    {
        #if (RTE_I2C1)
        return &Driver_I2C1;
        #endif
    }

    return NULL;
}

ARM_DRIVER_SPI* Mbtk_Get_Driver_SPIx(int dev_id)
{
    if(0 == dev_id)
    {
        #if (RTE_SPI0)
        return &Driver_SPI0;
        #endif
    }
    else if(1 == dev_id)
    {
        #if (RTE_SPI1)
        return &Driver_SPI1;
        #endif
    }

    return NULL;
}

#endif //MBTK_OPENCPU_SUPPORT
