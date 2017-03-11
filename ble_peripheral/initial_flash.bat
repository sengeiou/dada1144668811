@echo OFF
SETLOCAL
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION

..\utilities\scripts\suota\v11\initial_flash.bat -v %1\ble_peripheral.bin %2
