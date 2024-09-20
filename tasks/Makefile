CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache
TARGET2=test
FILE1 = output.txt
FILE2 = results_L1.txt


all:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)

hm:
	$(CC) $(CFLAGS) testhitmiss.c L1Cache.c -o $(TARGET2)

clean:
	rm $(TARGET)

runtest:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET2)

output:
	./test > output.txt


resposta:
	@diff -u $(FILE1) $(FILE2) > diff.txt || echo "Differences found. Please check the output."