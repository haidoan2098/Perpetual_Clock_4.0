/*
 * LCD_I2C.c
 *
 *  Created on: Apr 8, 2025
 *      Author: haidoan2098
 */

#include "LCD_I2C.h"

extern I2C_HandleTypeDef hi2c2; // change

#define SALVE_ADDRESS_LCD (0x27 << 1)

// configure BLA E RW RS
#define CMD_E_HIGH_COF 	0x0C	// BLA = 1, E = 1, RW = 0, RS = 0
#define CMD_E_LOW_COF 	0x08	// BLA = 1, E = 0, RW = 0, RS = 0
#define DATA_E_HIGH_COF 0x0D   	// BLA = 1, E = 1, RW = 0, RS = 1
#define DATA_E_LOW_COF 	0x09	// BLA = 1, E = 0, RW = 0, RS = 1

// Cấu hình với đèn nền tắt (BLA = 0)
#define CMD_E_HIGH_COF_NO_BL  0x04  // BLA = 0, E = 1, RW = 0, RS = 0
#define CMD_E_LOW_COF_NO_BL   0x00  // BLA = 0, E = 0, RW = 0, RS = 0
#define DATA_E_HIGH_COF_NO_BL 0x05  // BLA = 0, E = 1, RW = 0, RS = 1
#define DATA_E_LOW_COF_NO_BL  0x01  // BLA = 0, E = 0, RW = 0, RS = 1

static uint8_t backlight_state = 1; // 1: đèn nền bật, 0: đèn nền tắt

void LCD_send_cmd(char cmd)
{
    uint8_t cmd_h = (cmd & 0xF0);        // Lấy 4 bit cao
    uint8_t cmd_l = ((cmd << 4) & 0xF0); // Lấy 4 bit thấp

    uint8_t cmd_transmitted[4];
    if (backlight_state) {
        cmd_transmitted[0] = cmd_h | CMD_E_HIGH_COF;
        cmd_transmitted[1] = cmd_h | CMD_E_LOW_COF;
        cmd_transmitted[2] = cmd_l | CMD_E_HIGH_COF;
        cmd_transmitted[3] = cmd_l | CMD_E_LOW_COF;
    } else {
        cmd_transmitted[0] = cmd_h | CMD_E_HIGH_COF_NO_BL;
        cmd_transmitted[1] = cmd_h | CMD_E_LOW_COF_NO_BL;
        cmd_transmitted[2] = cmd_l | CMD_E_HIGH_COF_NO_BL;
        cmd_transmitted[3] = cmd_l | CMD_E_LOW_COF_NO_BL;
    }

    HAL_I2C_Master_Transmit(&hi2c2, SALVE_ADDRESS_LCD, cmd_transmitted, sizeof(cmd_transmitted), 100);
}

void LCD_send_data(char data)
{
    uint8_t data_h = (data & 0xF0);        // Lấy 4 bit cao
    uint8_t data_l = ((data << 4) & 0xF0); // Lấy 4 bit thấp

    uint8_t data_transmitted[4];
    if (backlight_state) {
        data_transmitted[0] = data_h | DATA_E_HIGH_COF;
        data_transmitted[1] = data_h | DATA_E_LOW_COF;
        data_transmitted[2] = data_l | DATA_E_HIGH_COF;
        data_transmitted[3] = data_l | DATA_E_LOW_COF;
    } else {
        data_transmitted[0] = data_h | DATA_E_HIGH_COF_NO_BL;
        data_transmitted[1] = data_h | DATA_E_LOW_COF_NO_BL;
        data_transmitted[2] = data_l | DATA_E_HIGH_COF_NO_BL;
        data_transmitted[3] = data_l | DATA_E_LOW_COF_NO_BL;
    }

    HAL_I2C_Master_Transmit(&hi2c2, SALVE_ADDRESS_LCD, data_transmitted, sizeof(data_transmitted), 100);
}

void LCD_backlight_on(void)
{
    backlight_state = 1;
    LCD_send_cmd(0x00); // Gửi lệnh dummy để cập nhật trạng thái đèn nền
}

void LCD_backlight_off(void)
{
    backlight_state = 0;
    LCD_send_cmd(0x00); // Gửi lệnh dummy để cập nhật trạng thái đèn nền
}

void LCD_Init(void)
{
	LCD_send_cmd (0x33); /* set 4-bits interface */
	LCD_send_cmd (0x32);
	HAL_Delay(50);
	LCD_send_cmd (0x28); /* start to set LCD function */
	HAL_Delay(50);
	LCD_send_cmd (0x01); /* clear display */
	HAL_Delay(50);
	LCD_send_cmd (0x06); /* set entry mode */
	HAL_Delay(50);
	LCD_send_cmd (0x0C); /* set display to on */
	HAL_Delay(50);
	LCD_send_cmd (0x02); /* move cursor to home and set data address to 0 */
	HAL_Delay(50);
	LCD_send_cmd (0x80);
}

void LCD_send_string(char *str)
{
	while(*str != '\0')
	{
		LCD_send_data(*(str++));
	}
}

void LCD_clear_display(void)
{
	LCD_send_cmd(0x01);
}

void LCD_goto_XY (uint8_t row, uint8_t col)
{
	uint8_t pos_Addr;
	if(row == 1)
	{
		pos_Addr = 0x80 + row - 1 + col;
	}
	else
	{
		pos_Addr = 0x80 | (0x40 + col);
	}
	LCD_send_cmd(pos_Addr);
}

void LCD_blink_cursor_on(void)
{
	LCD_send_cmd(0x0F);
}

void LCD_blink_cursor_off(void)
{
	LCD_send_cmd(0x0C);
}


