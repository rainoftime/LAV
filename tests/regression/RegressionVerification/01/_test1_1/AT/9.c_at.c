
#include <stdio.h>
#include <math.h>

int check_main(int x) {
  int j, d, s, h;
  printf("Unesi cetvorocifren broj\n");
  // scanf ("%d", &x);

  if (x < 1000 || x > 9999) { /* uslov da je broj cetvorocifren */
    {
      printf("-1\n");
      return -1;
    }
    // return 0;
  }

  j = ((x % 1000) % 100) % 10; /* izdvajamo cifru jedinica */
  d = ((x % 1000) % 100) / 10; /* izdvajamo cifru desetica */
  s = (x % 1000) / 100;        /* izdvajamo cifru stotina */
  h = x / 1000;                /* izdvajamo cifru hiljada */

  if (h < s && s < d && d < j) { /* da li je cifra hiljda manja od stotina,
stotina od desetica, desetica od jedinica (da li je niz rastuci ) */
    printf("1\n");
    return 1;
  } else if (h > s && s > d &&
             d > j) { /* da li je cifra hiljda veca od stotina, stotina od
desetica, desetica od jedinica (da li je niz opadajuci ) */
    printf("2");
    return 2;
  } else {
    printf("3\n");
    return 3;
  } /* inace: nije uredjen */

  // return 0;

}

#include "../check.h"
