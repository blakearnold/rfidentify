all: client avahi-publish


list.o: list.h list.c


string_utils.o: list.o string_utils.c string_utils.h


reader.o: reader.c reader.h


config.o: config.c config.h


client: client.c rfid_reader_handler.c avahi_dns_handler.c \
         client.h list.o reader.o config.o string_utils.o
	gcc -g -Wall client.c rfid_reader_handler.c \
                 avahi_dns_handler.c list.o reader.o config.o \
		 string_utils.o -o client \
                 -lftdi -lpthread -lcurl -lavahi-client


test: test.c config.o list.o
	gcc -g -Wall test.c config.o list.o -o test


test_rfid: big_rfid_test.c
	gcc -g -Wall big_rfid_test.c -o rfid_test


avahi-publish: avahi-publish.c
	gcc -g -Wall avahi-publish.c -lavahi-client -o avahi-publish


clean:
	rm -rf *~ client *.o avahi-publish test 






