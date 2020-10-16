#pragma once

#include <string>
#include <cinttypes>

#include <linux/input.h>

class Rumble
{
public:
    Rumble();
    void setDevice(std::string device);
    void applyForces(const uint8_t &smallMotorForce, const uint8_t &largeMotorForce);

private:
    int evFileDesc = -1;
    std::string evDevice;
    ff_effect effect;
    input_event play;

    int openEV();
};
