#include "Gamepad.hpp"

// #include <sys/ioctl.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <linux/joystick.h>

#define PSX_BTN_CROSS 1
#define PSX_BTN_CIRCLE 2
#define PSX_BTN_SQUARE 0
#define PSX_BTN_TRIANGLE 3
#define PSX_BTN_R1 4
#define PSX_BTN_L1 5
#define PSX_BTN_R2 6
#define PSX_BTN_L2 7
#define PSX_BTN_R3 5
#define PSX_BTN_L3 6
#define PSX_BTN_SELECT 7
#define PSX_BTN_START 4
#define PSX_BTN_DOWN 1
#define PSX_BTN_UP 3
#define PSX_BTN_RIGHT 2
#define PSX_BTN_LEFT 0

Gamepad::Gamepad()
{
    resetPsxData();
}

Gamepad::~Gamepad()
{
    thread.join();
}

void Gamepad::run(std::string jsDevice)
{
    this->jsDevice = jsDevice;

    std::thread myThread(&Gamepad::jsReaderThread, this);

    std::swap(thread, myThread);
}

std::atomic<uint8_t> *Gamepad::getGamepadDataPtr()
{
    return psxGamepadData;
}

void Gamepad::jsReaderThread()
{
    struct js_event event;
    openJS();

    while (true)
    {
        if (readJSEvent(jsFileDesc, &event) != 0)
        {
            resetPsxData();
            while (openJS() != 0) sleep(5);
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
            case 3:
                psxGamepadData[2] = (event.value + 32768) / 256;
                break;
            case 4:
                psxGamepadData[3] = (event.value + 32768) / 256;
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

int Gamepad::openJS()
{
    jsFileDesc = open(jsDevice.c_str(), O_RDONLY);
    if (jsFileDesc == -1)
    {
        std::cerr << "Could not open js0\n";
        return -1;
    }
    return 0;
}

// Private

void Gamepad::resetPsxData()
{
    std::atomic<uint8_t> *ptr = psxGamepadData;
    for (const uint8_t &val: {0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F})
    {
        *ptr++ = val;
    }
}

int Gamepad::readJSEvent(int fd, struct js_event *event)
{
    ssize_t bytes;
    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event)) return 0;

    return -1;
}

void Gamepad::setBitInGamepadData(uint8_t button, uint8_t byte, bool pressed)
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
