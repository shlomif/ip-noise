
all: arb

INCLUDES = conn.h delayer.h fcs_dm.h iface.h ourrealloc.h packet_logic.h pqueue.h queue.h rand.h read.h redblack.h rwlock.h switcher.h text_queue_in.h text_queue_out.h verdict.h 

OBJECTS_PROTO = conn.o delayer.o fcs_dm.o main.o packet_logic.o pqueue.o queue.o rand.o redblack.o rwlock.o str2int_dict.o switcher.o text_queue_in.o text_queue_out.o

DEBUG = 0

CC = kgcc

ifeq ($(DEBUG),1)
	#OPT_FLAGS = -g
else
	OPT_FLAGS = -g -O2
endif

CFLAGS = -DUSE_TEXT_QUEUE_IN -DUSE_TEXT_QUEUE_OUT $(OPT_FLAGS) -Wall

$(OBJECTS_PROTO) :: %.o : %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $<

iface.o: iface.c iface_handlers.c $(INCLUDES)
	$(CC) $(CFLAGS) -c iface.c

OBJECTS = $(OBJECTS_PROTO) iface.o

arb: $(OBJECTS)
	$(CC) -Wall -o $@ $(OBJECTS) -lipq -lpthread -lm

clean:
	rm -f arb $(OBJECTS)