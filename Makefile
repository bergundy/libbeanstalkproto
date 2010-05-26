CC=gcc
FLAGS=-Wall
TEST_FLAGS=-ltap -L.. -I.
EV_FLAGS=-levent

all: libbeanstalkclient.so

test: t/producer.t t/consumer.t t/other.t
	t/producer.t
	t/consumer.t
	t/other.t

t/other.t: t/test_other.c beanstalkclient.o beanstalkclient.h
	$(CC) $(FLAGS) $(TEST_FLAGS) t/test_other.c beanstalkclient.o -o t/other.t

t/consumer.t: t/test_consumer.c beanstalkclient.o beanstalkclient.h
	$(CC) $(FLAGS) $(TEST_FLAGS) t/test_consumer.c beanstalkclient.o -o t/consumer.t

t/producer.t: t/test_producer.c beanstalkclient.o beanstalkclient.h
	$(CC) $(FLAGS) $(TEST_FLAGS) -lsockutils t/test_producer.c beanstalkclient.o -o t/producer.t

libbeanstalkclient.so: beanstalkclient.o
	$(CC) $(FLAGS) -shared beanstalkclient.o -o libbeanstalkclient.so

beanstalkclient.o: beanstalkclient.c beanstalkclient.h
	$(CC) $(FLAGS) -c beanstalkclient.c

clean:
	rm -f *.o *.so t/*.t
