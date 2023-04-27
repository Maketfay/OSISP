CC = gcc
ARG = 127.0.0.1 34543
REMOTE_ARG = 178.124.147.66 5020

SRC = main.c

main: $(SRC)
	$(CC) -o $@ $^

.PHONY: run
run:
	make main
	./main $(ARG)
	make clean
.PHONY: remote

remote:
	make main
	./main $(REMOTE_ARG)
	make clean

.PHONY: clean
clean:
	rm main
