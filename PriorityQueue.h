
#ifndef PriorityQueue_h
#define PriorityQueue_h

#include <stdbool.h>

struct MaxHeap {
    int* indices;
    size_t count;
    size_t capacity;
};

typedef struct MaxHeap MaxHeap;

// create a MinHeap with the given capacity
MinHeap* createMaxHeap(size_t capacity);

// extract the maximum value in the heap and place it after (at elements[count])
int extractIndexForMax(MaxHeap* heap, double* array);

// insert a new value in the heap, return false if capacity is too low
void insertIndex(int index, MaxHeap* heap, double* array);

// return whether there is still place in the heap
bool isFull(MaxHeap* heap);

// destroy a heap
void destroyMaxHeap(MaxHeap* heap);

#endif /* PriorityQueue_h */
