#ifndef __SHT21_H__
#define __SHT21_H__

#define SHT21_ADDR 0x40

bool ICACHE_FLASH_ATTR sht21_getTemperature(float *temperature);
bool ICACHE_FLASH_ATTR sht21_getHumidity(float *humidity);
bool ICACHE_FLASH_ATTR sht21_getUserRegister(uint8_t *data);
bool ICACHE_FLASH_ATTR sht21_setUserRegister(uint8_t data);
bool ICACHE_FLASH_ATTR sht21_softReset(void);

#endif