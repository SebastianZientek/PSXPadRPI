#include "Vibration.hpp"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

Vibration::Vibration()
{
	effect.type = FF_RUMBLE;
	effect.id = -1;
	effect.u.rumble.strong_magnitude = 0x0000;
	effect.u.rumble.weak_magnitude = 0x0000;
	effect.replay.length = 1;
	effect.replay.delay = 0;

    play.type = EV_FF;
    play.code = effect.id;
    play.value = 1;
}

void Vibration::setDevice(std::string device)
{
    evDevice = device;
}

void Vibration::applyForces(const uint8_t &smallMotorForce, const uint8_t &largeMotorForce)
{
    if (evFileDesc == 0)
    {
        if (openEV() == -1)
        {
            return;
        }
    }

    effect.u.rumble.strong_magnitude = largeMotorForce * 255 * 0.2;
    effect.u.rumble.weak_magnitude = smallMotorForce * 255 * 0.2;

    if (ioctl(evFileDesc, EVIOCSFF, &effect) == -1)
    {
        evFileDesc = 0;
        return;
    }

    play.code = effect.id;

    if (write(evFileDesc, (const void*) &play, sizeof(play)) == -1) {
        evFileDesc = 0;
        return;
    }
}

int Vibration::openEV()
{
    evFileDesc = open(evDevice.c_str(), O_RDWR);
    if (evFileDesc == -1)
    {
        std::cerr << "Could not open event0\n";
        return -1;
    }
    return 0;
}
