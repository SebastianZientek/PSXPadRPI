CC=g++
CFLAGS=-I.
LIBS=-lpthread -lpigpio

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

psxpad: main.o Rumble.o Gamepad.o
	$(CC) -o psxpad main.o Rumble.o Gamepad.o $(LIBS)
