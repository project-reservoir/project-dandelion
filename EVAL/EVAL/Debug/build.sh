#!/bin/bash

sed 's/SHELL := cmd.exe/SHELL := bash/g' Makefile | sed 's/C:\\Program Files (x86)\\Atmel\\Atmel Toolchain\\ARM GCC\\Native\\4.8.1443\\arm-gnu-toolchain\\bin\\//g' | sed 's/$(QUOTE)//g' | sed 's/"arm-none-eabi-objcopy.exe"/arm-none-eabi-objcopy/g' | sed 's/"arm-none-eabi-objdump.exe"/arm-none-eabi-objdump/g' | sed 's/"arm-none-eabi-size.exe"/arm-none-eabi-size/g' | sed 's/arm-none-eabi-gcc.exe/arm-none-eabi-gcc/g' | sed 's/echo Invoking: ARM\/GNU C Compiler : 4.8.4/mkdir -p $(@D)/g' > Makefile

make all
