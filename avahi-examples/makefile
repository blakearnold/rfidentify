

all: avahi-poll avahi-publish




avahi-poll: avahi-poll.c
	gcc -g -Wall avahi-poll.c -lavahi-client -o avahi-poll



avahi-publish: avahi-publish.c
	gcc -g -Wall avahi-publish.c -lavahi-client -o avahi-publish


clean:
	rm -rf *~ avahi-poll avahi-publish