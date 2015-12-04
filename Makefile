CC=gcc

all: sender receiver

sender: sender.c
	$(CC) sender.c -o sender

receiver: receiver.c
	$(CC) receiver.c -o receiver

.PHONY: clean

clean:
	rm receiver sender