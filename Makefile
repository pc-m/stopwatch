TARGET = stopwatch
OBJECTS = stopwatch.o main.o
HEADERS = stopwatch.h
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Winit-self -Wmissing-format-attribute -Wformat=2 -O2 -g
LDFLAGS = -Wall
LDLIBS = -lrt

.PHONY: install clean

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

install: $(TARGET)
	mkdir -p $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)

