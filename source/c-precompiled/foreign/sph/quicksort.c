/* a generic quicksort implementation.
based on the public domain implementation from http://alienryderflex.com/quicksort/ */
/** less should return true if the first argument is < than the second.
   swap should exchange the values of the two arguments it receives */
void quicksort(uint8_t (*less_p)(void*, void*), void (*swap)(void*, void*), uint8_t element_size, void* array, size_t array_len) {
  char* pivot;
  char* a;
  size_t i;
  size_t j;
  if (array_len < 2) {
    return;
  };
  a = array;
  pivot = (a + (element_size * (array_len / 2)));
  i = 0;
  j = (array_len - 1);
  while (1) {
    while (less_p((a + (element_size * i)), pivot)) {
      i = (1 + i);
    };
    while (less_p(pivot, (a + (element_size * j)))) {
      j = (j - 1);
    };
    if (i >= j) {
      break;
    };
    swap((a + (element_size * i)), (a + (element_size * j)));
    i = (1 + i);
    j = (j - 1);
  };
  quicksort(less_p, swap, element_size, a, i);
  quicksort(less_p, swap, element_size, (a + (element_size * i)), (array_len - i));
};