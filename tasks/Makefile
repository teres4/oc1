CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache
TARGET2=test

all:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)

hm:
	$(CC) $(CFLAGS) testhitmiss.c L1Cache.c -o $(TARGET2)

clean:
	rm $(TARGET)