CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -lsqlite3 -phtread

SRC = main.c

main: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: run
run:
	make main
	./main
	make clean

.PHONY: clean
clean:
	rm main
