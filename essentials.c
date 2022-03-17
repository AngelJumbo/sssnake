#include "essentials.h"

Node *node_create(void *data) {
  Node *tmp = (Node *)malloc(sizeof(Node));
  tmp->data = data;
  tmp->next = NULL;
  return tmp;
}

//----------list-------

List *list_create() {
  List *tmp = (List *)malloc(sizeof(List));
  tmp->first = NULL;
  tmp->last = NULL;
  tmp->count = 0;
  return tmp;
}
void list_append(List *list, void *data) {
  if (list->count == 0) {
    list->last = node_create(data);
    list->first = list->last;
  } else {
    list->last->next = node_create(data);
    list->last = list->last->next;
  }
  list->count++;
}
void *list_get_first(List *list) {
  void *data = list->first->data;
  Node *next = list->first->next;
  free(list->first);
  list->first = next;
  list->count--;
  return data;
}
void list_free(List *list) {
  Node *node = list->first;
  Node *nextNode = NULL;
  while (node != NULL) {
    nextNode = node->next;
    free(node->data);
    free(node);
    node = nextNode;
  }
  free(list);
}

//----------stack-------
Stack *stack_create() {
  Stack *tmp = (Stack *)malloc(sizeof(Stack));
  tmp->last = NULL;
  tmp->count = 0;
  return tmp;
}

void stack_push(Stack *stack, void *data) {
  Node *node = node_create(data);
  node->next = stack->last;
  stack->last = node;
  stack->count++;
}
void *stack_pop(Stack *stack) {
  if (stack->last != NULL) {
    void *data = stack->last->data;
    Node *node = stack->last;
    stack->last = node->next;
    stack->count--;
    free(node);
    return data;
  } else {
    return NULL;
  }
}

void stack_free(Stack *stack) {

  void *tmp = stack_pop(stack);

  while (tmp != NULL) {
    free(tmp);
    tmp = stack_pop(stack);
  }
  free(stack);
}

//--------minheap-----------

HeapEntry *heap_entry_create(unsigned int value, void *data) {
  HeapEntry *entry = (HeapEntry *)malloc(sizeof(HeapEntry));
  entry->value = value;
  entry->data = data;
  return entry;
}

MinHeap *minheap_create() {
  MinHeap *tmp = (MinHeap *)malloc(sizeof(MinHeap));
  tmp->arr = (HeapEntry **)(malloc(sizeof(HeapEntry *) * HEAPSIZE));
  for (int i = 0; i < HEAPSIZE; i++) {
    tmp->arr[i] = NULL;
  }

  tmp->size = HEAPSIZE;
  tmp->count = 0;
  return tmp;
}

static void *double_array_size(void **arr, unsigned int size) {
  void **tmp = malloc(size * 2 * sizeof(void *));
  memcpy(tmp, arr, size * sizeof(void *));
  free(arr);
  return tmp;
}

static void minheap_swap(MinHeap *minheap, unsigned int x, unsigned int y) {
  HeapEntry *tmp = minheap->arr[x];
  minheap->arr[x] = minheap->arr[y];
  minheap->arr[y] = tmp;
}

static void minheap_bubble_up(MinHeap *heap, unsigned int x) {
  HeapEntry *e = heap->arr[x];
  unsigned int parentIndex = (x - 1) / 2;
  HeapEntry *parent = heap->arr[parentIndex];
  if (e->value < parent->value) {
    minheap_swap(heap, x, parentIndex);
    if (parentIndex > 0) {
      minheap_bubble_up(heap, parentIndex);
    }
  }
}

static void minheap_bubble_down(MinHeap *heap, unsigned int x) {
  HeapEntry *entry = heap->arr[x];
  unsigned int left_child_index = (x * 2) + 1;
  unsigned int right_child_index = left_child_index + 1;
  unsigned int swapped = 0;
  unsigned int swapped_index;
  if ((right_child_index < heap->count) &&
      (heap->arr[right_child_index]->value < heap->arr[left_child_index]->value)

      && entry->value > heap->arr[right_child_index]->value) {
    minheap_swap(heap, x, right_child_index);
    swapped = 1;
    swapped_index = right_child_index;
  } else if (entry->value > (heap->arr[left_child_index])->value) {
    minheap_swap(heap, x, left_child_index);
    swapped = 1;
    swapped_index = left_child_index;
  }
  if (swapped && (swapped_index * 2) + 1 < (heap->count) - 1) {
    minheap_bubble_down(heap, swapped_index);
  }
}

void minheap_add(MinHeap *heap, void *item, unsigned int value) {
  HeapEntry *e = heap_entry_create(value, item);
  if (heap->count + 1 > heap->size) {
    heap->arr = (HeapEntry **)double_array_size((void **)heap->arr, heap->size);
    heap->size = 2 * heap->size;
  }

  heap->arr[heap->count] = e;
  heap->count++;
  if (heap->count > 1) {
    minheap_bubble_up(heap, heap->count - 1);
  }
}
void *minheap_remove_min(MinHeap *heap) {
  void *item = NULL;
  if (heap->count > 1) {
    minheap_swap(heap, 0, heap->count - 1);
  }
  if (heap->count > 0) {
    HeapEntry *e = heap->arr[heap->count - 1];
    item = e->data;
    heap->count--;
    free(e);
  }
  if (heap->count > 1) {
    minheap_bubble_down(heap, 0);
  }
  return item;
}

void minheap_free(MinHeap *heap) {
  for (int i = 0; i < heap->count; i++) {
    free(heap->arr[i]->data);
    free(heap->arr[i]);
  }
  free(heap->arr);
  free(heap);
}
//----------hashmap------------
HMapNode *hmapnode_create(unsigned int key, void *data) {
  HMapNode *tmp = (HMapNode *)malloc(sizeof(HMapNode));
  tmp->data = data;
  tmp->key = key;
  return tmp;
}
HashMap *hashmap_create(int size) {
  HashMap *tmp = (HashMap *)malloc(sizeof(HashMap));
  tmp->size = size;
  tmp->table = (HMapNode **)malloc(sizeof(HMapNode *) * size);
  tmp->count = 0;
  for (int i = 0; i < size; i++)
    tmp->table[i] = NULL;
  return tmp;
}

int hashmap_insert(HashMap *hmap, unsigned int key, void *data) {

  int pos = (key) % hmap->size;
  if (hmap->table[pos] == NULL) {
    hmap->table[pos] = hmapnode_create(key, data);
  } else {
    HMapNode *bucket = hmap->table[pos];
    while (bucket->next != NULL) {
      if (bucket->key == key) {
        return 0;
      }
      bucket = bucket->next;
    }
    bucket->next = hmapnode_create(key, data);
  }
  hmap->count++;
  return 1;
}
void *hashmap_remove(HashMap *hmap, unsigned int key) {

  int pos = (key) % hmap->size;
  if (hmap->table[pos] == NULL) {
    return NULL;
  } else if (hmap->table[pos]->key == key) {
    HMapNode *bucket = hmap->table[pos];
    hmap->table[pos] = bucket->next;
    void *data = bucket->data;
    free(bucket);
    hmap->count--;
    return data;
  } else {
    HMapNode *bucket = hmap->table[pos]->next;
    HMapNode *prevBucket = hmap->table[pos];
    while (bucket != NULL) {
      if (bucket->key == key) {
        prevBucket->next = bucket->next;
        void *data = bucket->data;
        free(bucket);
        hmap->count--;
        return data;
      }
      bucket = bucket->next;
      prevBucket = prevBucket->next;
    }
  }
  return NULL;
}

void hashmap_free(HashMap *hmap) {
  for (int i = 0; i < hmap->size; i++) {
    HMapNode *bucket = hmap->table[i];
    if (bucket == NULL)
      continue;
    HMapNode *nextbucket = bucket->next;
    while (bucket != NULL) {
      nextbucket = bucket->next;
      free(bucket);
      bucket = nextbucket;
    }

    hmap->table[i] = NULL;
  }
  free(hmap->table);
  free(hmap);
}
