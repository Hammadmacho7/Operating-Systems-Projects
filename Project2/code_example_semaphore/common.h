/* -*- linux-c  -*-  */

#ifndef COMMON_H
#define COMMON_H

#define SEMNAME_MUTEX       "/name_sem_mutex"
#define SEMNAME_FULL        "/name_sem_fullcount"
#define SEMNAME_EMPTY       "/name_sem_emptycount"

#define SHM_NAME  "/name_shm_sharedsegment1"

#define BUFSIZE  10		/*  bounded buffer size */

#define NUM_ITEMS 1000000		/*  total items to produce  */

/* set to 1 to synchronize; 
   otherwise set to 0 and see race condition */
#define SYNCHRONIZED 1   // You can play with this and see race
#define TRACE 1

struct shared_data {
	int buf[BUFSIZE];    /* shared buffer */
	int count;           /* current number of items in buffer */
	int in; // this field is only accessed by the producer
	int out; // this field is only accessed by the consumer
};

#endif
