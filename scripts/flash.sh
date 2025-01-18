#!/usr/bin/env bash

/opt/st/stm32cube_programmer/bin/STM32_Programmer.sh --connect port=SWD --write "$1" --verify -rst -run
