#pragma once

#include <string>
#include <cinttypes>

#include <linux/input.h>

class Vibration
{
public:
    Vibration();
    void setDevice(std::string device);
    void applyForces(const uint8_t &smallMotorForce, const uint8_t &largeMotorForce);

private:
    int evFileDesc = 0;
    std::string evDevice;
    ff_effect effect;
    input_event play;

    int openEV();
};
