// Online C compiler to run C program online
// Online C++ compiler to run C++ program online

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

int getmseconds( const struct timeval *start, const struct timeval *end){
  long microseconds = end->tv_usec - start->tv_usec; 
  long seconds = (end->tv_sec - start->tv_sec) * 1000000;
  return (microseconds + seconds);
}


struct node {
   int data;
   struct node *next;
};

struct node *head = NULL;
struct node *tail = NULL;

void addNode(int item)
{
  struct node *temp = (struct node*)malloc(sizeof(struct node));
  temp->data = item;
  temp->next = NULL;

  if(head == NULL)
  {
     head = temp;
     tail = temp;
  }
  else
  {
    tail->next = temp;
    tail = tail->next; //pointing to the last node entered
    }
}

int main()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for ( int i = 0; i < 10000; i++)
    {
        addNode(rand());
    }
    
    gettimeofday(&end, NULL);
    printf("%d microseconds\n", getmseconds(&start,&end));

    return 0;
}




