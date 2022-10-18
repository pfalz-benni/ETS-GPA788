/*
 * dht11.ino
 * Utiliser la classe dhtlib_gpa788 pour mesurer la température et l'humidité
 * avec un sensor DHT11 connecté a un Arduino.
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

#include <LiquidCrystal.h>

#include "TimerOne.h"
#include "wait.h"
#include "DHTLib_GPA788.h"

/* CONSTANTES */

// Interruption à toutes les 2 secondes (pour la lecture)
const uint32_t TIMER_PERIOD{2000000};

// Relier le capteur à la broche #7
const int DHT11_PIN{7};

const int16_t NB_MSG_COUNT{2}; // TODO delete

/* VARIABLES GLOBALES */

// Créer un objet (global) de type LiquidCrystal.
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Créer un objet de type dht
dhtlib_gpa788 dht(DHT11_PIN);

// Création des variables afin de stocker et afficher les valeurs du DHT11
double dht_temp, dht_humi;

// Variable avec le code checked après une lecture
volatile DHTLIB_ErrorCode chk = DHTLIB_ErrorCode::DHTLIB_INVALID_VALUE; // TODO rename

/* FUNCTIONS */

/**
 * @brief Gestionnaire de l'interruption du timer qui lit la température
 * et l'humidité.
 */
void readTemperature()
{
   // Normalement, les interruptions sont bloquées lors du traitement des
   // interruptions. On doit les permettre ici car la fonction membre read11()
   // utilise delay() pour initier la communication série avec le capteur.

   interrupts(); // permettre les interruptions
   chk = dht.read11();
   noInterrupts(); // empêcher les interruptions
}

/**
 * @brief Fonction pour afficher la température et l'humidité du DHT11.
 */
void showTemp(int temp, int humi, LiquidCrystal &l)
{
   // On afffiche les valeurs de la température et
   // l'humidité sur le moniteur série
   Serial.print(F("Temperature : "));
   Serial.println(temp);
   Serial.print(F("Humidite : "));
   Serial.println(humi);

   // On afffiche les valeurs de la température et
   // l'humidité sur lcd
   l.setCursor(0, 0);
   l.print("Temp.: ");
   l.print(temp, 10);
   l.print((char)223);
   l.print("C");

   l.setCursor(0, 1);
   l.print("Hum.: ");
   l.print(humi, 10);
   l.print("%");
}

/**
 * @brief Fonction d'initialisation de l'Arduino.
 */
void setup()
{
   // Fixe le débit de communication en nombre de caracères par
   // seconde(baud) pour la communication série
   Serial.begin(115200);

   // Lecture des données a jeter
   dht.reset11();

   // L'objet Timer1 est automatiquement créé dans la bibliothèque TimerOne
   // Règle la période d'interruption
   Timer1.initialize(TIMER_PERIOD);
   // Règle la fonction de gestion d'interruption
   Timer1.attachInterrupt(readTemperature);
   // Démarrer le comptage
   Timer1.start();

   // Pour l'afficheur LCD
   // 0) Il s'agir d'un afficheur 16 x 2
   lcd.begin(16, 2);
   // 1) Effacer l'affichage
   lcd.clear();
}

/**
 * @brief Fonction principale de l'Arduino. Elle est exécutée comme dans
 * une boucle sans fin.
 */
void loop()
{
   // Alterner entre le message de bienvenue et les varibales du DHT11
   // messageCount: nombre de fois le contenu a été affiché
   // showingMessage: true -> bienvenue, false -> température, humidité
   static int16_t messageCount{0};
   static bool showingMessage{true};

   lcd.display();
   waitUntil(2000);
   lcd.noDisplay();
   waitUntil(1000);

   // Traitement du code de retour pour voir s'il est ok
   if (chk == DHTLIB_ErrorCode::DHTLIB_OK)
   {
      // Stockage des valeurs du DHT11
      dht_humi = dht.getHumidity();
      dht_temp = dht.getTemperature();
      showTemp((int)dht_temp, (int)dht_humi, lcd);
   }
   else
   {
      // Afficher message d'erreur
      Serial.print(F("Error: "));
      Serial.println(static_cast<int>(chk));

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error: ");
      lcd.print(static_cast<int>(chk), 10);
   }
}