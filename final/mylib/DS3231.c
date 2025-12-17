#include "DS3231.h"

//I2C Handle
extern I2C_HandleTypeDef hi2c1;

//convert Dec to BCD
uint8_t Decimal2BCD(uint8_t num)
{ 
  return (num/10)<<4|(num%10);
}

//convert BCD to Dec
uint8_t BCD2Decimal(uint8_t num) {
 return (num>>4)*10+(num&0x0F);
}

void RTC_Init(Datetime *dt){
	dt->hour = 23;
	dt->min = 35;
	dt->second = 0;
    dt->day = 2;
    dt->date = 16;
    dt->month = 12;
    dt->year = 25;
}
void RTC_Write(Datetime *dt){
	uint8_t buff[8];
	buff[0] = 0x00;
	buff[1] = Decimal2BCD(dt->second);
	buff[2] = Decimal2BCD(dt->min);
	buff[3] = Decimal2BCD(dt->hour);
	buff[4] = Decimal2BCD(dt->day);
	buff[5] = Decimal2BCD(dt->date);
	buff[6] = Decimal2BCD(dt->month);
	buff[7] = Decimal2BCD(dt->year);
	HAL_I2C_Master_Transmit(&hi2c1, RTC_ADDR, buff, 8, 100);
	
}


void RTC_Read(Datetime *dt){
	uint8_t buff[8];
	uint8_t addr_reg = 0x00;
	
	// G?i d?a ch? b?t d?u d?c là 0x00
	HAL_I2C_Master_Transmit(&hi2c1, RTC_ADDR, &addr_reg, 1, 100);
	
	// Ð?c 7 byte d? li?u (t? 0x00 d?n 0x06)
	HAL_I2C_Master_Receive(&hi2c1, RTC_ADDR, buff, 7, 100);
	
	dt->second = BCD2Decimal(buff[0]);
	dt->min    = BCD2Decimal(buff[1]);
	
	// S?A L?I: Mask bit 0x3F d? lo?i b? bit 12/24h ? bit 6
	dt->hour   = BCD2Decimal(buff[2] & 0x3F); 
	
	dt->day    = BCD2Decimal(buff[3]);
	dt->date   = BCD2Decimal(buff[4]);
	
	// S?A L?I: Mask bit 0x1F d? lo?i b? bit Century ? bit 7
	dt->month  = BCD2Decimal(buff[5] & 0x1F);
	
	dt->year   = BCD2Decimal(buff[6]);
}