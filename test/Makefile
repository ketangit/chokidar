DIR=$(DESTDIR)/opt/chokidar
CFGDIR=$(DESTDIR)/etc/chokidar

all: chokidar

.PHONY: all

chokidar: main.c mqtt_client.c
	export LIBRARY_PATH=$(COMPILELIBDIR):$(LIBRARY_PATH)
	$(CC) main.c mqtt_client.c -o $@ -lpaho-mqtt3as -lpthread -lssl -lm -I .
	strip $@

install: chokidar
	mkdir -p $(DIR)
	mkdir -p $(CFGDIR)
	install chokidar $(DIR)/chokidar

clean:
	rm -f chokidar

