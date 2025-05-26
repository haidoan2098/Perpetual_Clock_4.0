/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "LCD_I2C.h"
#include "DS3231.h"
#include "BUZZER.h"
#include "Custom_String.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
	SUN = 1,
	MON,
	TUE,
	WED,
	THU,
	FRI,
	SAT
} Day;

typedef enum
{
	STATE_DISPLAY,
	STATE_SET_HOUR,
	STATE_SET_MINUTE,
	STATE_SET_SECOND,
	STATE_SET_YEAR,
	STATE_SET_MONTH,
	STATE_SET_DATE,
	STATE_SET_DAY,
	STATE_SET_HOUR_ALARM,
	STATE_SET_MINUTE_ALARM,
	STATE_SET_MODE_ALARM
} ClockState;

typedef enum
{
	OFF_ALARM,
	ON_ALARM
} ModeAlarm;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PRESS_BUTTON			GPIO_PIN_RESET
#define RELEASE_BUTTON 			GPIO_PIN_SET

#define BUTTON_INCREASE 		GPIO_PIN_1
#define BUTTON_REDUCE 			GPIO_PIN_2
#define BUTTON_MODE 			GPIO_PIN_3

#define MAX_HOUR 	23
#define MIN_HOUR	0
#define MAX_MINUTE 	59
#define MIN_MINUTE	0
#define MAX_SECOND 	59
#define MIN_SECOND	0
#define MAX_YEAR 	99
#define MIN_YEAR 	0
#define MAX_MONTH	12
#define MIN_MONTH	1
#define MIN_DATE	1

#define TIMEOUT_ALARM 		60000
#define TIME_WAITING 		5000
#define ALARM_DELAY_TIME 	5000
#define BACKLIGH_WAITING 	10000

#define RX_BUFFER_SIZE 25

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

// Create Object
DS3231_DateTime_t DS3231_Time;
DS3231_DateTime_t DS3231_Date;
DS3231_Alarm2_t DS3231_Alarm;

const char *dayOfWeek[] = {"---", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const char *modeAlarm[] = {"OFF", "ON "};

const uint16_t alarmMelody1[] = {
    NOTE_C6, NOTE_C6, NOTE_G5, NOTE_G5,
    NOTE_A5, NOTE_A5, NOTE_G5,
    0,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5,
    NOTE_D5, NOTE_D5, NOTE_C5
};

const uint16_t alarmMelody2[] = {
    NOTE_C6, NOTE_E6, NOTE_G6, NOTE_C7,
    NOTE_B6, NOTE_G6, NOTE_E6, NOTE_C6,
    NOTE_E6, NOTE_G6, NOTE_C7, NOTE_E7
};

// String constrain time and data display
char timeDisplay[17];
char dateDisplay[20];
char alarmDisplay[17];

uint8_t hourBuffer, minuteBuffer, secondBuffer;
Day dayBuffer;
uint8_t dateBuffer, monthBuffer, yearBuffer;
uint8_t hourAlarmBuffer, minuteAlarmBuffer;
uint8_t checkAlarmStatus;

uint8_t flagAlarm;
uint32_t alarmStartTime;

ClockState currentClock = STATE_DISPLAY;

uint8_t flagUpdateTimeOnInternet = 0;
uint8_t rxByte = 0;
char rxBuffer[RX_BUFFER_SIZE] = {0};
uint8_t rxBufferIndex = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

// ------- ALARM SOUND ----------
void playAlarmMelody1(void);
void playAlarmMelody2(void);
void alarmSound(void);

//------- Leap Year Handling --------
uint8_t LeapYearHandling(uint8_t year);
uint8_t daysInMonth(uint8_t month, uint16_t year);

//------- Mode Alarm Handling ----------
void ModeAlarmHandling(uint8_t checkAlarmStatus);

//------- State Handler Functions -------
void DisplayClockHandler(void);
void SetHourHandler(void);
void SetMinuteHandler(void);
void SetSecondHandler(void);
void SetYearHandler(void);
void SetMonthHandler(void);
void SetDateHandler(void);
void SetDayHandler(void);
void SetHourAlarmHandler(void);
void SetMinuteAlarmHandler(void);
void SetModeAlarmHandler(void);

// ------- Handle Clock State Machine --------
void HandleClockStateMachine(void);

// ------- Energy Saving Processing -----------
void EnergySavingProcessing(void);

// ------- Handling when there is an alarm -------
void AlarmProcessing(void);

// -------------------
void parseInternetTimeString(const char* rxBuffer, uint8_t* hour, uint8_t* minute, uint8_t* second, uint8_t* weekDay, uint8_t* date, uint8_t* month, uint8_t* year)
{
	char weekDayStr[4];
    uint8_t idx = 0;
    uint8_t weekdayIndex = 0;

    // Parse hour
    while ('0' <= rxBuffer[idx] && rxBuffer[idx] <= '9')
    {
        *hour = *hour * 10 + (rxBuffer[idx] - '0');
        idx++;
    }

    if (rxBuffer[idx] == ':') idx++;

    // Parse minute
    while ('0' <= rxBuffer[idx] && rxBuffer[idx] <= '9')
    {
        *minute = *minute * 10 + (rxBuffer[idx] - '0');
        idx++;
    }

    if (rxBuffer[idx] == ':') idx++;

    // Parse second
    while ('0' <= rxBuffer[idx] && rxBuffer[idx] <= '9')
    {
        *second = *second * 10 + (rxBuffer[idx] - '0');
        idx++;
    }

    if (rxBuffer[idx] == '-') idx++;

    // Parse weekday
    while (rxBuffer[idx] != '-' && rxBuffer[idx] != '\0' && weekdayIndex < 3)
    {
    	weekDayStr[weekdayIndex] = rxBuffer[idx];
        idx++;
        weekdayIndex++;
    }
    weekDayStr[weekdayIndex] = '\0';

    if (rxBuffer[idx] == '-') idx++;

    // Parse day
    while ('0' <= rxBuffer[idx] && rxBuffer[idx] <= '9')
    {
        *date = *date * 10 + (rxBuffer[idx] - '0');
        idx++;
    }

    if (rxBuffer[idx] == '/') idx++;

    // Parse month
    while ('0' <= rxBuffer[idx] && rxBuffer[idx] <= '9')
    {
        *month = *month * 10 + (rxBuffer[idx] - '0');
        idx++;
    }

    if (rxBuffer[idx] == '/') idx++;

    // Parse year
    while ('0' <= rxBuffer[idx] && rxBuffer[idx] <= '9')
    {
        *year = *year * 10 + (rxBuffer[idx] - '0');
        idx++;
    }

    if (strCompare(weekDayStr, "SUN")) *weekDay = SUN;
    else if (strCompare(weekDayStr, "MON")) *weekDay = MON;
    else if (strCompare(weekDayStr, "TUE")) *weekDay = TUE;
    else if (strCompare(weekDayStr, "WED")) *weekDay = WED;
    else if (strCompare(weekDayStr, "THU")) *weekDay = THU;
    else if (strCompare(weekDayStr, "FRI")) *weekDay = FRI;
    else if (strCompare(weekDayStr, "SAT")) *weekDay = SAT;
}

void TimeInternetUpdateHandling(void)
{
	while (flagUpdateTimeOnInternet)
	{
		LCD_backlight_on();

	    uint8_t netHour = 0, netMinute = 0, netSecond = 0;
	    uint8_t netWeekday = 0, netDate = 0, netMonth = 0, netYear = 0;

	    parseInternetTimeString(rxBuffer, &netHour, &netMinute, &netSecond, &netWeekday, &netDate, &netMonth, &netYear);

        DS3231_SetTime(&DS3231_Time, netHour, netMinute, netSecond);			// Chốt Giờ-phút-giây
        DS3231_SetDate(&DS3231_Date, netWeekday, netDate, netMonth, netYear);	// Chốt ngày-ngày-tháng-năm

	    LCD_goto_XY(1, 2);
	    LCD_send_string("TIME UPDATED");
	    LCD_goto_XY(2, 2);
	    LCD_send_string("SUCCESSFULLY");

	    HAL_Delay(2000);

	    LCD_clear_display();
	    HAL_Delay(40);

	    flagUpdateTimeOnInternet = 0;

	    break;
	}
}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Perform an alarm if an interrupt signal occurs
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	flagAlarm = 1;

	LCD_clear_display();
	alarmStartTime = HAL_GetTick();

    DS3231_ClearnFlagAlarm2(&hi2c1);	// CLear Flag Alarm DS3231
}

// Nhận UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (rxByte != '\r' && rxByte != '\n') // Loại bỏ ký tự '\r' và '\n'
    {
        rxBuffer[rxBufferIndex] = rxByte;
        rxBufferIndex++;

        if (rxBufferIndex == RX_BUFFER_SIZE)
        {
            rxBufferIndex = 0;
        }
    }
    else if (rxByte == '\r')
    {
        rxBuffer[rxBufferIndex] = '\0'; // Kết thúc chuỗi khi nhận '\r'

        rxBufferIndex = 0; // Reset chỉ số bộ đệm

        flagUpdateTimeOnInternet = 1; // nhận chuỗi đủ rồi bật cờ update lên
        LCD_clear_display(); // Xóa màn hình
    }

    // Tiếp tục nhận byte tiếp theo
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(2000); // Cho LCD hoặc DS3231 ổn định nguồn

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  HAL_UART_Receive_IT(&huart1, &rxByte, 1);

  LCD_Init();
  LCD_backlight_on();
  HAL_Delay(500);

  DS3231_DateTime_Init(&hi2c1, &DS3231_Time);
  DS3231_DateTime_Init(&hi2c1, &DS3231_Date);
  DS3231_Alarm2_Init(&hi2c1, &DS3231_Alarm);
  HAL_Delay(500);
  // Get current time in DS3231 for setting
  DS3231_GetTime(&DS3231_Time);
  hourBuffer = DS3231_Time.Hour;
  minuteBuffer = DS3231_Time.Minutes;
  secondBuffer = DS3231_Time.Seconds;

  // Get current date in DS3231 for setting
  DS3231_GetDate(&DS3231_Date);
  dayBuffer = DS3231_Date.Day;
  dateBuffer = DS3231_Date.Date;
  monthBuffer = DS3231_Date.Month;
  yearBuffer = DS3231_Date.Year;

  // Get current alarm in DS3231 for setting
  DS3231_GetAlarm2(&DS3231_Alarm);
  hourAlarmBuffer = DS3231_Alarm.Hour;
  minuteAlarmBuffer = DS3231_Alarm.Minutes;

  // Get current mode alarm in DS3231 for setting
  checkAlarmStatus = DS3231_CheckModeAlarm2(&hi2c1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  HandleClockStateMachine();

	  HAL_Delay(100);

	  EnergySavingProcessing();

	  AlarmProcessing();

	  TimeInternetUpdateHandling();


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL10;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 700;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief Alarm tone storage
  */
void playAlarmMelody1(void) {
  for (uint8_t i = 0; i < sizeof(alarmMelody1) / sizeof(alarmMelody1[0]); i++)
  {
    playTone(alarmMelody1[i], &htim2);
    HAL_Delay(100);
  }
  noTone(&htim2);
}
void playAlarmMelody2(void)
{
	for(uint8_t i = 0; i < sizeof(alarmMelody2) / sizeof(alarmMelody2[0]); i++)
	{
		playTone(alarmMelody2[i], &htim2);
		HAL_Delay(150);
	}
	noTone(&htim2);
}
void alarmSound(void)
{
	  playAlarmMelody1();
	  HAL_Delay(100);
	  playAlarmMelody1();
	  HAL_Delay(100);
	  playAlarmMelody2();
}

/**
  * @brief Determines the number of days in a given month and year.
  *        - Returns 31 for months with 31 days (January, March, May, July, August, October, December).
  *        - Returns 30 for months with 30 days (April, June, September, November).
  *        - Returns 28 or 29 for February depending on whether the year is a leap year.
  * @param month: The month (1-12).
  * @param year: The year (e.g., 2023).
  * @retval The number of days in the specified month.
  */
uint8_t LeapYearHandling(uint8_t year)
{
	return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}
uint8_t daysInMonth(uint8_t month, uint16_t year)
{
    switch (month)
    {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return LeapYearHandling(year) ? 29 : 28;
        default:
            return 0;
    }
}

/**
  * @brief Enable or disable the alarm mode of the DS3231 clock.
  * @param checkAlarmStatus: Alarm control status.
  * 		- ON_ALARM: Turn on Alarm2.
  * 		- OFF_ALARM: Turn off Alarm2.
  */
void ModeAlarmHandling(uint8_t checkAlarmStatus)
{
	if (checkAlarmStatus == ON_ALARM)
	{
		DS3231_EnOrDisAlarm2(&hi2c1, EnableAlarm2);
	}
	else
	{
		DS3231_EnOrDisAlarm2(&hi2c1, DisableAlarm2);
	}
}

/**
  * @brief Display Hour - Minute - Second & Day - Date - Month - Year in the clock state machine.
  * 		- Get data form DS3231
  */
void DisplayClockHandler(void)
{
	DS3231_GetTime(&DS3231_Time);
	sprintf(timeDisplay, "%02d:%02d:%02d", DS3231_Time.Hour, DS3231_Time.Minutes, DS3231_Time.Seconds);
	LCD_goto_XY(1, 4);
	LCD_send_string(timeDisplay);

	DS3231_GetDate(&DS3231_Date);
	sprintf(dateDisplay, "%s %02d/%02d/20%02d", dayOfWeek[DS3231_Date.Day], DS3231_Date.Date, DS3231_Date.Month, DS3231_Date.Year);
	LCD_goto_XY(2, 1);
	LCD_send_string(dateDisplay);
}

/**
  * @brief Handles the Hour adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Hour value will increase by 1
  * 		- Press BUTTON_REDUCE the Hour value will reduce by 1
  * 		- The Hour value 0 -> 23
  */
void SetHourHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

    LCD_goto_XY(1, 5);
    LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        hourBuffer++;
        if (hourBuffer > MAX_HOUR)
        {
            hourBuffer = MIN_HOUR;
        }
        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);

        if (hourBuffer == MIN_HOUR)
        {
        	hourBuffer = MAX_HOUR;
        }
        else
        {
        	hourBuffer--;
        }

        LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Minute adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Minute value will increase by 1
  * 		- Press BUTTON_REDUCE the Minute value will reduce by 1
  * 		- The Minute value 0 -> 59
  */
void SetMinuteHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

    LCD_goto_XY(1, 8);
    LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        minuteBuffer++;
        if (minuteBuffer > MAX_MINUTE)
        {
        	minuteBuffer = MIN_MINUTE;
        }
        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
    	if (minuteBuffer == MIN_MINUTE)
    	{
    		minuteBuffer = MAX_MINUTE;
    	}
    	else
    	{
    		minuteBuffer--;
    	}

    	LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Second adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Second value will increase by 1
  * 		- Press BUTTON_REDUCE the Second value will reduce by 1
  * 		- The Second value 0 -> 59
  */
void SetSecondHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

    LCD_goto_XY(1, 11);
    LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        secondBuffer++;
        if (secondBuffer > MAX_SECOND)
        {
        	secondBuffer = MIN_SECOND;
        }
        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
    	if (secondBuffer == MIN_SECOND)
    	{
    		secondBuffer = MAX_SECOND;
    	}
    	else
    	{
    		secondBuffer--;
    	}

    	LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Year adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Year value will increase by 1
  * 		- Press BUTTON_REDUCE the Year value will reduce by 1
  * 		- The Year value 0 -> 99 (+ 2000)
  */
void SetYearHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

	LCD_goto_XY(2, 4);
	LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        yearBuffer++;
        if (yearBuffer > MAX_YEAR)
        {
        	yearBuffer = MIN_YEAR;
        }
        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
    	if (yearBuffer == MIN_YEAR)
    	{
    		yearBuffer = MAX_YEAR;
    	}
    	else
    	{
    		yearBuffer--;
    	}

    	LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Month adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Month value will increase by 1
  * 		- Press BUTTON_REDUCE the Month value will reduce by 1
  * 		- The Month value 1 -> 12
  */
void SetMonthHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

	LCD_goto_XY(2, 7);
	LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        monthBuffer++;
        if (monthBuffer > MAX_MONTH)
        {
        	monthBuffer = MIN_MONTH;
        }

        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
    	if (monthBuffer == MIN_MONTH)
    	{
    		monthBuffer = MAX_MONTH;
    	}
    	else
    	{
    		monthBuffer--;
    	}
    	LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Date adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Date value will increase by 1
  * 		- Press BUTTON_REDUCE the Date value will reduce by 1
  * 		- The Date value 1 -> 31 or 30 or 28 or 29
  */
void SetDateHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

	LCD_goto_XY(2, 10);
	LCD_blink_cursor_on();

	uint8_t MAX_DATE = daysInMonth(monthBuffer, 2000 + yearBuffer);

	if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
	{
		HAL_Delay(40);
		while (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

		dateBuffer++;
		if (dateBuffer > MAX_DATE)
		{
			dateBuffer = MIN_DATE;
		}

		LCD_blink_cursor_off();
	}

	if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
	{
		HAL_Delay(40);
		while (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);

		if (dateBuffer == MIN_DATE)
		{
			dateBuffer = MAX_DATE;
		}
		else
		{
			dateBuffer--;
		}

		LCD_blink_cursor_off();
	}
}

/**
  * @brief Handles the Day adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Day value will increase by 1
  * 		- Press BUTTON_REDUCE the Day value will reduce by 1
  * 		- The Day value in Day (enum)
  */
void SetDayHandler(void)
{
    sprintf(timeDisplay, "%02d:%02d:%02d", hourBuffer, minuteBuffer, secondBuffer);
    LCD_goto_XY(1, 4);
    LCD_send_string(timeDisplay);

	LCD_goto_XY(2, 1);
	sprintf(dateDisplay, "20%02d/%02d/%02d %s", yearBuffer, monthBuffer, dateBuffer, dayOfWeek[dayBuffer]);
	LCD_send_string(dateDisplay);

	LCD_goto_XY(2, 14);
	LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        dayBuffer++;
        if (dayBuffer > SAT)
        {
        	dayBuffer = SUN;
        }

        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
    	if (dayBuffer == SUN)
    	{
    		dayBuffer = SAT;
    	}
    	else
    	{
    		dayBuffer--;
    	}
    	LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Alarm Hour adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Alarm Hour value will increase by 1
  * 		- Press BUTTON_REDUCE the Alarm Hour value will reduce by 1
  * 		- The Hour value 0 -> 23
  */
void SetHourAlarmHandler(void)
{
	LCD_goto_XY(1, 3);
	LCD_send_string("ALARM MODE");
	LCD_goto_XY(2, 3);
	sprintf(alarmDisplay, "%02d:%02d  %s", hourAlarmBuffer, minuteAlarmBuffer, modeAlarm[checkAlarmStatus]);
	LCD_send_string(alarmDisplay);

    LCD_goto_XY(2, 4);
    LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

        hourAlarmBuffer++;
        if (hourAlarmBuffer > MAX_HOUR)
        {
        	hourAlarmBuffer = MIN_HOUR;
        }
        LCD_blink_cursor_off();
    }

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
    {
        HAL_Delay(40);
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);

        if (hourAlarmBuffer == MIN_HOUR)
        {
        	hourAlarmBuffer = MAX_HOUR;
        }
        else
        {
        	hourAlarmBuffer--;
        }

        LCD_blink_cursor_off();
    }
}

/**
  * @brief Handles the Alarm Minute adjustment in the clock state machine.
  * 		- Press BUTTON_INCREASE the Minute value will increase by 1
  * 		- Press BUTTON_REDUCE the Minute value will reduce by 1
  * 		- The Alarm Minute value 0 -> 59
  */
void SetMinuteAlarmHandler(void)
{
	LCD_goto_XY(1, 3);
	LCD_send_string("ALARM MODE");
	LCD_goto_XY(2, 3);
	sprintf(alarmDisplay, "%02d:%02d  %s", hourAlarmBuffer, minuteAlarmBuffer, modeAlarm[checkAlarmStatus]);
	LCD_send_string(alarmDisplay);

    LCD_goto_XY(2, 7);
    LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
    	 HAL_Delay(40);
         while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

         minuteAlarmBuffer++;
         if (minuteAlarmBuffer > MAX_MINUTE)
         {
        	 minuteAlarmBuffer = MIN_MINUTE;
         }
         LCD_blink_cursor_off();
     }

     if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
     {
         HAL_Delay(40);
         while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
     	if (minuteAlarmBuffer == MIN_MINUTE)
     	{
     		minuteAlarmBuffer = MAX_MINUTE;
     	}
     	else
     	{
     		minuteAlarmBuffer--;
     	}

     	LCD_blink_cursor_off();
     }
}

/**
  * @brief Handles Alarm Mode adjustment in the clock state machine.
  * 		- Have 2 modes: ON_ALARM or OFF_ALARM
  * 		- Press BUTTON_INCREASE or BUTTON_REDUCE to change the mode
  */
void SetModeAlarmHandler(void)
{
	LCD_goto_XY(1, 3);
	LCD_send_string("ALARM MODE");
	LCD_goto_XY(2, 3);
	sprintf(alarmDisplay, "%02d:%02d  %s", hourAlarmBuffer, minuteAlarmBuffer, modeAlarm[checkAlarmStatus]);
	LCD_send_string(alarmDisplay);

    LCD_goto_XY(2, 10);
    LCD_blink_cursor_on();

    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON)
    {
    	 HAL_Delay(40);
         while(HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON);

         checkAlarmStatus++;
         if (checkAlarmStatus > ON_ALARM)
         {
        	 checkAlarmStatus = OFF_ALARM;
         }
         ModeAlarmHandling(checkAlarmStatus);
         LCD_blink_cursor_off();
     }

     if (HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON)
     {
         HAL_Delay(40);
         while(HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON);
         if (checkAlarmStatus == OFF_ALARM)
         {
        	 checkAlarmStatus = ON_ALARM;
         }
         else
         {
        	 checkAlarmStatus--;
         }
         ModeAlarmHandling(checkAlarmStatus);
         LCD_blink_cursor_off();
      }
}


/**
 * @brief  Manages the clock setting state machine.
 * 			- Press BUTTON_MODE to changes status clock
 *
 * @note   This function should be called periodically in the main loop or a task.
 *
 */
void HandleClockStateMachine(void)
{
	switch (currentClock) {
		case STATE_DISPLAY:
			uint32_t currentDisplayMillis = HAL_GetTick();
			static uint32_t previousDisplayMillis = 0;

			if (currentDisplayMillis - previousDisplayMillis >= 1000)
			{
				previousDisplayMillis = currentDisplayMillis;
				DisplayClockHandler();
			}


		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        currentClock = STATE_SET_HOUR;

		    	// Trước khi cài đặt thì lấy thời gian hiện tại chứ ko phải là thời gian lần đầu lấy ở init();
		    	hourBuffer = DS3231_Time.Hour;
		    	minuteBuffer = DS3231_Time.Minutes;
		    	secondBuffer = DS3231_Time.Seconds;
		        dayBuffer = DS3231_Date.Day;
		        dateBuffer = DS3231_Date.Date;
		        monthBuffer = DS3231_Date.Month;
		        yearBuffer = DS3231_Date.Year;
		    }

			break;

		case STATE_SET_HOUR:
			SetHourHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        currentClock = STATE_SET_MINUTE;
		    }
			break;

		case STATE_SET_MINUTE:
			SetMinuteHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        currentClock = STATE_SET_SECOND;
		    }
			break;

		case STATE_SET_SECOND:
			SetSecondHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        DS3231_SetTime(&DS3231_Time, hourBuffer, minuteBuffer, secondBuffer);	//Chốt Giờ-phút-giây
		        currentClock = STATE_SET_YEAR;
		        // Trước khi set date thì lấy thời gian hiện tại đang chạy cái đã
		        dayBuffer = DS3231_Date.Day;
		        dateBuffer = DS3231_Date.Date;
		        monthBuffer = DS3231_Date.Month;
		        yearBuffer = DS3231_Date.Year;
		    }
			break;

		case STATE_SET_YEAR:
			SetYearHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        currentClock = STATE_SET_MONTH;
		    }
			break;

		case STATE_SET_MONTH:
			SetMonthHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        currentClock = STATE_SET_DATE;
		    }
			break;

		case STATE_SET_DATE:
			SetDateHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        currentClock = STATE_SET_DAY;
		    }
			break;

		case STATE_SET_DAY:
			SetDayHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        DS3231_SetDate(&DS3231_Date, dayBuffer, dateBuffer, monthBuffer, yearBuffer);	// Chốt thứ - ngày - tháng - năm
		        LCD_blink_cursor_off();		// Tắt con trỏ nhấp nháy
		        currentClock = STATE_SET_HOUR_ALARM;
		        LCD_clear_display(); // Xóa trước khi qua set alarm
		    }
			break;

		case STATE_SET_HOUR_ALARM:
			SetHourAlarmHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        LCD_blink_cursor_off();		// Tắt con trỏ nhấp nháy
		        currentClock = STATE_SET_MINUTE_ALARM;
		    }
			break;

		case STATE_SET_MINUTE_ALARM:
			SetMinuteAlarmHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);
		        LCD_blink_cursor_off();		// Tắt con trỏ nhấp nháy
		        currentClock = STATE_SET_MODE_ALARM;
		    }
			break;

		case STATE_SET_MODE_ALARM:
			SetModeAlarmHandler();
		    if (HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON)
		    {
		        HAL_Delay(40);
		        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_MODE) == PRESS_BUTTON);

		        DS3231_SetAlarm2(&DS3231_Alarm, hourAlarmBuffer, minuteAlarmBuffer);
		        LCD_blink_cursor_off();		// Tắt con trỏ nhấp nháy
		        currentClock = STATE_DISPLAY;
		        LCD_clear_display(); // Xóa trước khi qua hiển thị từ đầu
		    }
			break;
	}
}


/**
 * @brief  LCD backlight management.
 * 			- The backlight will turn off when Time 0->5 hour
 * 			- Press BUTTON_INCREASE, BUTTON_REDUCE the backlight will turn on for 10 seconds
 *
 * @note   This function should be called periodically in the main loop or a task.
 *
 */
void EnergySavingProcessing(void)
{
	static uint32_t currentBackLightMillis = 0;
	if ((currentClock == STATE_DISPLAY ) && (0 <= DS3231_Time.Hour && DS3231_Time.Hour < 5) && (HAL_GetTick() - currentBackLightMillis >= BACKLIGH_WAITING))
	{
		LCD_backlight_off();
		if ((HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON || HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON))
		{
			HAL_Delay(40);
			while ((HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON || HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON));

			LCD_backlight_on();

			currentBackLightMillis = HAL_GetTick();
		}
	}
	else
	{
		LCD_backlight_on();
	}
}


/**
 * @brief  Handling when there is an alarm.
 * 			- Press BUTTON_INCREASE, BUTTON_REDUCE to exit alarm mode
 *
 * @note   This function should be called periodically in the main loop or a task.
 *
 */
void AlarmProcessing(void)
{
	while (flagAlarm)
	{
		LCD_backlight_on();

		uint32_t currentAlarmMillis = HAL_GetTick();
		static uint32_t previousAlarmMillis = 0;

		LCD_goto_XY(1, 4);
		LCD_send_string("WAKE UP!");
		LCD_goto_XY(2, 1);
		LCD_send_string("RISE AND SHINE");

		if (currentAlarmMillis - previousAlarmMillis >= (TIME_WAITING + ALARM_DELAY_TIME))
		{
			alarmSound();
			previousAlarmMillis = currentAlarmMillis;
		}
		else
		{
			if ((HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON || HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON))
			{
				HAL_Delay(40);
				while ((HAL_GPIO_ReadPin(GPIOA, BUTTON_INCREASE) == PRESS_BUTTON || HAL_GPIO_ReadPin(GPIOA, BUTTON_REDUCE) == PRESS_BUTTON));

				flagAlarm = 0;	// Clear Flag Alarm
				LCD_clear_display();
				HAL_Delay(40);

				break; // Break out of the loop
			}
		}

		if (HAL_GetTick() - alarmStartTime >= TIMEOUT_ALARM)
		{
			flagAlarm = 0;
			LCD_clear_display();
			HAL_Delay(40);

			break;
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
