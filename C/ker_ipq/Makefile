
UML = 1

ifeq ($(UML),1)
	LINUX_KERNEL_DIR = /home/shlomi/Download/unpack/kernel/User-Mode-Linux/kernel/linux
	INCLUDES = -I$(LINUX_KERNEL_DIR)/include -I$(LINUX_KERNEL_DIR)/arch/um/include
	CFLAGS = -g -c -Wall $(INCLUDES) -U__i386__ -DMODULE -D__KERNEL__ -DUM_FASTCALL -DNESTING=0 -D__arch_um__ -DSUBARCH="i386"
else
	LINUX_KERNEL_DIR = /usr/src/linux
	CFLAGS = -g -c -Wall -I$(LINUX_KERNEL_DIR)/include -DMODULE -D__KERNEL__
endif

OBJECTS = ip_queue.o rand.o pqueue.o delayer.o k_stdlib.o


ip_queue_ker.o: $(OBJECTS)
	ld -r -o $@ $(OBJECTS)

$(OBJECTS) :: %.o : %.c
	gcc $(CFLAGS) -o $@ $<
clean:
	rm -f *.o
