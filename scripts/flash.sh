#!/usr/bin/env bash

INDEX="0"

remaining_args=()

#Loop through arguments
while [[ $# -gt 0 ]]
do
  case "$1" in
    --index=*)
      INDEX="${1#*=}"  # Extract value after '='
      shift
      ;;
    *)
      remaining_args+=("$1")  # Store unknown arguments
      shift                   # Remove current argument
      ;;
  esac
done

/home/james/st/stm32cubeide_2.2.0/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_2.2.500.202603051304/tools/bin/STM32_Programmer_CLI --connect port=SWD index=${INDEX} shared reset=HWrst --write "${remaining_args[@]}" --verify -rst
