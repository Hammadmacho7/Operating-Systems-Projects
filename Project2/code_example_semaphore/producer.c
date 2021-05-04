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

#define SHM_SIZE sizeof (struct shared_data) 

int
main(int argc, char **argv)
{

	sem_t *sem_mutex;       /* protects the buffer */
	sem_t *sem_full;        /* counts the number of items */
	sem_t *sem_empty;       /* counts the number of empty buffer slots */


	int fd;                     /* shm file descriptor */
	struct stat sbuf;           /* info about shm */
	void *shm_start;            /* pointer to the start of shared memory  */
	struct shared_data * sdp;   /* pointer to shared data structure */
	int item;                   /* an integer item to pass */
	int i; 


	/* first clean up a shm with same name */
	shm_unlink (SHM_NAME);
	
	/* create a shared memory segment */
	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
	if (fd < 0) {
		perror("can not create shared memory\n");
		exit (1); 
	}
	printf("shm created, fd = %d\n", fd);
		
	ftruncate(fd, SHM_SIZE);    /* set size of shared memmory */
	fstat(fd, &sbuf);
	printf("shm_size=%d\n", (int) sbuf.st_size);   

	shm_start = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, 
			 MAP_SHARED, fd, 0);
	if (shm_start < 0) {
		perror("can not map shm\n");
		exit (1); 
	}
	printf ("mapped shm; start_address=%lu\n", (unsigned long) shm_start);
	close(fd); /* no longer need the descriptor */

	sdp = (struct shared_data *) shm_start; 
	for (i = 0; i < BUFSIZE; ++i)
		sdp->buf[i] = 0; 
	sdp->count = 0; 
	sdp->in = 0; 


	/* first clean up semaphores with same names */
	sem_unlink (SEMNAME_MUTEX);
	sem_unlink (SEMNAME_FULL);
	sem_unlink (SEMNAME_EMPTY); 

	/* create and initialize the semaphores */
	sem_mutex = sem_open(SEMNAME_MUTEX, O_RDWR | O_CREAT, 0660, 1);
	if (sem_mutex < 0) {
		perror("can not create semaphore\n");
		exit (1); 
	}
	printf("sem %s created\n", SEMNAME_MUTEX);
 
	sem_full = sem_open(SEMNAME_FULL, O_RDWR | O_CREAT, 0660, 0);
	if (sem_full < 0) {
		perror("can not create semaphore\n");
		exit (1); 
	} 
	printf("sem %s created\n", SEMNAME_FULL);

	sem_empty =
		sem_open(SEMNAME_EMPTY, O_RDWR | O_CREAT, 0660, BUFSIZE);
	if (sem_empty < 0) {
		perror("can not create semaphore\n");
		exit (1); 
	} 
	printf("sem %s create\n", SEMNAME_EMPTY); 
			
	item  = 0; 
	while (item < NUM_ITEMS) {
		if (SYNCHRONIZED) {
			sem_wait(sem_empty);
			sem_wait(sem_mutex);

			sdp->buf[sdp->in] = item; 
			sdp->in = (sdp->in + 1) % BUFSIZE;
			sdp->count ++; 
			printf ("produced %d\n", item);
			sem_post(sem_mutex);
			sem_post(sem_full);
		} else {
			while (sdp->count == BUFSIZE) 
				; /* busy wait */
			sdp->count++; 
			sdp->buf[sdp->in] = item;
			sdp->in = (sdp->in + 1) % BUFSIZE;
		}

		if (TRACE) 
			printf("producer put item=%d\n", item);

		item++;
		// are are just putting integers as items 
	}

	sem_close(sem_mutex);
	sem_close(sem_full);
	sem_close(sem_empty);

	printf("producer ended; bye...\n");
	exit(0);
}
