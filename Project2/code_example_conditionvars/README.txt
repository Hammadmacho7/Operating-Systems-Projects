
We have two programs here.


1) mutexcond.c is a program that is creating 2 threads. One thread is acting as a producer, and the other thread is acting as a consumer. Pthreads mutex and condition variables are used for synchronization of the producer and consumer threads.



2) bcast.c is a program that is using more than 2 threads. There are TCOUNT producer threads and 1 consumer thread. Again Pthreads mutex and condition variables are used. But instead of signal() operation, broadcast() operation is used to wake up multiple threads.


We also have a simple gen.c file that is generating integers into a file. The file will be input file for the mutexcond program. 