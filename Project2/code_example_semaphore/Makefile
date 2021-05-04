
all: producer consumer

producer: producer.c
	gcc -Wall -g -o producer producer.c -lrt -lpthread

consumer: 	    
	gcc -Wall -g -o consumer consumer.c -lrt -lpthread


clean:
	rm -fr producer consumer *~ *.o   core*
