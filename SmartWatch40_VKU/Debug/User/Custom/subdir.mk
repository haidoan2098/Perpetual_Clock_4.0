################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Custom/Custom_String.c 

OBJS += \
./User/Custom/Custom_String.o 

C_DEPS += \
./User/Custom/Custom_String.d 


# Each subdirectory must supply rules for building sources it contributes
User/Custom/%.o User/Custom/%.su User/Custom/%.cyclo: ../User/Custom/%.c User/Custom/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"/home/haidoan2098/Workspace/STM32CubeIDE/SmartWatch_Project_VKU/SmartWatch40_VKU/User/PassiveBuzzer" -I"/home/haidoan2098/Workspace/STM32CubeIDE/SmartWatch_Project_VKU/SmartWatch40_VKU/User/DS3231_RTC" -I"/home/haidoan2098/Workspace/STM32CubeIDE/SmartWatch_Project_VKU/SmartWatch40_VKU/User/LCD_I2C" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/home/haidoan2098/Workspace/STM32CubeIDE/SmartWatch_Project_VKU/SmartWatch40_VKU/User/Custom" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-User-2f-Custom

clean-User-2f-Custom:
	-$(RM) ./User/Custom/Custom_String.cyclo ./User/Custom/Custom_String.d ./User/Custom/Custom_String.o ./User/Custom/Custom_String.su

.PHONY: clean-User-2f-Custom

