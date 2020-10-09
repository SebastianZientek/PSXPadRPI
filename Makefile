CC=g++
CFLAGS=-I.
LIBS=-lpthread -lpigpio

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

psxpad: main.o Vibration.o
	$(CC) -o psxpad main.o Vibration.o $(LIBS)
