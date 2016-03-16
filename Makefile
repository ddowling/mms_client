CPPFLAGS=-Wall -g -O2

all: client_test set_test

client_test: client_test.o mms_client.o
	$(CXX) -o client_test client_test.o mms_client.o

client_test.o: client_test.cpp mms_client.h

mms_client.o: mms_client.cpp mms_client.h

set_test.o: set_test.cpp mms_client.h

set_test: set_test.o mms_client.o
	$(CXX) -o set_test set_test.o mms_client.o -lm


clean:
	$(RM) *.o
	$(RM) client_test set_test
