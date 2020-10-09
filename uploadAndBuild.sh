rsync *.cpp pi@raspberrypi:/home/pi/PSXPadRPI/
rsync *.hpp pi@raspberrypi:/home/pi/PSXPadRPI/
rsync Makefile pi@raspberrypi:/home/pi/PSXPadRPI/
ssh pi@raspberrypi "cd PSXPadRPI; make"


