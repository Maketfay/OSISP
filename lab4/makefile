CC = gcc


SRC = main.c

ipc: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: run
run:
	make main
	./main
	make clean

.PHONY: clean
clean:
	rm main
