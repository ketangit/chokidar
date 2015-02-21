CC = g++
LIBS = -lwiringPi -lmosquitto
CFLAGS = -I.
DEPS = mqtt_client.h
OBJ = main.o mqtt_client.o

all: chokidar

%.o: %.cpp $(DEPS)
    $(CC) -c -o $@ $< $(CFLAGS)

chokidar: $(OBJ)
    $(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
    rm -rf *o chokidar
