   
all: mutexcond bcast gen

mutexcond: mutexcond.c
	gcc -Wall -o  mutexcond mutexcond.c -lrt -lpthread

bcast: bcast.c
	gcc -Wall -o  bcast bcast.c -lrt -lpthread

gen: gen.c
	gcc -Wall -o gen gen.c  -lrt -lpthread

clean: 
	rm -fr *~  mutexcond bcast  gen
