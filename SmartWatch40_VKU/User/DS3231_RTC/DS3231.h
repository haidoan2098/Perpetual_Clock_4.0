/*
 * DS3231.h
 *
 *  Created on: Apr 16, 2025
 *      Author: haidoan2098
 */

#ifndef DS3231_RTC_DS3231_H_
#define DS3231_RTC_DS3231_H_

#include "stm32f1xx.h"

typedef struct
{	// Tránh Padding memory => cao xuống thấp
	I2C_HandleTypeDef* I2C; // change
	uint8_t Seconds;
	uint8_t Minutes;
	uint8_t Hour;
	uint8_t Day;		// Range: 1–7
	uint8_t Date;		// Range: 00–31
	uint8_t Month;
	uint8_t Year;		// Range: 0-99
	uint8_t TxTimeBuff[3];
	uint8_t RxTimeBuff[3];
	uint8_t TxDateBuff[4];
	uint8_t RxDateBuff[4];
} DS3231_DateTime_t;

typedef struct
{
	I2C_HandleTypeDef* I2C; // change
	uint8_t Minutes;
	uint8_t Hour;
	uint8_t TxAlarm2Buff[3];
	uint8_t RxAlarm2Buff[3];
} DS3231_Alarm2_t;		// Mode Alarm when hours and minutes match

typedef enum {
    DisableAlarm2 = 0,
    EnableAlarm2
} Alarm2State;


void DS3231_DateTime_Init(I2C_HandleTypeDef* hi2c, DS3231_DateTime_t *DateTime);
void DS3231_SetTime(DS3231_DateTime_t *DS3231_Time, uint8_t Hour, uint8_t Minutes, uint8_t Seconds);
void DS3231_GetTime(DS3231_DateTime_t *DS3231_Time);
void DS3231_SetDate(DS3231_DateTime_t *DS3231_Date, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year);
void DS3231_GetDate(DS3231_DateTime_t *DS3231_Date);

void DS3231_Alarm2_Init(I2C_HandleTypeDef* hi2c, DS3231_Alarm2_t *Alarm);
void DS3231_SetAlarm2(DS3231_Alarm2_t *DS3231_Alarm2, uint8_t Minutes, uint8_t Hour);
void DS3231_GetAlarm2(DS3231_Alarm2_t *DS3231_Alarm2);
void DS3231_EnOrDisAlarm2(I2C_HandleTypeDef* I2C, uint8_t EnOrDisAlarm);
void DS3231_ClearnFlagAlarm2(I2C_HandleTypeDef* I2C);
uint8_t DS3231_GetFlagAlarm2(I2C_HandleTypeDef* I2C);
uint8_t DS3231_CheckModeAlarm2(I2C_HandleTypeDef* I2C);



#endif /* DS3231_RTC_DS3231_H_ */
