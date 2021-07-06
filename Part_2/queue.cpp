/*
##########################################
#        Gokhan Has - 161044067          #
#       CSE 312 - Final Project          #
# Lecturer : Prof. Dr. Yusuf Sinan Akgul #
##########################################
*/

#include "queue.h"

void initializeQueue(Queue* queue) {
    queue->size = 0;
    queue->first = NULL;
    queue->last = NULL;
}

void push(Queue* queue, int element, int refer, int modi) {
    
    Node* temp = (Node*)malloc(sizeof(Node) * 1);
    temp->value = element;
    temp->next = NULL;
    temp->modified = modi;
    temp->referenced = refer;

    if(queue->size == 0) {
        queue->first = queue->last = temp;
    }
    else {
        queue->last->next = temp;
        queue->last = temp;
    }
    queue->size = queue->size + 1;
}

int pop(Queue* queue){
    if(queue->size == 0)
        return -1;
    int returnVal = queue->first->value;
    Node * temp = queue->first;
    queue->first = queue->first->next;
    queue->size = queue->size - 1;
    free(temp);
    return returnVal; 
}


int front(Queue* queue) {
    return queue->first->value;
}

int findSC(Queue* queue) {
    int i = 0;
    int size = queue->size;
    Node* temp = queue->first;
    while(i < size && temp != NULL) {
        if(temp->referenced == 0) {
            return i;
        }
        temp = temp->next;
        i++;
    }
    return 0;
}

void freeQueue(Queue* queue) {
    while(queue->first != NULL) {
        Node* temp = queue->first;
        queue->first = queue->first->next;
        free(temp);
    }
    if(queue->first != NULL)
        free(queue->first);
    free(queue);
}
