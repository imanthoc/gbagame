#!/bin/bash

make clean && make && LIBGL_ALWAYS_SOFTWARE=1 mgba-qt -g my_demo.gba &

#arm-none-eabi-gdb my_demo.elf
