CC = gcc
CFLAGS=-Wall -Wextra
TARGET=test
FILE1 = output.txt
FILE2 = results_L1.txt
DIFF_FILE = diff.txt

all:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)


clean:
	rm $(TARGET) $(FILE1) $(DIFF_FILE)

output:
	./test > $(FILE1)

resposta:
	@diff -u $(FILE1) $(FILE2) > $(DIFF_FILE) || echo "Differences found. Please check the output."