/*
 * LCD_I2C.h
 *
 *  Created on: Apr 8, 2025
 *      Author: haidoan2098
 */

#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include "stm32f1xx.h"

// Truyền lệnh
void LCD_send_cmd(char cmd);

// Truyền dữ liệu == 8 bit
void LCD_send_data(char data);

void LCD_backlight_on(void);
void LCD_backlight_off(void);

// Khởi tạo LCD
void LCD_Init(void);

void LCD_send_string(char *str);

void LCD_clear_display(void);

void LCD_goto_XY(uint8_t row, uint8_t col);

void LCD_blink_cursor_on(void);

void LCD_blink_cursor_off(void);

#endif /* INC_LCD_I2C_H_ */
