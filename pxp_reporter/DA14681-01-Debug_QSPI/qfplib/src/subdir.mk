################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../qfplib/src/qfpio.S \
../qfplib/src/qfplib.S 

OBJS += \
./qfplib/src/qfpio.o \
./qfplib/src/qfplib.o 

S_UPPER_DEPS += \
./qfplib/src/qfpio.d \
./qfplib/src/qfplib.d 


# Each subdirectory must supply rules for building sources it contributes
qfplib/src/%.o: ../qfplib/src/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -x assembler-with-cpp -Ddg_configBLACK_ORCA_IC_REV=BLACK_ORCA_IC_REV_A -Ddg_configBLACK_ORCA_IC_STEP=BLACK_ORCA_IC_STEP_E -I"../../../../../sdk/interfaces/ble_stack/" -I"E:\wang\dialogp\DA1468x_SDK_BTLE_v_1.0.8.1050\sdk\bsp\config" -I"E:\wang\dialogp\DA1468x_SDK_BTLE_v_1.0.8.1050\projects\dk_apps\demos\pxp_reporter\qfplib\src" -include"E:\wang\dialogp\DA1468x_SDK_BTLE_v_1.0.8.1050\projects\dk_apps\demos\pxp_reporter\config\custom_config_qspi.h" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


