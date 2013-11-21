CC=clang
CFLAGS=-c -Werror
LDFLAGS=
SOURCES=main.c daemonize.c logging.c utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=GoHipsterd

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	-rm -rf *.o $(EXECUTABLE)
