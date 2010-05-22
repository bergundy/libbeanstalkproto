CC=gcc
FLAGS=-g -std=c99 -Wall

all: producer.so

test: producer.t
	t/producer.t

producer.t: t/test_producer.c producer.so producer.h
	$(CC) $(FLAGS) -L.. -I. t/test_producer.c producer.o -o t/producer.t

producer.so: producer.o
	$(CC) $(FLAGS) -shared producer.o -o producer.so

producer.o: producer.c producer.h
	$(CC) $(FLAGS) -c producer.c

clean:
	rm -f *.o *.so t/*.t
