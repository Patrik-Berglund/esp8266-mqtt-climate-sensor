#include "osapi.h"
#include "ets_sys.h"
#include "driver/i2c_master.h"
#include "driver/sht21.h"
#include "debug.h"


static bool ICACHE_FLASH_ATTR byteRead(uint8_t adr, uint8_t reg, uint8_t data[], uint8_t length){
	uint8_t i;

	i2c_master_start();

	i2c_master_writeByte((uint8_t)(adr << 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	i2c_master_writeByte(reg);
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	i2c_master_start();

	i2c_master_writeByte((uint8_t)((adr << 1) | 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	for(i = 0; i < length-1; i++){
		data[i] = i2c_master_readByte();
		i2c_master_send_ack();	
	}

	data[i] = i2c_master_readByte();
	
	i2c_master_send_nack();
	i2c_master_stop();

	return true;
}

static bool ICACHE_FLASH_ATTR byteReadNoReg(uint8_t adr, uint8_t data[], uint8_t length){
	uint8_t i;

	i2c_master_start();

	i2c_master_writeByte((uint8_t)((adr << 1) | 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	for(i = 0; i < length-1; i++){
		data[i] = i2c_master_readByte();
		i2c_master_send_ack();	
	}

	data[i] = i2c_master_readByte();
	
	i2c_master_send_nack();
	i2c_master_stop();

	return true;
}

static bool ICACHE_FLASH_ATTR singleByteWrite(uint8_t adr, uint8_t reg, uint8_t data){
	i2c_master_start();

	i2c_master_writeByte((uint8_t)(adr << 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	i2c_master_writeByte(reg);
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}
	
	i2c_master_writeByte(data);
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	i2c_master_stop();

	return true;
}

static bool ICACHE_FLASH_ATTR commandOnlyWrite(uint8_t adr, uint8_t reg){
	i2c_master_start();

	i2c_master_writeByte((uint8_t)(adr << 1));
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	i2c_master_writeByte(reg);
	if (!i2c_master_checkAck()) {
		i2c_master_stop();
		return false;
	}

	i2c_master_stop();

	return true;
}

uint8_t SHT2x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum) {   
	uint8_t crc = 0;    
	uint8_t byteCtr;   
	uint8_t bit = 0;
	uint16_t POLYNOMIAL = 0x131;

	for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr) { 
		crc ^= (data[byteCtr]); 
    	for (bit = 8; bit > 0; --bit) { 
    		if (crc & 0x80) 
    			crc = (crc << 1) ^ POLYNOMIAL;       
    		else 
    			crc = (crc << 1);     
    	}   
    }   
	if (crc != checksum) 
		return false;    
	
	return true; 
}  

bool ICACHE_FLASH_ATTR sht21_getTemperature(float *temperature){
	uint8_t data[3], i;
	uint16_t tmp;
	bool status = false;

	if(!commandOnlyWrite(SHT21_ADDR, 0xF3)){
		return false;
	}

	for(i = 0, status = false; (i < 85) && !status; i++){
		os_delay_us(1000);
		status = byteReadNoReg(SHT21_ADDR, data, 3);		
	}

	if(status){
		if(SHT2x_CheckCrc(data, 2, data[2])){
			tmp = (uint16_t)data[0] << 8 | (data[1] & 0xFC);
			*temperature = -46.85 + 175.72 * ((float) tmp / 65536);
			return true;	
		}
	}
	return false;
}
bool ICACHE_FLASH_ATTR sht21_getHumidity(float *humidity){
	uint8_t data[3], i;
	uint16_t tmp;
	bool status = false;

	if(!commandOnlyWrite(SHT21_ADDR, 0xF5)){
		return false;
	}

	for(i = 0, status = false; (i < 85) && !status; i++){
		os_delay_us(1000);
		status = byteReadNoReg(SHT21_ADDR, data, 3);		
	}

	if(status){
		if(SHT2x_CheckCrc(data, 2, data[2])){
			tmp = (uint16_t)data[0] << 8 | (data[1] & 0xFC);
			*humidity = -6.0 + 125.0 * ((float) tmp / 65536);
			return true;	
		}
	}
	return false;
}
bool ICACHE_FLASH_ATTR sht21_getUserRegister(uint8_t *data){
	return byteRead(SHT21_ADDR, 0xE7, data, 1);
}
bool ICACHE_FLASH_ATTR sht21_setUserRegister(uint8_t data){
	return singleByteWrite(SHT21_ADDR, 0xE6, data);
}
bool ICACHE_FLASH_ATTR sht21_softReset(){
	return commandOnlyWrite(SHT21_ADDR, 0xFE);
}