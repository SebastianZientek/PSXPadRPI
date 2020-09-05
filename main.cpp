#include <fcntl.h>
#include <linux/joystick.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <pigpio.h>
#include <atomic>
#include <thread>

#define PSX_BTN_CROSS 1
#define PSX_BTN_CIRCLE 2
#define PSX_BTN_SQUARE 0
#define PSX_BTN_TRIANGLE 3
#define PSX_BTN_R1 4
#define PSX_BTN_L1 5
#define PSX_BTN_R2 6
#define PSX_BTN_L2 7
#define PSX_BTN_R3 6
#define PSX_BTN_L3 5
#define PSX_BTN_SELECT 7
#define PSX_BTN_START 4
#define PSX_BTN_DOWN 1
#define PSX_BTN_UP 3
#define PSX_BTN_RIGHT 2
#define PSX_BTN_LEFT 0
#define I2C_RPI_SLAVE_ADDR 0x6A

const char *device = "/dev/input/js0";
int jsFileDesc = 0; // File descriptor for joystick file
bsc_xfer_t xfer; // For i2c (slave) communication

/**
 * @brief Gamepad data
 * bytes meaning:
 *  0: [SELECT R3 L3 START UP RGHT DOWN LEFT]
 *  1: [L2 R2 L1 R1 TRIANGLE CIRCLE CROSS SQUARE]
 *  2: [Right analog: left - right]
 *  3: [Right analog: up - down]
 *  4: [Left analog: left - right]
 *  5: [Left analog: up - down]
 *  IMPORTANT: Bit value 0 means button is pressed
 */
std::atomic<uint8_t> psxGamepadData[6];

/**
 * @brief opens joystick device for reading
 *
 * @return 0 if ok, -1 not ok
 */
static int openJS()
{
    jsFileDesc = open(device, O_RDONLY);
    if (jsFileDesc == -1)
    {
        std::cerr << "Could not open js0\n";
        return -1;
    }
    return 0;
}

/**
 * @brief Reads event from joystick
 *
 * @param fd - joystick file descriptor (e.g. for /dev/input/js0)
 * @param event - struct filled with data comming from controller
 * @return 0 - ok, -1 error
 */
static int readJSEvent(int fd, struct js_event *event)
{
    ssize_t bytes;
    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event)) return 0;

    return -1;
}

/**
 * @brief Sets bit in gamepad data
 *
 * @param button - pressed/released button
 * @param byte - number of related byte (see psxGamepadData)
 * @param pressed - is button is pressed
 */
static void setBitInGamepadData(uint8_t button, uint8_t byte, bool pressed)
{
    if (pressed)
    {
        psxGamepadData[byte] &= ~(0b10000000 >> button);
    }
    else
    {
        psxGamepadData[byte] |= 0b10000000 >> button;
    }
}

/**
 * @brief thread for reading data from connected controller
 *
 */
static void jsReaderThread()
{
    struct js_event event;
    openJS();

    while (true)
    {
        if (readJSEvent(jsFileDesc, &event) != 0)
        {
            while (openJS() != 0) sleep(1);
            continue;
        }

        switch (event.type)
        {
        case JS_EVENT_BUTTON:
            switch (event.number)
            {
            case 0:
                setBitInGamepadData(PSX_BTN_CROSS, 1, event.value);
                break;
            case 1:
                setBitInGamepadData(PSX_BTN_CIRCLE, 1, event.value);
                break;
            case 2:
                setBitInGamepadData(PSX_BTN_SQUARE, 1, event.value);
                break;
            case 3:
                setBitInGamepadData(PSX_BTN_TRIANGLE, 1, event.value);
                break;
            case 4:
                setBitInGamepadData(PSX_BTN_L1, 1, event.value);
                break;
            case 5:
                setBitInGamepadData(PSX_BTN_R1, 1, event.value);
                break;
            case 6:
                setBitInGamepadData(PSX_BTN_SELECT, 0, event.value);
                break;
            case 7:
                setBitInGamepadData(PSX_BTN_START, 0, event.value);
                break;
            case 8:
                setBitInGamepadData(PSX_BTN_L3, 0, event.value);
                break;
            case 9:
                setBitInGamepadData(PSX_BTN_R3, 0, event.value);
                break;
            }
            break;
        case JS_EVENT_AXIS:
            switch (event.number)
            {
            case 0:
                psxGamepadData[4] = (event.value + 32768) / 256;
                break;
            case 1:
                psxGamepadData[5] = (event.value + 32768) / 256;
                break;
            case 2:
                setBitInGamepadData(PSX_BTN_L2, 1, event.value > 0);
                break;
            case 5:
                setBitInGamepadData(PSX_BTN_R2, 1, event.value > 0);
                break;
            case 6:
                setBitInGamepadData(PSX_BTN_RIGHT, 0, event.value > 1);
                setBitInGamepadData(PSX_BTN_LEFT, 0, event.value < -1);
                break;
            case 7:
                setBitInGamepadData(PSX_BTN_UP, 0, event.value < -1);
                setBitInGamepadData(PSX_BTN_DOWN, 0, event.value > 1);
                break;
            }
            break;
        }
    }
}

/**
 * @brief thread that handles opened i2c connection
 * with stm32 board
 *
 */
static void i2cSlaveConnection()
{
    gpioInitialise();

    xfer.control = 0x40; // Disable i2c, abort operations
    bscXfer(&xfer);

    xfer.control = (I2C_RPI_SLAVE_ADDR << 16) | 0x305; // Enable i2c (tx/rx), pass the address
    xfer.txCnt = 0;

    while (1)
    {
        if (xfer.rxCnt == 1 && xfer.rxBuf[0] == 'r')
        {
            memcpy(xfer.txBuf, psxGamepadData, 6);
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
    std::atomic<uint8_t> *ptr = psxGamepadData;
    for (const uint8_t &val: {0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F})
    {
        *ptr++ = val;
    }
    gpioInitialise();
    gpioSetPullUpDown(18, PI_PUD_UP);
    gpioSetPullUpDown(19, PI_PUD_UP);
}

int main()
{
    init();

    std::thread jsReader{jsReaderThread};
    std::thread i2cConnection(i2cSlaveConnection);

    jsReader.join();
    i2cConnection.join();

    return 0;
}
