/* -*- linux-c -*- */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "common.h"

int
main(int argc, char **argv)
{
        int fd;
        struct stat sbuf;
	void *shm_start;
	struct shared_data *sdp;
	int item;     /* retrieved item */
	int expected; /* expected item */

        /* semaphores */
        sem_t *sem_mutex; 
        sem_t *sem_full;
        sem_t *sem_empty;

	/* open the shared memory segment */
	fd = shm_open(SHM_NAME, O_RDWR, 0600);
	if (fd < 0) {
		perror("can not open shm\n");
		exit (1); 
	}
	printf("shm open success, fd = %d\n", fd);
	
	fstat(fd, &sbuf);
	printf("shm size = %d\n", (int) sbuf.st_size);	
	
	shm_start = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, 
			MAP_SHARED, fd, 0);

	if (shm_start < 0) {
		perror("can not map the shm \n");
		exit (1); 
	}
	printf ("mapped shm; start_address=%lu\n", (unsigned long)shm_start);
	close(fd);	

	sdp = (struct shared_data *) shm_start; 

	/* open the semaphores; 
	   they should already be created and initialized  */
	sem_mutex = sem_open(SEMNAME_MUTEX, O_RDWR);
	if (sem_mutex < 0) {
		perror("can not open semaphore\n");
		exit (1); 
	} 
	printf("sem %s opened\n", SEMNAME_MUTEX);


	sem_full = sem_open(SEMNAME_FULL, O_RDWR);
	if (sem_full < 0) {
		perror("can not open semaphore\n");
		exit (1); 
	}
	printf("sem %s opened\n", SEMNAME_FULL);

	sem_empty = sem_open(SEMNAME_EMPTY, O_RDWR);
	if (sem_empty < 0) {
		perror("can not open semaphore\n");

	} 
	printf("sem %s opened\n", SEMNAME_EMPTY);

	sdp->out = 0;
	item = -1; 
	expected = 0;
	while (expected  < NUM_ITEMS) {
		if (SYNCHRONIZED) {
			sem_wait(sem_full);
			sem_wait(sem_mutex);

			item = sdp->buf[sdp->out];
			sdp->out = (sdp->out + 1) % BUFSIZE;
			sdp->count--; 

			sem_post(sem_mutex);
			sem_post(sem_empty);
		} else {
			while (sdp->count == 0) 
				; /* busy wait */
			sdp->count--; 
			item  = sdp->buf[sdp->out];
			sdp->out = (sdp->out + 1) % BUFSIZE;
		}

		if (TRACE)
			printf("consumer retrieved item=%d\n", item);
		
		if (item != expected) {
			printf("race condition occured; expected=%d, item=%d\n",
			       expected, item);
			exit(1);
		}
		expected++;
	}

	sem_close(sem_mutex);
	sem_close(sem_full);
	sem_close(sem_empty);


	/* remove the semaphores */
	sem_unlink(SEMNAME_MUTEX);
	sem_unlink(SEMNAME_FULL);
	sem_unlink(SEMNAME_EMPTY);

	/* remove the shared memory */
	shm_unlink(SHM_NAME);

	printf("consumer ended; bye...\n");
	exit(0);
}
