
all: arb

INCLUDES = conn.h delayer.h iface.h pqueue.h queue.h rwlock.h verdict.h

OBJECTS = conn.o delayer.o iface.o main.o pqueue.o queue.o redblack.o rwlock.o str2int_dict.o

$(OBJECTS) :: %.o : %.c $(INCLUDES)
	gcc -g -Wall -c $<

arb: $(OBJECTS)
	gcc -g -Wall -o $@ $(OBJECTS) -lipq -lpthread

clean:
	rm -f arb $(OBJECTS)