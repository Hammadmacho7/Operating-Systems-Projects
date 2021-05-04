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
#include <sys/signal.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>

#define BUFSIZE 10
#define TCOUNT  10
#define ITEMS   10000


struct item {
        int value;   // produced value
        int source;  // index of producer
};


/***
     Definiton of shared buffer object and two operations on it:
     buf_add() and buf_rem()
 ***/

struct buffer {
        int count;
        int in; //in from which thread
        int out;
        struct item data[BUFSIZE];
        pthread_mutex_t mutex;   // for mutual exclusion
        pthread_cond_t xp; // producer will wait for a slot using this
        pthread_cond_t xc; // consumer will wait for an item using this
};


void
buf_add (struct buffer *bp, int tindex, int item)
{
    
        pthread_mutex_lock(&(bp->mutex));

        while (bp->count == BUFSIZE)
                pthread_cond_wait(&(bp->xp), &(bp->mutex));
        // sleeps in this loop as long as buffer is full
    
        // put the item into buffer
        bp->data[bp->in].value = item;
        bp->data[bp->in].source = tindex;
        bp->in = (bp->in + 1) % BUFSIZE;
        bp->count++;
    
        if (bp->count == 1)
                pthread_cond_signal(&(bp->xc));
    
        pthread_mutex_unlock(&(bp->mutex));

}


int
buf_rem (struct buffer *bp, struct item *itemp)
{
        pthread_mutex_lock(&bp->mutex);
    
        while (bp->count == 0)
                pthread_cond_wait(&bp->xc, &bp->mutex);
            // sleeps in this loop as long as buffer is empty
    
        // retrive an item from buffer
        itemp->value = bp->data[bp->out].value;
        itemp->source = bp->data[bp->out].source;
        bp->out = (bp->out + 1) % BUFSIZE;
        bp->count--;
        if (bp->count == (BUFSIZE - 1)) {
            // wake up all possible workers;
            // otherwise some workers will always sleep
                pthread_cond_broadcast (&bp->xp);
        }
        pthread_mutex_unlock(&bp->mutex);
    
        return (0);  // 0 indicates successs
}

/*** End of shared object definition  ***/


// GLOBAL VARIABLES
struct buffer *buf;     // shared bounded buffer




/* thread start function */
static void *
producer_thread(void *arg)
{

        long int index;  // thread index
        int i = 0;

        index = (long int) arg;
        for (i = 0; i < ITEMS; ++i)  {
                // value of i is the item produced
                buf_add(buf, index, i);
            
        }
        pthread_exit (NULL);
}



/* thread start function */
static void *
consumer_thread(void *arg)
{
        int n = 0;
        struct item data;

        while (1) {
                buf_rem (buf, &data); // remove an item from buffer
                n++;
                if (n == (TCOUNT * ITEMS))
                        break;
        }

        printf ("retrieved %d items\n", n);
        printf ("consumer finished successfully\n");

        pthread_exit (NULL);
}




int
main(int argc, char **argv)
{

        pthread_t pids[TCOUNT]; // producer tids
        pthread_t ctid;         // consumer tid
        int i;
        int ret;


        buf = (struct buffer *) malloc(sizeof (struct buffer));
        buf->count = 0;
        buf->in = 0;
        buf->out = 0;
        pthread_mutex_init(&buf->mutex, NULL);
        pthread_cond_init(&buf->xp, NULL);
        pthread_cond_init(&buf->xc, NULL);

	
        // create producer threads
        for (i = 0; i < TCOUNT; ++i) {
                ret = pthread_create(&pids[i], NULL, producer_thread,
				     (void *) (long) i);
                if (ret < 0) {
                        perror("thread create failed\n");
                        exit(1);
                }
        }

        // create consumer thread
        ret = pthread_create(&ctid, NULL, consumer_thread, (void *) NULL);
        if (ret < 0) {
                perror("thread create failed\n");
                exit(1);
        }


        // wait for the producer threads to terminate
        for (i = 0; i < TCOUNT; ++i)
                pthread_join(pids[i], NULL);

        // wait for the consumer thread to terminate
        pthread_join(ctid, NULL);

        free(buf);

        pthread_mutex_destroy(&buf->mutex);
        pthread_cond_destroy(&buf->xp);
        pthread_cond_destroy(&buf->xc);

        printf("program ending...bye..\n");
        exit (0);
}
