/*
 * wait.c
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

#include "wait.h"

void waitUntil(uint32_t w)
{
    uint32_t t{millis()};
    // Attendre w millisecondes
    while (millis() < t + w)
    {
    }
}