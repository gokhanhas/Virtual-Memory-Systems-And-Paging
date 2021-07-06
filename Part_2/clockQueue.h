/*
##########################################
#        Gokhan Has - 161044067          #
#       CSE 312 - Final Project          #
# Lecturer : Prof. Dr. Yusuf Sinan Akgul #
##########################################
*/

#ifndef _CLOCKQUEUE_H_
#define _CLOCQUEUE_H_

#include <iostream>
#include <stdlib.h>

#include <iostream>
#include <stdlib.h>


typedef struct _nodex {
    int value;
    struct _nodex* next;
    int referenced;
    int aging;
} NodeX;

typedef struct _queueClock {
    int size;
    struct _nodex* current;
    struct _nodex* first;
    struct _nodex* last;
} ClockQueue;

// Queue is initialized according to the initial values.
void initializeClockQueue(ClockQueue* queue);

// Adding an element to the end of the queue. Since the last node is held, it is added in O (1) time.
void pushClock(ClockQueue* queue, int element, int refer);


int frontClock(ClockQueue* queue);
int getx();
int getWSClock(ClockQueue* queue, int index, int *count);

// Resources used for the queue are free.
void freeClockQueue(ClockQueue* queue);



#endif
