#include "common.h"
#include "serial.h"
#include "targets.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <string.h>
#include <math.h>

uint8_t rxPacket[16];
uint8_t txPacket[18];
uint8_t vtxModeLocked;
uint8_t pitMode = 0;

void clearSerialBuffer(void)
{
    while (serial_available()) {
        serial_read();
    }
}

void zeroRxPacket(void)
{
    memset(rxPacket, 0, sizeof(rxPacket));
}

void zeroTxPacket(void)
{
    memset(txPacket, 0, sizeof(txPacket));
}


#if !defined(LED1) && defined(LED)
#define LED1 LED
#endif

#ifdef LED1
static gpio_out_t led1_pin;
#endif
#ifdef LED2
static gpio_out_t led2_pin;
#endif
#ifdef LED3
static gpio_out_t led3_pin;
#endif

void status_leds_init(void)
{
#ifdef LED1
  led1_pin = gpio_out_setup(LED1, 1);
#endif
#ifdef LED2
  led2_pin = gpio_out_setup(LED2, 0);
#endif
#ifdef LED3
  led3_pin = gpio_out_setup(LED3, 0);
#endif
}

void status_led1(uint8_t state)
{
#ifdef LED1
  gpio_out_write(led1_pin, state);
#else
  (void)state;
#endif
}

void status_led2(uint8_t state)
{
#ifdef LED2
  gpio_out_write(led2_pin, state);
#else
  (void)state;
#endif
}

void status_led3(uint8_t state)
{
#ifdef LED3
  gpio_out_write(led3_pin, state);
#else
  (void)state;
#endif
}

void setPowerdB(float dB)
{
    myEEPROM.currPowerdB = dB;
    updateEEPROM = 1;

    if (pitMode)
    {
      rtc6705PowerAmpOff();
      target_set_power_dB(0);
    } else
    {
      rtc6705PowerAmpOn();
      target_set_power_dB(dB);
    }
}

void setPowermW(uint16_t mW)
{
    float dB = 10.0 * log10((float)mW);
    setPowerdB(dB);
}

#define BOOTLOADER_KEY  0x4f565458 // OVTX
#define BOOTLOADER_TYPE 0xACDC

struct bootloader {
    uint32_t key;
    uint32_t reset_type;
    uint32_t baud;
};

void reboot_into_bootloader(uint32_t baud)
{
    extern uint32_t _bootloader_data;
    struct bootloader * blinfo = (struct bootloader*)&_bootloader_data;
    blinfo->key = BOOTLOADER_KEY;
    blinfo->reset_type = BOOTLOADER_TYPE;
    blinfo->baud = baud;

    rtc6705PowerAmpOff();
    target_set_power_dB(0);
    delay(200);

    mcu_reboot();
}
