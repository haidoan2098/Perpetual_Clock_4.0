################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/LCD/LCD_I2C.c 

OBJS += \
./User/LCD/LCD_I2C.o 

C_DEPS += \
./User/LCD/LCD_I2C.d 


# Each subdirectory must supply rules for building sources it contributes
User/LCD/%.o User/LCD/%.su User/LCD/%.cyclo: ../User/LCD/%.c User/LCD/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../User/LCD -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-User-2f-LCD

clean-User-2f-LCD:
	-$(RM) ./User/LCD/LCD_I2C.cyclo ./User/LCD/LCD_I2C.d ./User/LCD/LCD_I2C.o ./User/LCD/LCD_I2C.su

.PHONY: clean-User-2f-LCD

