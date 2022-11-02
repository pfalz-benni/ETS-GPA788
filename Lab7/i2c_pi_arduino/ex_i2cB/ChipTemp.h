/*  ChipTemp.h
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
#ifndef ChipTemp_H
#define ChipTemp_H

// must be >= 1000, else the gain setting has no effect
static const int16_t samples = 1000;

/* --------------------------------------------------
   Classe ChipTemp
   Lire samples échantillons de la température interne 
   de l'ATMega328P et faire la moyenne puis convertir
   la valeur en Celsius et Fahrenheit.
   -------------------------------------------------- */   
class ChipTemp 
{
public:
  // Constructeur de la classe
  ChipTemp(float g, float d) {
    offset = d; gain = g;
    offsetFactor = long(offset * samples);
    divideFactor = gain * samples / 10.0;
  }
  // Services publics
  float deciCelsius(); 
  float celsius(); 
  float deciFahrenheit();
  float fahrenheit();

  float getOffset() { return offset; }
  float getGain() { return gain; }
   
private:
  // Initialiser l'ADC 
  inline void initialize(); 
  // Lire la valeur de l'ADC
  inline int16_t readAdc();
  // Paramètres pour la conversion valeur -> température
  float offset, gain, divideFactor;
  int32_t offsetFactor;
};

#endif

