
all: arb

INCLUDES = conn.h delayer.h iface.h pqueue.h queue.h rwlock.h verdict.h

OBJECTS_PROTO = conn.o delayer.o main.o pqueue.o queue.o redblack.o rwlock.o str2int_dict.o

$(OBJECTS_PROTO) :: %.o : %.c $(INCLUDES)
	gcc -g -Wall -c $<

iface.o: iface.c iface_handlers.c $(INCLUDES)
	gcc -g -Wall -c iface.c

OBJECTS = $(OBJECTS_PROTO) iface.o

arb: $(OBJECTS)
	gcc -g -Wall -o $@ $(OBJECTS) -lipq -lpthread

clean:
	rm -f arb $(OBJECTS)