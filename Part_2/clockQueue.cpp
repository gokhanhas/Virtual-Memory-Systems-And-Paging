/*
##########################################
#        Gokhan Has - 161044067          #
#       CSE 312 - Final Project          #
# Lecturer : Prof. Dr. Yusuf Sinan Akgul #
##########################################
*/

#include "clockQueue.h"

void initializeClockQueue(ClockQueue* queue) {
    queue->size = 0;
    queue->first = NULL;
    queue->last = NULL;
    queue->current = NULL;
}

void pushClock(ClockQueue* queue, int element, int refer) {
    
    NodeX* temp = (NodeX*)malloc(sizeof(NodeX) * 1);
    temp->value = element;
    temp->next = NULL;
    temp->referenced = refer;
    temp->aging = 0;

    if(queue->size == 0) {
        queue->first = queue->last = temp;
        queue->current = queue->first;
    }
    else {
        queue->last->next = temp;
        queue->last = temp;
        queue->last->next = queue->first;
    }
    queue->size = queue->size + 1;
}


int frontClock(ClockQueue* queue) {
    return queue->first->value;
}

int getWSClock(ClockQueue* queue, int index, int *count) {
    int val = queue->current->referenced;
    while(true) {
        srand(time(NULL));
        *count = rand() % 10;
        if(val == 0) {
            return queue->current->value;
            break;
        }
        queue->current = queue->current->next;
    }
    return -1;
}

void freeClockQueue(ClockQueue* queue) {
    while(queue->first != NULL) {
        NodeX* temp = queue->first;
        queue->first = queue->first->next;
        free(temp);
    }
    if(queue->first != NULL)
        free(queue->first);
    free(queue);
}

int getx() {
    srand(time(NULL));
    return rand() % 12;
}