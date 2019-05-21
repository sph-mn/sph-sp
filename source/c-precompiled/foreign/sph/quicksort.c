/* a generic quicksort implementation.
based on the public domain implementation from http://alienryderflex.com/quicksort/ */
/** less should return true if the first argument is < than the second.
   swap should exchange the values of the two arguments it receives */
void quicksort(uint8_t (*less_p)(void*, size_t, size_t), void (*swap)(void*, size_t, size_t), void* array, size_t size, size_t offset) {
  size_t pivot;
  size_t a;
  size_t i;
  size_t j;
  if (size < 2) {
    return;
  };
  pivot = (offset + (size / 2));
  i = 0;
  j = (size - 1);
  while (1) {
    while (less_p(array, (offset + i), pivot)) {
      i = (1 + i);
    };
    while (less_p(array, pivot, (offset + j))) {
      j = (j - 1);
    };
    if (i >= j) {
      break;
    };
    swap(array, (offset + i), (offset + j));
    i = (1 + i);
    j = (j - 1);
  };
  quicksort(less_p, swap, array, i, offset);
  quicksort(less_p, swap, array, (size - i), (offset + i));
};