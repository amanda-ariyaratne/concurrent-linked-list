target1: TARGET=sequential
target2: TARGET=mutex
target3: TARGET=rwlock

sequential.o: sequential.c
	gcc -g -Wall -o serial serial.c -lpthread

mutex.o: mutex.c
	gcc -g -Wall -o mutex mutex.c -lpthread

rwlock: read_write_lock.c
	@echo "Read wite lock..."
	gcc -g -Wall -c read_write_lock.c -lpthread

clean:
	rm *.*