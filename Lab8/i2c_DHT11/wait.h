/*
 * wait.h
 * Une fonction d'attents sans l'utilisation de delay() de Arduino qui
 * empêche des interruptions.
 * 
 * Auteurs : Philippe Boivin, Sandrine Bouchard, Alexandre Lins-d'Auteuil,
 * Benedikt Franz Witteler
 *
 * Dans le cadre du cours :
 * GPA788 - ETS
 * T. Wong
 * 08-2019
 * 08-2020
 */

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