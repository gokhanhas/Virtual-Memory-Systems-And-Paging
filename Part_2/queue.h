/*
##########################################
#        Gokhan Has - 161044067          #
#       CSE 312 - Final Project          #
# Lecturer : Prof. Dr. Yusuf Sinan Akgul #
##########################################
*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <iostream>
#include <stdlib.h>


typedef struct _node {
    int value;
    struct _node* next;
    int referenced;
    int modified;
} Node;

typedef struct _queue {
    int size;
    struct _node* first;
    struct _node* last;
} Queue;

// Queue is initialized according to the initial values.
void initializeQueue(Queue* queue);

// Adding an element to the end of the queue. Since the last node is held, it is added in O (1) time.
void push(Queue* queue, int element, int refer, int modi);

//  Extracts an element from the queue. And it returns that element. Since the first node is held, 
// the element is removed in O (1) time.
int pop(Queue* queue);

int front(Queue* queue);

int findSC(Queue* queue);

// Resources used for the queue are free.
void freeQueue(Queue* queue);

#endif // !_QUEUE_H
