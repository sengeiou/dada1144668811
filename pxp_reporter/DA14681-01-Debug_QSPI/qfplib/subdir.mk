################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../qfplib/qfpio.S \
../qfplib/qfplib.S 

OBJS += \
./qfplib/qfpio.o \
./qfplib/qfplib.o 

S_UPPER_DEPS += \
./qfplib/qfpio.d \
./qfplib/qfplib.d 


# Each subdirectory must supply rules for building sources it contributes
qfplib/%.o: ../qfplib/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -x assembler-with-cpp -Ddg_configBLACK_ORCA_IC_REV=BLACK_ORCA_IC_REV_A -Ddg_configBLACK_ORCA_IC_STEP=BLACK_ORCA_IC_STEP_E -I"E:\wang\dialogp\hy_ble\sdk\bsp\config" -I../../sdk/interfaces/ble_stack/ -I"E:\wang\dialogp\hy_ble\pxp_reporter\qfplib" -include"E:\wang\dialogp\hy_ble\pxp_reporter\config\custom_config_qspi.h" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


