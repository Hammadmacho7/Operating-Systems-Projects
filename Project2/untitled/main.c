#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#define TRACE 1
int N;
int Bcount = 7; //count of bursts for each thread
int BcountFromFile;
char algGiven[128];
char FCFS[128] = "FCFS";
char SJF[128] = "SJF";
char PRIO[128] = "PRIO";
char VRUNTIME[128] = "VRUNTIME";

//int BUFSIZE = N * Bcount;
int avgA = 1500;
int avgB = 200;
int minA = 1000;
int minB = 100;
double vruntime[11]; //11 as I wanna keep with 1 and 2 and all that
double waitingtime[11];

//int listSize   =  N * Bcount;

#define MAXFILENAME 128
#define ENDOFDATA   -1 /* marks the end of data stream from producer */

double exponentialDist(int random, int average){ //param is the random
    double lambda = 1/average;
    double try = exp(-lambda * random);
    double result = lambda * try;
    return result;
}



struct bb_qelem {
    struct bb_qelem *next;
    int threadIndex;  /* an item - an integer */
    double time;
    double length;
    int burstIndex;
    double wallClockTime;
};

struct bb_queue {
    struct bb_qelem *head;
    struct bb_qelem *tail;
    int             count;   /* number of items in the buffer */
};


void
bb_queue_init(struct bb_queue *q)
{
    q->count = 0;
    q->head = NULL;
    q->tail = NULL;
}


// this function assumes that space for item is already allocated
void
bb_queue_insert(struct bb_queue *q, struct bb_qelem *qe)
{

    if (q->count == 0) {
        q->head = qe;
        q->tail = qe;
    } else {
        q->tail->next = qe;
        q->tail = qe;
    }

    q->count++;
}

// this function does not free the item
// it is deleting the item from the list
struct bb_qelem *
bb_queue_retrieve(struct bb_queue *q)
{
    struct bb_qelem *qe;

    if (q->count == 0)
        return NULL;

    qe = q->head;
    q->head = q->head->next;
    q->count--;

    return (qe);
}

struct bb_qelem *
bb_queue_retrieve_SJF(struct bb_queue *q)
{
    struct bb_qelem *minimumNode;
    struct bb_qelem *tracer;

    if (q->count == 0)
        return NULL;

    if( q->count == 1){
        return q->head;
    }
    minimumNode = q->head; ///points to the head node, starting from there
    //tracer = q->head;

    ///this loop is for finding the node with smallest burst length
    for(tracer = q->head; tracer != NULL; tracer = tracer->next){
        if( tracer->length < minimumNode->length){
            minimumNode = tracer;
        }
    }
    if( minimumNode == q->head){
        q->head = q->head->next; //move to the next one
        q->count--;
        return minimumNode;

    }
    if( minimumNode == q->tail ){
        // for(tracer = q->head; tracer->next != q->tail; tracer = tracer->next){
        //just to reach that point
        //}
        tracer = q->head;
        while( tracer->next != q->tail){
            //just traverse
            tracer = tracer->next;
        }
        tracer->next = NULL;
        q->tail = tracer; //new tail
        q->count--;
        return minimumNode;
    }
    tracer = q->head;
    while(tracer->next != minimumNode){
        tracer = tracer->next;
    }
    // for(tracer = q->head; tracer->next != minimumNode; tracer = tracer->next){
    //this way I'LL get the node before minimum node
    //}
    tracer->next = minimumNode->next; //connecting to the node after min node
    q->count--;

    return minimumNode;
}

struct bb_qelem *
bb_queue_retrieve_PRIO(struct bb_queue *q)
{
    struct bb_qelem *minTNode;
    struct bb_qelem *tracer;

    if (q->count == 0)
        return NULL;

    if( q->count == 1){
        return q->head;
    }
    minTNode = q->head; ///points to the head node, starting from there
    //tracer = q->head;

    ///this loop is for finding the node with smallest burst length
    for(tracer = q->head; tracer != NULL; tracer = tracer->next){
        if( tracer->threadIndex < minTNode->threadIndex){
            minTNode = tracer;
        }
    }
    if( minTNode == q->head){
        q->head = q->head->next; //move to the next one
        q->count--;
        return minTNode;

    }
    if( minTNode == q->tail ){
        // for(tracer = q->head; tracer->next != q->tail; tracer = tracer->next){
        //just to reach that point
        //}
        tracer = q->head;
        while( tracer->next != q->tail){
            //just traverse
            tracer = tracer->next;
        }
        tracer->next = NULL;
        q->tail = tracer; //new tail
        q->count--;
        return minTNode;
    }
    tracer = q->head;
    while(tracer->next != minTNode){
        tracer = tracer->next;
    }
    // for(tracer = q->head; tracer->next != minimumNode; tracer = tracer->next){
    //this way I'LL get the node before minimum node
    //}
    tracer->next = minTNode->next; //connecting to the node after min node
    q->count--;

    return minTNode;
}
struct bb_qelem *
bb_queue_retrieve_VRUNTIME(struct bb_queue *q)
{
    struct bb_qelem *minVNode;
    struct bb_qelem *tracer;

    if (q->count == 0)
        return NULL;

    if( q->count == 1){
        int index = q->head->threadIndex;
        int t = q->head->length;
        printf("thread %d is selected with vruntime  = %f\n", index, vruntime[index]);
        vruntime[index] = vruntime[index] + t * (0.7 + (0.3 * index));
        //printf("Vruntime of thread %d is selected and is = %f \n", index, vruntime[index]);
        return q->head;
    }
    minVNode = q->head; ///points to the head node, starting from there
    //tracer = q->head;

    ///this loop is for finding the node with smallest vruntime
    for(tracer = q->head; tracer != NULL; tracer = tracer->next){
        if( vruntime[tracer->threadIndex] < vruntime[minVNode->threadIndex]){
            minVNode = tracer;
        }
    }
    if( minVNode == q->head){
        q->head = q->head->next; //move to the next one
        q->count--;
        int index = minVNode->threadIndex;
        int t = minVNode->length;
        printf("thread %d is selected with vruntime  = %f\n", index, vruntime[index]);
        vruntime[index] = vruntime[index] + t * (0.7 + (0.3 * index));
        return minVNode;

    }
    if( minVNode == q->tail ){
        // for(tracer = q->head; tracer->next != q->tail; tracer = tracer->next){
        //just to reach that point
        //}
        tracer = q->head;
        while( tracer->next != q->tail){
            //just traverse
            tracer = tracer->next;
        }
        tracer->next = NULL;
        q->tail = tracer; //new tail
        q->count--;
        int index = minVNode->threadIndex;
        int t = minVNode->length;
        printf("thread %d is selected with vruntime  = %f\n", index, vruntime[index]);
        vruntime[index] = vruntime[index] + t * (0.7 + (0.3 * index));
        //printf("Vruntime of thread %d is selected and is = %f\n", index, vruntime[index]);
        return minVNode;
    }
    tracer = q->head;
    while(tracer->next != minVNode){
        tracer = tracer->next;
    }
    // for(tracer = q->head; tracer->next != minimumNode; tracer = tracer->next){
    //this way I'LL get the node before minimum node
    //}
    tracer->next = minVNode->next; //connecting to the node after minV node
    q->count--;
    int index = minVNode->threadIndex;
    int t = minVNode->length;
    printf("thread %d is selected with vruntime  = %f\n", index, vruntime[index]);
    vruntime[index] = vruntime[index] + t * (0.7 + (0.3 * index));
    // printf("Vruntime of thread %d is selected and is = %f\n", index, vruntime[index]);
    return minVNode;
}


/*******************************************************
 Below is shared object and its two operations.
 *******************************************************/

// This is the shared object
struct bounded_buffer {
    struct bb_queue *q;               /* bounded buffer queue  */
    pthread_mutex_t th_mutex_queue;   /* mutex  to protect queue */
    pthread_cond_t  th_cond_hasspace;  /* will cause producer to wait */
    pthread_cond_t  th_cond_hasitem;   /* will cause consumer to wait */
};
// It will have two operations: bb_add and bb_rem


void
bb_add_file(struct  bounded_buffer* bbp, struct bb_qelem *qep)
{
    pthread_mutex_lock(&bbp->th_mutex_queue);

    /* critical section begin */
    //while (bbp->q->count == BcountFromFile) {
       //pthread_cond_wait(&bbp->th_cond_hasspace,
                //          &bbp->th_mutex_queue);
  //  }

    bb_queue_insert(bbp->q, qep); //

    if (TRACE) {
        printf ("producer insert item = %f with Tindex as %d and interarrival time: %f\n",qep->length, qep->threadIndex, qep->time);
        fflush (stdout);
    }

    if (bbp->q->count == 1)
        pthread_cond_signal(&bbp->th_cond_hasitem);

    /* critical section end */
    pthread_mutex_unlock(&bbp->th_mutex_queue);
}


struct bb_qelem *
bb_rem_file(struct bounded_buffer *bbp)
{
    struct bb_qelem *qe = NULL;

    pthread_mutex_lock(&bbp->th_mutex_queue);

    /* critical section begin */

    while (bbp->q->count == 0) {
        pthread_cond_wait(&bbp->th_cond_hasitem,
                          &bbp->th_mutex_queue);
    }
    if( strcmp(algGiven, FCFS) == 0){
       // printf("it's FCFS \n");
        qe = bb_queue_retrieve(bbp->q);
    }
    else if( strcmp(algGiven, PRIO) == 0){
        // printf("it's FCFS \n");
        qe = bb_queue_retrieve_PRIO(bbp->q);
    }
    else if( strcmp(algGiven, SJF) == 0){
        // printf("it's FCFS \n");
        qe = bb_queue_retrieve_SJF(bbp->q);
    }
    else if( strcmp(algGiven, VRUNTIME) == 0){
        // printf("it's FCFS \n");
        qe = bb_queue_retrieve_VRUNTIME(bbp->q);
    }


    if (qe == NULL) {
        printf("can not retrieve; should not happen\n");
        exit(1);
    }

    if (TRACE) {
        printf ("consumer retrieved item = %f with thread index %d\n", qe->length, qe->threadIndex);
        fflush (stdout);
    }

   // if (bbp->q->count == ((BcountFromFile) - 1)) {
    // pthread_cond_signal(&bbp->th_cond_hasspace);
   // }

    /* critical section end */

    pthread_mutex_unlock(&bbp->th_mutex_queue);
    return (qe);
}


/*******************************************************
          end shared object data and functions
 *******************************************************/




/*********** GLOBAL VARIABLE ******************************************/
char infilename[MAXFILENAME];
char outfilename[MAXFILENAME];

struct bounded_buffer *bbuffer;  /* bounded buffer pointer */
void *
producer_file (void * arg)
{
    //vruntime[(long int) arg] = 0; //the vruntime of the thread
    struct bb_qelem *qe;

    int index;  // thread index
    index = (int) arg;

    char buffer[100];
    //int n = i;
    //char prefix[20] = "infile";
    sprintf(buffer, "%s-%d.txt", infilename, index);
    FILE *fp;
    fp = fopen(buffer, "r");

    double burstTime;
    double burstLength;
    int i = 1;
    struct timeval current_time;

    while (fscanf (fp, "%lf %lf", &burstLength, &burstTime) == 2)  {
        /*  insert item into buffer */

        usleep((burstTime) * 1000); //since its in miliseconds


        gettimeofday(&current_time, NULL);
        double current_time_ms = (current_time.tv_sec) * 1000 + current_time.tv_sec;

        qe = (struct bb_qelem *) malloc (sizeof (struct bb_qelem)); //burst generated and added
        if (qe == NULL) {
            perror ("malloc failed\n");
            exit (1);
        }
        qe->next = NULL;
        qe->threadIndex = index;
        qe->burstIndex = i++;
        qe->length = burstLength;
        qe->time = burstTime;
        qe->wallClockTime = current_time_ms; //now we have the wallclock time

        bb_add_file(bbuffer, qe); // one thread at a time

    }

    printf ("producer %ld terminating\n", (long int) arg);  //fflush (stdout);
    pthread_exit (NULL);
}
void *
consumer_file(void * arg)
{
    struct timeval start, end;
    struct timeval current_time_2;

    gettimeofday(&start, NULL);
    int traverse = 0;

    struct bb_qelem *qe;

    while( traverse != BcountFromFile){

        qe = bb_rem_file(bbuffer); // one thread at a time
        if (qe->next != NULL) {
            gettimeofday(&current_time_2, NULL);
            double finish_time_ms = (current_time_2.tv_sec) * 1000 + current_time_2.tv_sec;
            double wait_time = finish_time_ms - qe->wallClockTime;
            waitingtime[qe->threadIndex] =  waitingtime[qe->threadIndex] + wait_time;

            usleep(qe->length * 1000);
            qe->next = NULL; ////added
            free (qe);
        }
        else{
            gettimeofday(&current_time_2, NULL);
            double finish_time_ms = (current_time_2.tv_sec) * 1000 + current_time_2.tv_sec;
            double wait_time = finish_time_ms - qe->wallClockTime;
            waitingtime[qe->threadIndex] =  waitingtime[qe->threadIndex] + wait_time;

            usleep(qe->length * 1000);
           // free (qe); // deallocating memory that was allocated
        }
        traverse++;
    }
    gettimeofday(&end, NULL);
    printf(" \n%ld microseconds \n", ((end.tv_sec * 1000000 + end.tv_usec)
                                      - (start.tv_sec * 1000000 + start.tv_usec)));

    printf ("consumer terminating\n");
    fflush (stdout);
    pthread_exit (NULL);
}




int main(int argc, char **argv) {


    pthread_t constid;
    pthread_t prodtid[10];
    int i, ret;

    for (int i = 1; i <= 10; i++) {
        vruntime[i] = 0;
    } //make the vruntime array
    for (int i = 1; i <= 10; i++) {
        waitingtime[i] = 0;
    } //make the vruntime array


    if(argc == 5) {
        N = atoi(argv[1]);
        // char * alg = argv[4];
        strcpy(algGiven, argv[2]);


        strcpy(infilename, argv[4]);
        BcountFromFile = 0;

        for (i = 1; i <= N; i++) {
            int count_lines = 0;
            char buffer[134];
            int n = i;
            //char prefix[20] = "infile";
            sprintf(buffer, "%s-%d.txt", infilename, n);
            //printf("%s\n", buffer);
            printf("%s \n", buffer);
            FILE *checkLines = fopen(buffer, "r");
            int x;
            int y;
            while (fscanf(checkLines, "%d %d", &x, &y) == 2) {
                count_lines++;
                BcountFromFile++;
            }
            printf("There are %d bursts in thread %d\n", count_lines, i);
        }
        printf("Total Number of Bursts from all Threads = %d \n", BcountFromFile);


        bbuffer = (struct bounded_buffer *) malloc(sizeof(struct bounded_buffer));
        bbuffer->q = (struct bb_queue *) malloc(sizeof(struct bb_queue));
        bb_queue_init(bbuffer->q);
        pthread_mutex_init(&bbuffer->th_mutex_queue, NULL);
        pthread_cond_init(&bbuffer->th_cond_hasspace, NULL);
        pthread_cond_init(&bbuffer->th_cond_hasitem, NULL);

        for (i = 1; i <= N; ++i) {
            ret = pthread_create(&prodtid[i], NULL, producer_file,
                                 (void *) (long) i);
            if (ret < 0) {
                perror("thread create failed\n");
                exit(1);
            }
        }

        if (ret != 0) {
            perror("thread create failed\n");
            exit(1);
        }

        ret = pthread_create(&constid, NULL,
                             consumer_file, NULL);
        if (ret != 0) {
            perror("thread create failed\n");
            exit(1);
        }

        /* wait for threads to terminate */
        for (i = 0; i < N; ++i)
            pthread_join(prodtid[i], NULL);

        pthread_join(constid, NULL);

        /* destroy buffer and mutex/condition variables */
        free(bbuffer->q);
        free(bbuffer);

        pthread_mutex_destroy(&bbuffer->th_mutex_queue);
        pthread_cond_destroy(&bbuffer->th_cond_hasspace);
        pthread_cond_destroy(&bbuffer->th_cond_hasitem);

        for (int j = 1; j <= N; j++) {
            printf(" waiting time of thread %d is %f \n", j, waitingtime[j]);
        }

        printf("closing...\n");
        return 0;
    }
    printf("wrong commands \n");
    return 0;

}