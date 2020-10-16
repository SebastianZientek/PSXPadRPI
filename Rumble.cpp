#include "Rumble.hpp"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <cstring>

Rumble::Rumble()
{
}

void Rumble::setDevice(std::string device)
{
    evDevice = device;
}

void Rumble::applyForces(const uint8_t &smallMotorForce, const uint8_t &largeMotorForce)
{
    if (evFileDesc == -1)
    {
        if (openEV() == -1)
        {
            return;
        }
    }

    effect.u.rumble.strong_magnitude = largeMotorForce * 255 * 0.2;
    effect.u.rumble.weak_magnitude = smallMotorForce * 255 * 0.2;

    if (ioctl(evFileDesc, EVIOCSFF, &effect) == -1) {
        perror("Error");
        evFileDesc = -1;
        return;
    }
}

int Rumble::openEV()
{
    evFileDesc = open(evDevice.c_str(), O_RDWR);
    if (evFileDesc == -1)
    {
        return -1;
    }

	effect.type = FF_RUMBLE;
	effect.id = -1;
	effect.u.rumble.strong_magnitude = 0;
	effect.u.rumble.weak_magnitude = 0;
	effect.replay.length = 0;
	effect.replay.delay = 0;

    if (ioctl(evFileDesc, EVIOCSFF, &effect) == -1) {
        perror("Error");
        evFileDesc = -1;
        return -1;
    }

	memset(&play,0,sizeof(play));
	play.type = EV_FF;
	play.code = effect.id;
	play.value = 1;


    if (write(evFileDesc, (const void*)&play, sizeof(play)) == -1) {
        perror("Play effect");
        evFileDesc = -1;
        return -1;
    }

    return 0;
}
