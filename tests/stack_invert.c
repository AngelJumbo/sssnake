#include "../structs.h"
#include <stdio.h>

int main(void) {
  const int sizes[] = {0, 1, 3};
  for (int t = 0; t < 3; t++) {
    int size = sizes[t];
    Stack *stack = stack_create();
    for (int i = 0; i < size; i++) {
      int *value = malloc(sizeof(int));
      *value = i;
      stack_push(stack, value);
    }
    stack = stack_invert(stack);
    if (stack->count != size) {
      fprintf(stderr, "inverted size %d: got count %d\n", size, stack->count);
      stack_free(stack);
      return 1;
    }
    for (int i = 0; i < size; i++) {
      int *value = stack_pop(stack);
      if (!value || *value != i || stack->count != size - i - 1) return 2;
      free(value);
    }
    if (stack_pop(stack) != NULL || stack->count != 0) return 3;
    stack_free(stack);
  }
  puts("stack inversion: empty/single/multiple counts and order pass");
  return 0;
}
