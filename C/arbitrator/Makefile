
all: arb


OBJECTS = main.o

$(OBJECTS) :: %.o : %.c
	gcc -c $<

arb: $(OBJECTS)
	gcc -o $@ $(OBJECTS) -lipq