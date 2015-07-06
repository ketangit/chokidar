DIR=$(DESTDIR)/opt/chokidard

CC = g++
LIBS = -ldl -lwiringPi -lmosquitto
CFLAGS = -I.
DEPS = main.h mqtt_client.h Event.h Timer.h PortDebounce.h I2CPortDebounce.h
OBJ = main.o mqtt_client.o Event.o Timer.o PortDebounce.o I2CPortDebounce.o

all: chokidard

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chokidard: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *o chokidard
	rm -f chokidard
	rm -f debian/debhelper.log
	rm -f debian/files
	rm -f debian/postinst.debhelper
	rm -f debian/postrm.debhelper
	rm -f debian/prerm.debhelper
	rm -f debian/substvars
	rm -rf debian/tmp
	rm -f ../chokidard_*

install:
	mkdir -p $(DIR)	
	install chokidard $(DIR)/chokidard
