# Overview

This package contains implementations of each board type supported by this codebase. These boards should allow for compile-time discovery of its peripherals. Board classes should also provide simple accessors for each of these peripherals. To the extent possible, the goal here is to be correct by construction -- if you can define a usage of the board and its peripherals, it should work.

# Key responsibilities of Board types

- Handle bootstrapping the board to a default state
- Provide accessors to peripherals
- Be trivially interchangeable with other Boards when both boards have the peripherals required for the use case
- Provide mechanisms to vector ISRs to the appropriate peripheral instances (ie, callbacks)
- Provide implementations of ISRs that affect the system state

# Design elements to achieve these goals

- All board types are classes named Board. This ensures easy substitution of one board for another without conditional compilation.
- All Board classes are final. We do not provide facilities to inherit from them. This minimizes cognitive overhead in understanding the functionality of the board.
- Board classes should have the same function signatures for the same features and peripherals. Usually, this means adhering to the accessor names already created in other Board classes or changing them all at the same time.
- The exact board header is selected at compile time in the file board.h. Changing the 
 build configuration should change the Board class being used *without any other conditional compilation*.
- Boards are singletons. We define a board to be a distinct unit of computation and its peripherals. This is usually a PCB with an MCU, sensors, memories, power, etc. By definition, we have one Board per executable as the Board represents the environment where the code is running.
- ISRs are typically simple vectors to a callback that includes a reference to the appropriate peripheral. We want to offload the logic of the processing to the peripheral implementations.
- The Board instance is instantiated in a single place in board.cc. This instance is created without including a specific board header file, only board.h. board.cc does not include any conditional compilation. This approach ensures that all Board types are instantiated the same way and at the same logical time in software startup.
- ISRs that apply to all boards are also defined in board.cc. These ISRs represent the least common denominator of interrupt functionality. ISRs that do not apply to all boards are defined in other implementation files.
- Bind the ISRs to the specific instance of the peripheral in the <system>_board.cc file. And have each of the peripherals expose a method to handle the interrupt. This allows the binding to happen where the peripheral instances are known and also allows the implementation to be specific to the concrete type of the peripheral.
