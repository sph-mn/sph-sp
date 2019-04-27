/* quicksort implementation based on the public domain implementation from http://alienryderflex.com/quicksort/ */
#define quicksort_t int
#define quicksort_count_t int
#define quicksort_max_levels 1000
uint8_t quicksort(quicksort_t a, quicksort_count_t a_len) {
  quicksort_t piv;
  quicksort_t beg[quicksort_max_levels];
  quicksort_t end[quicksort_max_levels];
  quicksort_t i;
  quicksort_t l;
  quicksort_t r;
  i = 0;
  beg[0] = 0;
  end[0] = a_len;
  while ((i >= 0)) {
    l = beg[i];
    r = (end[i] - 1);
    if (l >= r) {
      i = (i - 1);
      continue;
    };
    piv = a[l];
    if (i == (quicksort_max_levels - 1)) {
      return (1);
    };
    while ((l < r)) {
      while (((a[r] >= piv) && (l < r))) {
        r = (r - 1);
        if (l < r) {
          l = (1 + l);
          a[l] = a[r];
        };
      };
      while (((a[l] <= piv) && (l < r))) {
        l = (1 + l);
        if (l < r) {
          r = (r - 1);
          a[r] = a[l];
        };
      };
    };
    a[l] = piv;
    beg[(1 + i)] = (1 + l);
    end[(1 + i)] = end[i];
    i = (1 + i);
    end[i] = l;
  };
  return (0);
};