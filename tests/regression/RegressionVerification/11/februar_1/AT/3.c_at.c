
#include <stdio.h>

int check_main(float a11, float a12, float a21, float a22) {
  //deklarisanje  elemenata matrice
  float det;
  // scanf ("%f%f%f%f", &a11, &a12, &a21, &a22);
  det = (a11 * a22) - (a21 * a12); // izracunavanje determinante
  {
    printf("%.4lf\n", det);
    return det;
  }
  // return 0;
}

#include "../check.h"