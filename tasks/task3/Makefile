CC = gcc
CFLAGS=-Wall -Wextra
TARGET=L2Cache
TARGET2=test
FILE1 = output.txt
FILE2 = results_L2_1W.txt
DIFF_FILE = diff.txt

all:
	$(CC) $(CFLAGS) SimpleProgramL2.c L2Cache2w.c -o $(TARGET)
	$(CC) $(CFLAGS) SimpleProgramL2.c L2Cache2w.c -o $(TARGET2)

	./test > $(FILE1)
	@diff -u $(FILE1) $(FILE2) > diff.txt || echo "Differences found. Please check the output."


clean:
	rm $(TARGET) $(TARGET2) $(FILE1) $(DIFF_FILE)

# output:
# 	./test > $(FILE1)


# resposta:
# 	@diff -u $(FILE1) $(FILE2) > diff.txt || echo "Differences found. Please check the output."
