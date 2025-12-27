CC = gcc
CFLAGS = -Wall -Wextra -I include
TARGET = test_djlc.elf

.PHONY: all clean test

all: $(TARGET)

$(TARGET): test_djlc.c
	$(CC) $(CFLAGS) -o $@ $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

