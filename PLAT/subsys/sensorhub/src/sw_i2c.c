
#include <stdio.h>
#include "pad.h"
#include "gpio.h"
#include "ic.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "bsp.h"

#include "sw_i2c.h"

static sw_i2c_bus i2c_bus;




static void wait_us(uint32_t nCount)
{
    delay_us(nCount);
}

static void bus_sda_mode(uint8_t output)
{
    GpioPinConfig_t config;
    if(output)
    {
        if(GPIO_pinRead(SDA_GPIO_INSTANCE, SDA_GPIO_PIN)){
            config.misc.initOutput = 1;
        }
        else {
            config.misc.initOutput = 0;
        }
        config.pinDirection = GPIO_DIRECTION_OUTPUT; 
        GPIO_pinConfig(SDA_GPIO_INSTANCE, SDA_GPIO_PIN, &config);
    }
    else {
        config.pinDirection = GPIO_DIRECTION_INPUT;
        GPIO_pinConfig(SDA_GPIO_INSTANCE, SDA_GPIO_PIN, &config);
    }
}

static void bus_sda_level(uint8_t level)
{
    if(level)
    {
        GPIO_pinWrite(SDA_GPIO_INSTANCE, 1 << SDA_GPIO_PIN, 1 << SDA_GPIO_PIN);
    }
    else {
        GPIO_pinWrite(SDA_GPIO_INSTANCE, 1 << SDA_GPIO_PIN, 0);
    }
}
static void bus_scl_level(uint8_t level)
{
    if(level)
    {
        GPIO_pinWrite(SCL_GPIO_INSTANCE, 1 << SCL_GPIO_PIN, 1 << SCL_GPIO_PIN);
    }
    else {
        GPIO_pinWrite(SCL_GPIO_INSTANCE, 1 << SCL_GPIO_PIN, 0);
    }
}
static uint8_t bus_sda_val(void)
{
    if(GPIO_pinRead(SDA_GPIO_INSTANCE, SDA_GPIO_PIN)) return 1;
    return 0;
}

static sw_i2c_bus i2c_bus = {
    .interval = 6,
    .val = bus_sda_val,
    .sda = bus_sda_level,
    .scl = bus_scl_level,
    .output = bus_sda_mode,
    .delay = wait_us
};

static void i2c_start(sw_i2c_bus bus)
{
    bus.output(1);
    bus.sda(1);
    bus.scl(1);
    bus.delay(bus.interval);
    bus.sda(0);
    bus.delay(bus.interval);
    bus.scl(0);
}
static void i2c_stop(sw_i2c_bus bus)
{
    bus.output(1);
    bus.sda(0);
    bus.scl(1);
    bus.delay(bus.interval);
    bus.sda(1);
}

static uint8_t i2c_wait(sw_i2c_bus bus,uint8_t us)
{
    uint8_t timeout = us;
    bus.output(0);
    bus.sda(1);
    bus.delay(bus.interval);
    bus.scl(1);
    bus.delay(bus.interval/2);
    while(bus.val() && timeout)
	{
        bus.delay(1);
		timeout--;
	}
    uint8_t ack = bus.val();
    bus.scl(0);
    bus.delay(bus.interval);
    bus.output(1);
	return ack; 
} 

static void i2c_ack(sw_i2c_bus bus)
{
    bus.output(1);
    bus.sda(0);
    bus.delay(bus.interval);
    bus.scl(1);
    bus.delay(bus.interval);
    bus.scl(0);
    bus.delay(bus.interval);
    bus.sda(1);
}

static void i2c_nack(sw_i2c_bus bus)
{
    bus.output(1);
    bus.sda(1);
    bus.scl(1);    
    bus.delay(bus.interval);
    bus.scl(0);
    bus.delay(bus.interval);
}


static void i2c_char(sw_i2c_bus bus, uint8_t data)
{ 
    uint8_t txd = data; 
	bus.output(1);
	bus.scl(0);
	for(uint8_t t = 0;t < 8;t++)
	{ 
        if(txd & 0x80) bus.sda(1);
        else bus.sda(0);
		txd <<= 1; 
		bus.delay(bus.interval);
		bus.scl(1);
		bus.delay(bus.interval);
		bus.scl(0);
	}
} 
static uint8_t i2c_byte(sw_i2c_bus bus)
{   
    uint8_t receive = 0;  
    bus.output(0);
    for(uint8_t i = 0;i < 8;i++)
    {  
        receive <<= 1;
        bus.scl(1);
        bus.delay(bus.interval/2);
        receive += bus.val();
        bus.delay(bus.interval/2);
        bus.scl(0);
        bus.delay(bus.interval);
    }
	return receive;
}

static uint8_t i2c_bus_send(sw_i2c_bus bus, uint8_t addr,uint8_t reg,uint16_t len,uint8_t *data)
{     
    i2c_start(bus); 
	i2c_char(bus,addr*2+WRITE_CMD);  
    i2c_wait(bus,10); 
    i2c_char(bus,reg); 
    i2c_wait(bus,10);
    for(uint16_t n = 0;n < len;n++)
	{ 
        i2c_char(bus,data[n]);   
        i2c_wait(bus,10); 
    }
    i2c_stop(bus);
	return 0;
}

static uint8_t i2c_bus_wait(sw_i2c_bus bus,uint8_t addr,uint16_t len,uint8_t *data)
{ 
	uint8_t receive = 0;
    uint8_t sel = 1;
    i2c_start(bus); 
	i2c_char(bus,addr*2+READ_CMD);  
    i2c_wait(bus,10);
    for(uint16_t n = 0;n < len;n++)
	{ 
        data[n]=i2c_byte(bus);
        bus.output(1);
        if(n < (len-1)) i2c_ack(bus);
    }
    i2c_nack(bus);
    i2c_stop(bus);
	return 0;
} 
static uint8_t i2c_bus_read(sw_i2c_bus bus,uint8_t addr,uint8_t reg,uint16_t len,uint8_t *data)
{ 
    uint8_t ret = 0;
    i2c_start(bus); 
	i2c_char(bus,addr*2+WRITE_CMD);  
    i2c_wait(bus,10);
    i2c_char(bus,reg);  
    i2c_wait(bus,10);
    i2c_start(bus); 
    i2c_char(bus,addr*2+READ_CMD);  
    i2c_wait(bus,10);
    for(uint16_t n = 0;n < len;n++)
	{
        data[n]=i2c_byte(bus);
        bus.output(1);
        if(n < (len-1)) i2c_ack(bus);
    }
    i2c_nack(bus);
    i2c_stop(bus);
	return ret;
} 

static uint8_t i2c_bus_scan(sw_i2c_bus bus,uint8_t *addr,uint8_t max)
{ 
    uint8_t num = 0;
    for(uint8_t i = 1; i<127; i++)
    {
        i2c_start(bus);  
        i2c_char(bus,i*2);  
        uint8_t ack = i2c_wait(bus,10); 
        i2c_stop(bus); 
        if(ack==0){
            addr[num++] = i;
            if(num >= max) break;
        }
    }
    return num;
}


uint8_t sw_i2c_wait(uint8_t addr,uint8_t *data,uint16_t len,uint32_t time)
{
    i2c_bus.delay(time);
    return i2c_bus_wait(i2c_bus, addr, len, data);
}
uint8_t sw_i2c_send(uint8_t addr,uint8_t reg,uint16_t len,uint8_t *data)
{     
	return i2c_bus_send(i2c_bus, addr, reg, len, data);
}
uint8_t sw_i2c_read(uint8_t addr,uint8_t reg,uint16_t len,uint8_t *data)
{     
	return i2c_bus_read(i2c_bus, addr, reg, len, data);
}
uint8_t sw_i2c_scan(uint8_t *data,uint8_t max)
{
    // taskENTER_CRITICAL();
    uint8_t num = i2c_bus_scan(i2c_bus, data, max); 
    // taskEXIT_CRITICAL();
    for(uint8_t i = 0; i< num; i++){
        printf("[%d]i2c:0x%X\r\n",i,data[i]);
    }
}


uint8_t sw_i2c_init(uint8_t sn)
{
    PadConfig_t padConfig;
    GpioPinConfig_t config;
    PAD_getDefaultConfig(&padConfig);

    padConfig.mux = SDA_PAD_ALT_FUNC;
    PAD_setPinConfig(SDA_PAD_INDEX, &padConfig);
    PAD_setPinPullConfig(SDA_PAD_INDEX, PAD_INTERNAL_PULL_UP);

    padConfig.mux = SCL_PAD_ALT_FUNC;
    PAD_setPinConfig(SCL_PAD_INDEX, &padConfig);
    PAD_setPinPullConfig(SCL_PAD_INDEX, PAD_INTERNAL_PULL_UP);

    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 1;
    GPIO_pinConfig(SCL_GPIO_INSTANCE, SCL_GPIO_PIN, &config);
    // GPIO_pinConfig(SDA_GPIO_INSTANCE, SDA_GPIO_PIN, &config);

    config.pinDirection = GPIO_DIRECTION_INPUT;
    GPIO_pinConfig(SDA_GPIO_INSTANCE, SDA_GPIO_PIN, &config);
}



bus_t sw_i2c_default(void)
{
    bus_t i2c_bus = {
        .i2c_init = sw_i2c_init,
        .i2c_scan = sw_i2c_scan,
        .i2c_wait = sw_i2c_wait,
        .i2c_read = sw_i2c_read,
        .i2c_send = sw_i2c_send
    };
    i2c_bus.i2c_init(0);
    return i2c_bus;
}