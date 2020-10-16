#include <fcntl.h>
#include <linux/joystick.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <pigpio.h>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <csignal>

#include "Rumble.hpp"
#include "Gamepad.hpp"

#define I2C_RPI_SLAVE_ADDR 0x6A

bsc_xfer_t xfer; // For i2c (slave) communication

Gamepad gamepad;
Rumble rumble;


/**
 * @brief thread that handles opened i2c connection
 * with stm32 board
 *
 */
static void i2cSlaveConnection()
{
    xfer.control = 0x40; // Disable i2c, abort operations
    bscXfer(&xfer);

    xfer.control = (I2C_RPI_SLAVE_ADDR << 16) | 0x305; // Enable i2c (tx/rx), pass the address
    xfer.txCnt = 0;

    while (1)
    {
        if (xfer.rxCnt == 3 && xfer.rxBuf[0] == 'r')
        {
            std::atomic<uint8_t> * data = gamepad.getGamepadDataPtr();
            for (uint8_t i = 0; i < 6; ++i) xfer.txBuf[i] = data[i];

            const uint8_t &smallMotor = xfer.rxBuf[1];
            const uint8_t &largeMotor = xfer.rxBuf[2];

            rumble.applyForces(smallMotor, largeMotor);

            xfer.rxBuf[1] = 0;
            xfer.rxBuf[2] = 0;

            xfer.txCnt = 6;
        }
        else
        {
            xfer.txCnt = 0;
        }

        bscXfer(&xfer);
    }
}

/**
 * @brief initialize data and hardware
 *
 */
static void init()
{
    gpioInitialise();
    gpioSetPullUpDown(18, PI_PUD_UP);
    gpioSetPullUpDown(19, PI_PUD_UP);
}

void signalHandler(int signum)
{
    xfer.control = 0x40; // Disable i2c, abort operations
    bscXfer(&xfer);

    exit(signum);
}

int main()
{
    init();

    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);

    gamepad.run("/dev/input/js0");
    rumble.setDevice("/dev/input/event0");

    std::thread i2cConnection(i2cSlaveConnection);
    i2cConnection.join();

    return 0;
}
