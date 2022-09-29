#include "wait.h"

void waitUntil(uint32_t w)
{
    uint32_t t{millis()};
    // Attendre w millisecondes
    while (millis() < t + w)
    {
    }
}