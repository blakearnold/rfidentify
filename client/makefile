all: client avahi-publish


list.o: list.h list.c


reader.o: reader.c reader.h


client: client.c rfid_reader_handler.c avahi_dns_handler.c \
         client.h list.o reader.o
	gcc -g -Wall client.c rfid_reader_handler.c \
                 avahi_dns_handler.c list.o reader.o -o client \
                 -lftdi -lpthread -lcurl -lavahi-client


avahi-publish: avahi-publish.c
	gcc -g -Wall avahi-publish.c -lavahi-client -o avahi-publish


clean:
	rm -rf *~ client *.o avahi-publish






