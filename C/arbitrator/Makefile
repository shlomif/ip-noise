
all: arb


OBJECTS = main.o queue.o

$(OBJECTS) :: %.o : %.c
	gcc -g -Wall -c $<

arb: $(OBJECTS)
	gcc -g -Wall -o $@ $(OBJECTS) -lipq -lpthread

clean:
	rm -f arb $(OBJECTS)