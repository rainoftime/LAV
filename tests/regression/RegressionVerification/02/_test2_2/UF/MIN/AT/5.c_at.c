int __UF__5_1(float, float *, int, int, int);
int __UF__5_0(float, float *, int, int, int);

#define __RET_UNDEF -1
#include <stdio.h>
#define MAX 100

#if 0
void ucitaj(float niz[], int n) {
  int i;
  printf("Uneti niz:");
  for (i = 0; i < n; i++)
    scanf("%f", &niz[i]);
}
#endif

int indeks_maksimuma(float niz[], int n) {
  int i, max_i = 0;
  float max;
  max = niz[0];
  i = 1;
  if (i < n) {
    if (niz[i] >= max) {
      max = niz[i];
      max_i = i;
    }
    i++;
    max_i = __UF__5_0(max, niz, n, i, max_i);
  }

  return max_i;
}

int indeks_minimuma(float niz[], int n) {
  int i, min_i = 0;
  float min;
  min = niz[0];
  i = 0;
  if (i < n) {
    if (niz[i] <= min) {
      min = niz[i];
      min_i = i;
    }
    i++;
    min_i = __UF__5_1(min, niz, n, i, min_i);
  }

  return min_i;
}

#if 0
int main() {
  float niz[MAX];
  int n, min, c, max;
  printf("Uneti dimenziju niza: ");
  scanf("%d", &n);
  if (n < 0 || n > MAX) {
    printf("Izlaz:\n-1");
    printf("\n");
    return 0;
  }
  ucitaj(niz, n);
  min = indeks_minimuma(niz, n);
  max = indeks_maksimuma(niz, n);
  if (min > max)
    c = 1;
  else
    c = 0;
  printf("Izlaz:\n%d\n%d\n%d", min, max, c);
  printf("\n");
  return 0;
}
#endif
#include "../check.h"
