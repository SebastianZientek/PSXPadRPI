#pragma once

#include <string>
#include <atomic>
#include <thread>

class Gamepad
{
public:
    Gamepad();
    ~Gamepad();
    void run(std::string jsDevice);
    std::atomic<uint8_t> *getGamepadDataPtr();

    void jsReaderThread();

private:
    void resetPsxData();
    int readJSEvent(int fd, struct js_event *event);
    void setBitInGamepadData(uint8_t button, uint8_t byte, bool pressed);

    std::atomic<uint8_t> psxGamepadData[6];

    int jsFileDesc = 0;
    std::string jsDevice;
    std::thread thread;

    int openJS();
};
