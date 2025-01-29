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

/opt/st/stm32cube_programmer/bin/STM32_Programmer.sh --connect port=SWD index=${INDEX} shared --write "${remaining_args[@]}" --verify -rst -run
