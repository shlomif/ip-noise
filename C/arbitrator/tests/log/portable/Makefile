all: test

log.o: log.c
	gcc -c -Wall log.c

test_log.o: test_log.c
	gcc -c -Wall test_log.c

test: log.o test_log.o
	gcc -o test log.o test_log.o -lm
