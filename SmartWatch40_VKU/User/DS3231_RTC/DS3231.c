/*
 * DS3231.c
 *
 *  Created on: Apr 16, 2025
 *      Author: haidoan2098
 */

#include "DS3231.h"

#define SLAVE_DS3231_ADDRESS 		   (0x68 << 1)

#define DS3231_TIME_REGISTER_ADDRESS 	0x00
#define DS3231_DATE_REGISTER_ADDRESS 	0x03
#define DS3231_ALARM2_REGISTER_ADDRESS 	0x0B
#define DS3231_CONTROL_REGISTER_ADDRESS 0x0E
#define DS3231_STATUS_REGISTER_ADDRESS  0x0F

#define DS3231_CONTROL_BIT_A2IE       (1 << 1)  // Enable Alarm2 interrupt
#define DS3231_CONTROL_BIT_INTCN      (1 << 2)  // Use INT output instead of square wave
#define DS3231_FLAG_STATUS_A2F        (1 << 1)  // Alarm2 triggered flag

#define DS3231_FLAG_A2F_OFF 0
#define DS3231_FLAG_A2F_ON 1


static void Handle_I2C_Error(void)
{
	// I2C Error
}

static uint8_t DS3231_BCDtoDec(uint8_t valueBCD)
{
	return (valueBCD >> 4) * 10 + (valueBCD & 0x0F);
}

static uint8_t DS3231_DectoBCD(uint8_t valueDec)
{
	return ((valueDec / 10) << 4) | (valueDec % 10);
}

static HAL_StatusTypeDef I2C_WriteTime(DS3231_DateTime_t *DS3231_Time)
{
	return HAL_I2C_Mem_Write(DS3231_Time->I2C,
					  	  	 SLAVE_DS3231_ADDRESS,
							 DS3231_TIME_REGISTER_ADDRESS,
							 I2C_MEMADD_SIZE_8BIT,
							 DS3231_Time->TxTimeBuff,
							 sizeof(DS3231_Time->TxTimeBuff),
							 100);
}

static HAL_StatusTypeDef I2C_ReadTime(DS3231_DateTime_t *DS3231_Time)
{
	return HAL_I2C_Mem_Read(DS3231_Time->I2C,
					 	 	SLAVE_DS3231_ADDRESS,
							DS3231_TIME_REGISTER_ADDRESS,
							I2C_MEMADD_SIZE_8BIT,
							DS3231_Time->RxTimeBuff,
							sizeof(DS3231_Time->RxTimeBuff),
							100);
}

static HAL_StatusTypeDef I2C_WriteDate(DS3231_DateTime_t *DS3231_Date)
{
	return HAL_I2C_Mem_Write(DS3231_Date->I2C,
					  	  	 SLAVE_DS3231_ADDRESS,
							 DS3231_DATE_REGISTER_ADDRESS,
							 I2C_MEMADD_SIZE_8BIT,
							 DS3231_Date->TxDateBuff,
							 sizeof(DS3231_Date->TxDateBuff),
							 100);
}

static HAL_StatusTypeDef I2C_ReadDate(DS3231_DateTime_t *DS3231_Date)
{
	return HAL_I2C_Mem_Read(DS3231_Date->I2C,
					 	 	SLAVE_DS3231_ADDRESS,
							DS3231_DATE_REGISTER_ADDRESS,
							I2C_MEMADD_SIZE_8BIT,
							DS3231_Date->RxDateBuff,
							sizeof(DS3231_Date->RxDateBuff),
							100);
}

static HAL_StatusTypeDef I2C_WriteAlarm2(DS3231_Alarm2_t *DS3231_Alarm2)
{
	return HAL_I2C_Mem_Write(DS3231_Alarm2->I2C,
					 	 	 SLAVE_DS3231_ADDRESS,
							 DS3231_ALARM2_REGISTER_ADDRESS,
							 I2C_MEMADD_SIZE_8BIT,
							 DS3231_Alarm2->TxAlarm2Buff,
							 sizeof(DS3231_Alarm2->TxAlarm2Buff),
							 100);
}

static HAL_StatusTypeDef I2C_ReadAlarm2(DS3231_Alarm2_t *DS3231_Alarm2)
{
	return HAL_I2C_Mem_Read(DS3231_Alarm2->I2C,
						 	SLAVE_DS3231_ADDRESS,
							DS3231_ALARM2_REGISTER_ADDRESS,
							I2C_MEMADD_SIZE_8BIT,
							DS3231_Alarm2->RxAlarm2Buff,
							sizeof(DS3231_Alarm2->RxAlarm2Buff),
							100);
}


void DS3231_DateTime_Init(I2C_HandleTypeDef* hi2c, DS3231_DateTime_t *DateTime)
{
	DateTime->I2C = hi2c;
}

void DS3231_SetTime(DS3231_DateTime_t *DS3231_Time, uint8_t Hour, uint8_t Minutes, uint8_t Seconds)
{
	DS3231_Time->TxTimeBuff[0] = DS3231_DectoBCD(Seconds);
	DS3231_Time->TxTimeBuff[1] = DS3231_DectoBCD(Minutes);
	DS3231_Time->TxTimeBuff[2] = DS3231_DectoBCD(Hour);

	if (I2C_WriteTime(DS3231_Time) != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_GetTime(DS3231_DateTime_t *DS3231_Time)
{
	if (I2C_ReadTime(DS3231_Time) == HAL_OK)
	{
		DS3231_Time->Seconds = DS3231_BCDtoDec(DS3231_Time->RxTimeBuff[0]);
		DS3231_Time->Minutes = DS3231_BCDtoDec(DS3231_Time->RxTimeBuff[1]);
		DS3231_Time->Hour = DS3231_BCDtoDec(DS3231_Time->RxTimeBuff[2]);
	}
	else
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_SetDate(DS3231_DateTime_t *DS3231_Date, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year)
{
	DS3231_Date->TxDateBuff[0] = DS3231_DectoBCD(Day);
	DS3231_Date->TxDateBuff[1] = DS3231_DectoBCD(Date);
	DS3231_Date->TxDateBuff[2] = DS3231_DectoBCD(Month);
	DS3231_Date->TxDateBuff[3] = DS3231_DectoBCD(Year);

	if (I2C_WriteDate(DS3231_Date) != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error(); 
	}
}

void DS3231_GetDate(DS3231_DateTime_t *DS3231_Date)
{
	if (I2C_ReadDate(DS3231_Date) == HAL_OK)
	{
		DS3231_Date->Day = DS3231_BCDtoDec(DS3231_Date->RxDateBuff[0]);
		DS3231_Date->Date = DS3231_BCDtoDec(DS3231_Date->RxDateBuff[1]);
		DS3231_Date->Month = DS3231_BCDtoDec(DS3231_Date->RxDateBuff[2]);
		DS3231_Date->Year = DS3231_BCDtoDec(DS3231_Date->RxDateBuff[3]);
	}
	else
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_Alarm2_Init(I2C_HandleTypeDef* I2C, DS3231_Alarm2_t *Alarm)
{
	Alarm->I2C = I2C;
	uint8_t ControlRegister;	// Buff contrain Control Register
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(I2C,
					  	  	  	  				SLAVE_DS3231_ADDRESS,
								  				DS3231_CONTROL_REGISTER_ADDRESS,
								  				I2C_MEMADD_SIZE_8BIT,
								  				&ControlRegister,
								  				1,
								  				100);
	if (status != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}

	ControlRegister |= DS3231_CONTROL_BIT_INTCN;


	status = HAL_I2C_Mem_Write(I2C,
					  	  	   SLAVE_DS3231_ADDRESS,
							   DS3231_CONTROL_REGISTER_ADDRESS,
							   I2C_MEMADD_SIZE_8BIT,
							   &ControlRegister,
							   1,
							   100);
	if (status != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_SetAlarm2(DS3231_Alarm2_t *DS3231_Alarm2, uint8_t Hour, uint8_t Minutes)
{
	DS3231_Alarm2->TxAlarm2Buff[0] = DS3231_DectoBCD(Minutes) & 0x7F;
	DS3231_Alarm2->TxAlarm2Buff[1] = DS3231_DectoBCD(Hour) & 0x3F;
	DS3231_Alarm2->TxAlarm2Buff[2] = 0x80;

	if (I2C_WriteAlarm2(DS3231_Alarm2) != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_GetAlarm2(DS3231_Alarm2_t *DS3231_Alarm2)
{
	if (I2C_ReadAlarm2(DS3231_Alarm2) == HAL_OK)
	{
		DS3231_Alarm2->Minutes = DS3231_BCDtoDec(DS3231_Alarm2->RxAlarm2Buff[0]);
		DS3231_Alarm2->Hour = DS3231_BCDtoDec(DS3231_Alarm2->RxAlarm2Buff[1]);
	}
	else
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_ClearnFlagAlarm2(I2C_HandleTypeDef* I2C)
{
	uint8_t StatusRegister;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(I2C,
					  	  	  	  				SLAVE_DS3231_ADDRESS,
								  				DS3231_STATUS_REGISTER_ADDRESS,
								  				I2C_MEMADD_SIZE_8BIT,
								  				&StatusRegister,
								  				1,
								  				100);
	if (status != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}

	StatusRegister &= ~DS3231_FLAG_STATUS_A2F;

	status = HAL_I2C_Mem_Write(I2C,
					  	  	   SLAVE_DS3231_ADDRESS,
							   DS3231_STATUS_REGISTER_ADDRESS,
							   I2C_MEMADD_SIZE_8BIT,
							   &StatusRegister,
							   1,
							   100);
	if (status != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}
}

void DS3231_EnOrDisAlarm2(I2C_HandleTypeDef* I2C, uint8_t EnOrDisAlarm)
{
	uint8_t ControlRegister;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(I2C,
					  	  	  	  				SLAVE_DS3231_ADDRESS,
								  				DS3231_CONTROL_REGISTER_ADDRESS,
								  				I2C_MEMADD_SIZE_8BIT,
								  				&ControlRegister,
								  				1,
								  				100);
	if (status != HAL_OK)
	{
		// I2C ERROR
		Handle_I2C_Error();
	}

	if (EnOrDisAlarm == EnableAlarm2)
	{
		DS3231_ClearnFlagAlarm2(I2C);
		ControlRegister |= DS3231_CONTROL_BIT_A2IE;
	}
	else
	{
		ControlRegister &= ~DS3231_CONTROL_BIT_A2IE;
	}

	status = HAL_I2C_Mem_Write(I2C,
							   SLAVE_DS3231_ADDRESS,
							   DS3231_CONTROL_REGISTER_ADDRESS,
							   I2C_MEMADD_SIZE_8BIT,
							   &ControlRegister,
							   1,
							   100);
	if (status != HAL_OK)
	{
		// I2C ERROR

	}
}

uint8_t DS3231_GetFlagAlarm2(I2C_HandleTypeDef* hi2c)
{
    uint8_t statusRegister;

    if (HAL_I2C_Mem_Read(hi2c,
                         SLAVE_DS3231_ADDRESS,
                         DS3231_STATUS_REGISTER_ADDRESS,
                         I2C_MEMADD_SIZE_8BIT,
                         &statusRegister,
                         1,
                         100) != HAL_OK)
    {
    	Handle_I2C_Error();
    }

    return (statusRegister & DS3231_FLAG_STATUS_A2F) ? DS3231_FLAG_A2F_ON : DS3231_FLAG_A2F_OFF;
}

uint8_t DS3231_CheckModeAlarm2(I2C_HandleTypeDef* I2C)
{
	uint8_t ControlRegister;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(I2C,
					  	  	  	  				SLAVE_DS3231_ADDRESS,
								  				DS3231_CONTROL_REGISTER_ADDRESS,
								  				I2C_MEMADD_SIZE_8BIT,
								  				&ControlRegister,
								  				1,
								  				100);
	if (status != HAL_OK)
    {
    	Handle_I2C_Error();
    }

    return (ControlRegister & DS3231_CONTROL_BIT_A2IE) ? EnableAlarm2 : DisableAlarm2;
}




