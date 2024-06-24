<b>STM32H745ZI_M4</b>

This project aims to bring up the M4 core of a STM32H745ZI dual core microcontroller, the other (M7) core is covered in my STM32H745ZI_M7 repo. The goal is to set up the chip from power-on boot working mostly at register level, and bring it to the max clock rate with several peripherals running in bare metal mode, and finally to enable the two cores to talk to each other.

Toolchain is Visual Studio with the VisualGDB plugin for talking to the microcontroller. The target hardware is a Nucleo-H645ZI-Q development board.

This is also my first time using GitHub, so that will be a fun learning process too.
