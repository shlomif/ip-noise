
all: arb

INCLUDES = delayer.h pqueue.h queue.h verdict.h

OBJECTS = delayer.o main.o pqueue.o queue.o

$(OBJECTS) :: %.o : %.c $(INCLUDES)
	gcc -g -Wall -c $<

arb: $(OBJECTS)
	gcc -g -Wall -o $@ $(OBJECTS) -lipq -lpthread

clean:
	rm -f arb $(OBJECTS)