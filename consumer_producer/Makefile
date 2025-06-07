CC = gcc
CFLAGS = -Wall -Wextra

all: producer consumer

producer: producer.c common.h
	$(CC) $(CFLAGS) -o producer producer.c

consumer: consumer.c common.h
	$(CC) $(CFLAGS) -o consumer consumer.c

clean:
	rm -f producer consumer
	rm -f /tmp/shm.*
	rm -f /tmp/sem.* 