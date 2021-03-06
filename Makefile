C=clang
CFLAGS=-g -c -Werror
LDFLAGS=-lpthread
SOURCES=main.c daemonize.c logging.c utils.c GoRaspberry.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=GoRaspberryd

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

install:
	cp $(EXECUTABLE) /usr/sbin/$(EXECUTABLE)

uninstall:
	rm -f /usr/sbin/$(EXECUTABLE)

.PHONY: clean

clean:
	-rm -rf *.o $(EXECUTABLE)
