CFLAGS = -std=c11 -Wall

bin: test.c
	gcc $(CFLAGS) test.c -o test

debugbin: test.c
	gcc $(CFLAGS) -g test.c -o test

lib: lock.c hashtable.o
	gcc $(CFLAGS) -fPIC -shared -o ./libpreload.so lock.c hashtable.o -ldl

test: lib bin
	LD_PRELOAD=./libpreload.so ./test

hashtable.o: hashtable.c
	gcc $(CFLAGS) -c hashtable.c

htest: ht_test.c hashtable.o
	gcc $(CFLAGS) ht_test.c hashtable.o -o htest
