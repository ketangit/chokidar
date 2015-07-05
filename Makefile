CC = g++
LIBS = -ldl -lwiringPi -lmosquitto -lkompex-sqlite-wrapper
CFLAGS = -I.
DEPS = main.h mqtt_client.h Event.h Timer.h Debounce.h
OBJ = main.o mqtt_client.o Event.o Timer.o Debounce.o

all: chokidar

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chokidar: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *o chokidar

install:
	cp chokidar ~/bin/.
