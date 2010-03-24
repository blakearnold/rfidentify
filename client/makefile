all: client

list.o: list.h list.c

reader.o: reader.c reader.h


client: client.c client.h list.o reader.o
	gcc -g -Wall client.c list.o reader.o -o client \
          -lftdi -lpthread -lcurl -lavahi-client


clean:
	rm -rf *~ client *.o






