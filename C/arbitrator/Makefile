
all: arb

INCLUDES = conn.h delayer.h fcs_dm.h iface.h packet_logic.h pqueue.h queue.h rand.h rwlock.h switcher.h verdict.h 

OBJECTS_PROTO = conn.o delayer.o fcs_dm.o main.o packet_logic.o pqueue.o queue.o rand.o redblack.o rwlock.o str2int_dict.o switcher.o

$(OBJECTS_PROTO) :: %.o : %.c $(INCLUDES)
	gcc -g -Wall -c $<

iface.o: iface.c iface_handlers.c $(INCLUDES)
	gcc -g -Wall -c iface.c

OBJECTS = $(OBJECTS_PROTO) iface.o

arb: $(OBJECTS)
	gcc -g -Wall -o $@ $(OBJECTS) -lipq -lpthread -lm -lefence

clean:
	rm -f arb $(OBJECTS)