CC = g++
LIBS = -ldl -lwiringPi -lmosquitto -lkompex-sqlite-wrapper
CFLAGS = -I.
DEPS = main.h mqtt_client.h Event.h Timer.h PortDebounce.h I2CPortDebounce.h
OBJ = main.o mqtt_client.o Event.o Timer.o PortDebounce.o I2CPortDebounce.o

all: chokidar

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chokidar: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *o chokidar

install:
	cp chokidar ~/bin/.
