CFLAGS=-Wall -g -O2

all: client_test set_test

client_test: client_test.o mms_client.o
	$(CC) -o client_test client_test.o mms_client.o

client_test.o: client_test.c mms_client.h

mms_client.o: mms_client.c mms_client.h

set_test.o: set_test.c mms_client.h

set_test: set_test.o mms_client.o
	$(CC) -o set_test set_test.o mms_client.o -lm

