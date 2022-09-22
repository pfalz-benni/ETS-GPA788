/*  ChipTemp.cpp
 *  Cette classe a été conçue par Albert van Daln
 *  pour réaliser la lecture de la température
 *  interne du microcontrôleur ATMega328P.
 *  
 *  Code modifié par: T. Wong
 *  08-2018
 *  08-2020
 *  
 *  Le notice originale:
 *      ATmega328 temperature sensor interface 
 *      Rev 1.0 Albert van Dalen (www.avdweb.nl)
 *      Based on "InternalTemp"
 *      Requires 166 ... 204 bytes program memory
 *      Resolution 0.1 degree
 */
#include <Arduino.h>
#include "ChipTemp.h"


/* --------------------------------------------------
   Cette fonction membre initialise l'ADC du
   microcontrôleur.
   -------------------------------------------------- */
inline void ChipTemp::initialize() 
{ 
  ADMUX = 0xC8; // select reference, select temp sensor
  delay(10); // wait for the analog reference to stabilize
  readAdc(); // discard first sample (never hurts to be safe) 
}

/* --------------------------------------------------
   Lire la valeur de la tension aux bornes de la
   diode interne.
   -------------------------------------------------- */
inline int16_t ChipTemp::readAdc()
{ 
  ADCSRA |= _BV(ADSC); // start the conversion 
  while (bit_is_set(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes 
  return (ADCL | (ADCH << 8)); // combine bytes 
}

/* --------------------------------------------------
   Convertir la tension lue en deci-degrés Celsius.
   Les autres unités de température sont dérivées
   du déci-degrés Celsius.
   -------------------------------------------------- */
float ChipTemp::deciCelsius() 
{ 
  int32_t averageTemp = 0; 
  // Une bonne pratique d'initialiser à chaque fois car 
  // l'ADC peut être reprogrammé entre temps par d'autres
  // routines d'un programme.
  initialize();
  for (int i = 0; i < samples; i++) averageTemp += readAdc();
  averageTemp -= offsetFactor;
  return averageTemp / divideFactor; // return deci degree Celsius
}

float ChipTemp::celsius() 
{ 
  return deciCelsius() / 10;
}

float ChipTemp::deciFahrenheit() 
{ 
  return (9 * deciCelsius() + 1600) / 5;
}

float ChipTemp::fahrenheit() 
{ 
  return (9 * deciCelsius() + 1600) / 50; // do not use deciFahrenheit()/10;
}
