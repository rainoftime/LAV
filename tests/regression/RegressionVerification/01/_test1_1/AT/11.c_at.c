
#include <stdio.h>

int check_main(long a) {

  long int c1, c2, c3, c4; // posto je broj cetvorocifren, opseg je veci

  printf("unesi broj a\n");
  // scanf ("%ld", &a);
  //izdvajanje cifara
  c1 = a % 10;         //jedinica
  c2 = (a / 10) % 10;  //desetica
  c3 = (a / 100) % 10; //stotica
  c4 = a / 1000;       //hiljada

  if (a < 1000 || a > 9999 || a < 0) {
    printf("Rezultat je : -1\n");
    return -1;
  } //broj nije cetvorocifren
      else if (c4 > c3 && c3 > c2 && c2 > c1) {
    {
      printf("rezultat je : 2 \n");
      return 2;
    } // cifre su opadajuce, cifra jedinice je najmanja
  } else if (c4 < c3 && c3 < c2 && c2 < c1) {
    {
      printf("Rezultat je : 1\n");
      return 1;
    } // cifre su rastuce, cifra jedinice je najveca
  } else {
    printf("Rezultat je : 3\n");
    return 3;
  } // cifre su neuredjene

  // return 0;

}

#include "../check.h"
