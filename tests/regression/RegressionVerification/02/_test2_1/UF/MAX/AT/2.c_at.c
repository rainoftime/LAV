float __UF__4_1(float, float *, int, int);
float __UF__4_0(float, float *, int, int);

#include <stdio.h>
#define MAX_SIZE 100
float max_broj(float a[], int n) { /*funkcija koja racuna maximum datog niza */
  int i;
  float max;
  max = a[0];
  i = 1;
  if (i < n) {
    if (max < a[i])
      max = a[i];
    ;
    i++;
    max = __UF__4_0(max, a, n, i);
  }

  return max;
}

float aritmeticka_sredina(float a[], int n) {
  /*funkcija koja racuna aritmeticku sredinu niza */
  int i;
  float suma = 0;
  i = 0;
  if (i < n) {
    suma += a[i];
    ;
    i++;
    suma = __UF__4_1(suma, a, n, i);
  }

  return suma / n;
}

#if 0
int main() {
  int n, i;
  float a[MAX_SIZE];
  printf("Uneti dimenziju niza: ");
  scanf("%d", &n);
  if ((n <= 0) || (n > MAX_SIZE)) { /*provera korektnosti unetih podataka */
    printf("Izlaz:\n-1\n");         /*nekorektan unos */
    return 0;
  } else {
    printf("Unesi niz: "); /*korektan unos */
    for (i = 0; i < n; i++) {
      scanf("%f", &a[i]);
    }
    printf("Izlaz:\n");
    printf("%f \n", max_broj(a, n));
    printf("%f \n", aritmeticka_sredina(a, n));
    if (max_broj(a, n) > (aritmeticka_sredina(a, n) * 2))
      printf("1\n");
    else
      printf("0\n");
  }
  return 0;
}
#endif
#include "../check.h"
