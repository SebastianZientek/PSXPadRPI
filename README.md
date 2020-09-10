# PSXPadRPI

PSXPad is the project that can emulate communication between Playstation 1 and linux compatible gamepad. This repository is related to one part of two of this project. Second part is named PSXPadStm32 and you can find it here: https://github.com/bleakdev/PSXPadSTM32

![PSXPad](https://github.com/bleakdev/PSXPadSTM32/raw/master/media/photo.png)

# How it works
To run this project you will need two boards: Raspberry PI (I tested it with RPI Zero) and Blue pill (stm32 compatible board).

Connection will look like bellow:

```[Gamepad] <--BT, WIRE etc.--> [RPI] <--i2c--> [STM32] <--SPI--> [Playstation 1]```

# Why not just RPI?
Playstation 1 talks with gamepads over SPI. SPI is protocol that works in master-slave architecture. In this case our master will be PS1 and controller will work as slave. This is the point where just raspberry is not enough. RPI have SPI port, but it can works only in master mode. Theoretically there is pheriferial to run SPI as slave, but no one can handle this (if you don't belive just google it). I tried to implement it as bitbang, but it was to slow. In general controller was working, but sometimes weird things happened (sometimes not whole bytes was transmitted so it was looking like pushing random buttons occasionally).

# Why not just stm32?
Answer is simple - drivers. RPI can handle most of known controllers. On Stm32 I will be forced to write my own drivers. That will be too hard and time consuming for this little project.

# Limitations
I tested everything only on Xbox one S gamepad so buttons mapping for another devices may be wrong. There is no rumble support (at least yet). I can handle only one controller at time, and controller needs to be visible under /dev/input/js0
