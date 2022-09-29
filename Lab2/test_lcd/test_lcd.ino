/*
 * test_lcd.ino
 * Utiliser la bibliothèque LiquidCrystal pour faire afficher
 * des messages/données sur l'afficheur LCD.
 *
 * L'afficheur LCD pourrait être intégré dans le projet IoT
 * de ce cours.
 *
 * GPA788 - ETS
 * T. Wong
 * 08-2019
 * 08-2020
 *
 * Ce programme est inspéré de l'exemple "Hello World!" de la page:
 * https://www.arduino.cc/en/Tutorial/HelloWorld. La classe LiquidCrystal()
 * et ses fonctions membres sont listées sur la page:
 * https://www.arduino.cc/en/Reference/LiquidCrystal
 * Le branchement électrique est présenté d'une façon plus évidente dans le
 * protocole de laboratoire du cours. Finalement, la technique de lecture
 * de la température interne de l'ATmega328P est donnée dans les notes
 * de cours.
 */

// Classe pour afficher des caractères à l'aide du LCD
#include <LiquidCrystal.h>
// Classe pour mesurer la température interne de l'ATmega328P
// #include "ChipTemp.h"
#include "dht.h"

/* ---------------------------------------------------------------
   Créer un objet (global) de type LiquidCrystal.
   Le constructeur utilisé est:
     LiquidCrystal(rs, enable, d4, d5, d6, d7)
     rs (register select): broche 12
     enable: broche 11
     d4: broche 5, d5: broche 4, d6: broche 3, d7: broche 2
   ---------------------------------------------------------------
   Note: Les numéros de broches sont ceux du côté d'Arduino.
         Brancher ces broches sur l'afficheur LCD selon le
         diagramme électrique donné sur le protocole de laboratoire.
   --------------------------------------------------------------- */
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Créer un objet de type dht, une température et une humidité
dht DHT;

/*Variables et Constantes*/
double dht_temp, dht_humi;     // Création de variables afin de stocker et afficher les valeurs du DHT11
const int DHT11_PIN{7};        // Relier le capteur à la broche #7
const int16_t NB_MSG_COUNT{2}; //

/* ---------------------------------------------------------------
   Fonction d'initialisation obligatoire (exécutée 1 seule fois).
   --------------------------------------------------------------- */
void setup()
{
   // Fixe le débit de communication en nombre de caracères par
   // seconde(baud) pour la communication série
   Serial.begin(115200);

   // Sur le VS Code, l'ouverture du port série prend du temps et on
   // peut perdre des caractères. Ce problème n'existe pas sur la carte
   // de l'Arduino.

   // Attente de 2 secondes
   waitUntil(2000);

   // Pour l'afficheur LCD
   // 0) Il s'agir d'un afficheur 16 x 2
   lcd.begin(16, 2);
   // 1) Effacer l'affichage
   lcd.clear();

   // Lecture des données a jeter
   // TODO
}

/* ---------------------------------------------------------------
   Fonction principale
   (Exécutée comme une fonction dans une boucle sans fin)
   --------------------------------------------------------------- */
void loop()
{
   // Alterner entre le message de bienvenue et les varibales du DHT11
   // messageCount: nombre de fois le contenu a été affiché
   // showingMessage: true -> bienvenue, false -> température, humidité
   static int16_t messageCount{0};
   static bool showingMessage{true};

   int chk = DHT.read11(DHT11_PIN);

   // Traitement du code de retour pour voir s'il est ok
   if (chk == DHTLIB_OK)
   {
      // Stockage des valeurs du DHT11
      dht_humi = DHT.humidity;
      dht_temp = DHT.temperature;

      // Si le compteur est à zéro alors change le contenu
      if (messageCount == 0)
      {
         //
         lcd.clear();
         if (showingMessage)
         {                                               // était message de bienvenue? alors
            showTemp((int)dht_temp, (int)dht_humi, lcd); // affiche la température interne.
         }
         else
         {
            welcome(lcd);
         }
      }
      // Si le compteur est > NB_MSG_COUNT alors remettre le compteur
      // à zéro et change le contenu de l'afficheur.
      if (messageCount++ > NB_MSG_COUNT)
      {
         messageCount = 0;
         showingMessage = !showingMessage;
      }
   }

   else
   {
      // Afficher message d'erreur
      Serial.print(F("Error: "));
      Serial.println(chk);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error: ");
      lcd.print(chk, 10);
   }

   // Faire clignoter lentement l'afficheur
   lcd.display();
   waitUntil(2000);
   lcd.noDisplay();
   waitUntil(1000);
}

/* ---------------------------------------------------------------
   Fonction pour créer un delai de w millisecondes
   La fonction delay() est utilisée dans bien des tutoriel pour
   créer un delai temporel. On peut aussi créer notre propre délai
   et utiliser une unité de temps à notre guise.
   --------------------------------------------------------------- */
void waitUntil(uint32_t w)
{
   uint32_t t{millis()};
   // Attendre w millisecondes
   while (millis() < t + w)
   {
   }
}

/* ---------------------------------------------------------------
   Fonction pour afficher un message de bienvenue.
   ---------------------------------------------------------------
   Note: On aurait pu utiliser l'objet global lcd directement.
   --------------------------------------------------------------- */
void welcome(LiquidCrystal &l)
{
   // D'abord le terminal série
   Serial.println(F("Bienvenue au GPA788 OC/IoT"));
   // Ensuite l'afficheur LCD
   l.setCursor(0, 0);        // Cursuer à la 1ere colonne, 1ere ligne
   l.print("Bienvenue au");  // Afficher le texte...
   l.setCursor(0, 1);        // Curseur à la 1ere colonne, 2e ligne
   l.print("GPA788 OC/IoT"); // Afficher le texte...}
}

/* ---------------------------------------------------------------
   Fonction pour afficher la température et l'humidité du DHT11.
   ---------------------------------------------------------------
   Note: On aurait pu utiliser l'objet global chipTemp directement.
   --------------------------------------------------------------- */
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