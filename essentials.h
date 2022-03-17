#ifndef ESSENTIALS_H
#define ESSENTIALS_H

#include <stdlib.h>
#include <string.h>
#define HEAPSIZE 100

typedef struct node {
  void *data;
  struct node *next;
} Node;

typedef struct list {
  Node *first;
  Node *last;
  int count;
} List;

typedef struct stack {
  Node *last;
  int count;
} Stack;

typedef struct heapEntry {
  unsigned int value;
  void *data;
} HeapEntry;

typedef struct minHeap {
  HeapEntry **arr;
  int size;
  int count;
} MinHeap;

typedef struct hmapNode {
  unsigned int key;
  void *data;
  struct hmapNode *next;
} HMapNode;

typedef struct hashmap {
  HMapNode **table;
  int size;
  int count;
} HashMap;

Node *node_create(void *data);

List *list_create();

void list_append(List *list, void *data);
void *list_get_first(List *list);
void list_free(List *list);

HeapEntry *heap_entry_create(unsigned int value, void *data);
MinHeap *minheap_create();

void minheap_free(MinHeap *heap);
void minheap_add(MinHeap *heap, void *item, unsigned int value);
void *minheap_remove_min(MinHeap *heap);
void **increase_size_arr(void **arr, int size);

Stack *stack_create();

void stack_push(Stack *stack, void *data);
void *stack_pop(Stack *stack);
void stack_free(Stack *stack);

HMapNode *hmapnode_create(unsigned int key, void *data);
HashMap *hashmap_create(int size);

// returns 1 if it was successfully added, 0 if already exists,
int hashmap_insert(HashMap *hmap, unsigned int key, void *data);
void *hashmap_remove(HashMap *hmap, unsigned int key);
void hashmap_free(HashMap *hmap);

#endif // !ESSENTIALS_H
