#ifndef wait
#define wait

#include <Arduino.h>

/* ---------------------------------------------------------------
   Fonction pour créer un delai de w millisecondes
   La fonction delay() est utilisée dans bien des tutoriel pour
   créer un delai temporel. On peut aussi créer notre propre délai
   et utiliser une unité de temps à notre guise.
   --------------------------------------------------------------- */
void waitUntil(uint32_t w);

#endif