################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include sdk/peripherals/src/subdir.mk
-include sdk/osal/subdir.mk
-include sdk/memory/src/subdir.mk
-include sdk/cpm/subdir.mk
-include sdk/ble_services/src/subdir.mk
-include sdk/ble/src/util/subdir.mk
-include sdk/ble/src/stack/plf/black_orca/src/driver/rf/src/subdir.mk
-include sdk/ble/src/stack/plf/black_orca/src/arch/main/ble/subdir.mk
-include sdk/ble/src/stack/modules/rwip/src/subdir.mk
-include sdk/ble/src/stack/modules/nvds/src/subdir.mk
-include sdk/ble/src/stack/ip/ble/profiles/subdir.mk
-include sdk/ble/src/stack/ip/ble/ll/src/rwble/subdir.mk
-include sdk/ble/src/manager/subdir.mk
-include sdk/ble/src/adapter/subdir.mk
-include sdk/ble/src/subdir.mk
-include sdk/adapters/src/subdir.mk
-include sdk/FreeRTOS/portable/MemMang/subdir.mk
-include sdk/FreeRTOS/portable/GCC/ARM_CM0/subdir.mk
-include sdk/FreeRTOS/subdir.mk
-include qfplib/subdir.mk
-include invn/common/subdir.mk
-include driver/subdir.mk
-include dmp3a/subdir.mk
-include common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
pxp_reporter.bin \

SECONDARY_SIZE += \
pxp_reporter.siz \


# All Target
all: pre-build main-build

# Main-build Target
main-build: pxp_reporter.elf secondary-outputs

# Tool invocations
pxp_reporter.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -T "E:\wang\dialogp\hy_ble\pxp_reporter/DA14681-01-Debug_QSPI/mem.ld" -T "E:\wang\dialogp\hy_ble\pxp_reporter/DA14681-01-Debug_QSPI/sections.ld" -Xlinker --gc-sections -L"E:\wang\dialogp\hy_ble\sdk\bsp\misc" -L../../sdk/interfaces/ble_stack/DA14681-01-Debug -Wl,-Map,"pxp_reporter.map" --specs=nano.specs --specs=nosys.specs -o "pxp_reporter.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

pxp_reporter.bin: pxp_reporter.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O binary "pxp_reporter.elf"  "pxp_reporter.bin"
	@echo 'Finished building: $@'
	@echo ' '

pxp_reporter.siz: pxp_reporter.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "pxp_reporter.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) pxp_reporter.elf
	-@echo ' '

pre-build:
	-@echo 'Generate linker scripts.'
	-make generate_ldscripts IC_REV=A IC_STEP=E APP_CONFIG_H="E:\wang\dialogp\hy_ble\pxp_reporter\config\custom_config_qspi.h" CC="arm-none-eabi-gcc" BSP_CONFIG_DIR="E:\wang\dialogp\hy_ble\sdk\bsp\config"
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets
