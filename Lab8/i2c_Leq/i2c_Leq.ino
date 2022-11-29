/*  ex_i2cA - Noeud A
 *  Ce programme est un exemple de communication I2C
 *  entre un coordonnateur (Pi) et un neoud (Arduino).
 *  Ce noeud est capable de transférer vers le coordonnateur:
 *
 *    - la valeur de la température interne;
 *    - le numéro de l'échantillon.
 *
 *  De plus, le noeud est capable de recevoir les commandes suivantes
 *  du coordonnateur:
 *    - STOP: arrêter l'échantillonnage;
 *    - GO:   démarrer l'échantillonnage.
 *
 *  Dans cet exemple, l'arrêt de l'échantillonnage remet à zéro le numéro
 *  de l'échantillon.
 *
 *  GPA788 Conception et intégration des objets connectés
 *  T. Wong
 *  06/2018
 *  08/2020
 *  11/2021
 */
#include <Wire.h>            // Pour la communication I2C
#include <util/atomic.h>     // Pour la section critique
#include "calculateur_leq.h" // Pour lire Leq sur Arduino

#include "wait.h"

/* ------------------------------------------------------------------
   Globales pour la classe Calculateur_Leq
   ------------------------------------------------------------------ */

const uint32_t TS{62};        // Péruide d'échantillionnage (ms)
const uint16_t NB_SAMPLE{32}; // 32 x 62 ms ~ 2 secondes
const uint16_t NB_LI{10};     // 150 x 2 secondes = 5 minutes (*) // TODO Reset to 150

// Créer un objet de type Leq
Calculateur_Leq leq(TS, NB_SAMPLE, NB_LI);

/* ------------------------------------------------------------------
   Globales pour la communication I2C
   ------------------------------------------------------------------ */
const uint8_t ADR_NOEUD{0x45}; // Adresse I2C de ce noeud
const uint8_t NB_REGISTRES{7}; // Nombre de registres de ce noeud

/* La carte des registres ------------------------------------------- */
union CarteRegistres
{
  // Cette structure: Utilisée par le noeud pour lire et écrire
  //                  des données.
  struct
  {
    // Taux d'échantillonnage (1 octet)
    volatile uint8_t Ts;
    // Nombre d'échantillons (2 octets)
    volatile int16_t nb_echantillons;
    // Leq mesuréee par sur l'Arduino
    // (4 octets)
    volatile float Leq;
  } champs;
  // Ce tableau: Utilisé par le coordonnateur pour lire et écrire
  //             des données.
  uint8_t regs[NB_REGISTRES];
};

enum class CMD
{
  ChangeTs = 0xA0,
  Stop = 0xA1,
  Go = 0xA2,
  Reset = 0xA3,
  Pause = 0xA4
}; // Commandes venant du coordonnateur

union CarteRegistres cr; // Une carte des registres
float Leq;               // Variable intermédiaire pour mémoriser la Leq
uint8_t adrReg;          // Adresse du registre reçue du coordonnateur

volatile CMD cmd; // Go -> échantilloner, Stop -> arrêter

uint32_t wakeUpTime = -1; // Temps à reveiller l'Arduino après commande pause

const uint8_t MIN_TS_SEC{5};   // Période d'échantillonnage min (sec)
const uint8_t MAX_TS_SEC{200}; // Période d'échantillonnage max (sec)

const uint8_t MIN_PAUSE_SEC{5};      // Période d'échantillonnage min (sec)
const uint8_t MAX_PAUSE_SEC{100000}; // Période d'échantillonnage max (sec)

/* ------------------------------------------------------------------
   Initialisation
   ------------------------------------------------------------------ */
void setup()
{
  // Pour la communication série
  Serial.begin(115200);

  // Sur le VS Code, l'ouverture du port série prend du temps et on
  // peut perdre des caractères. Ce problème n'existe pas sur l'IDE
  // de l'Arduino.
  waitUntil(2000);

  // Initialiser les champs de la carte des registres
  cr.champs.Ts = MIN_TS_SEC;
  cr.champs.nb_echantillons = 0;
  cr.champs.Leq = -1;
  Leq = -1;

  // Initialiser les variables de contrôle de la
  // communication I2C
  cmd = CMD::Stop;
  adrReg = -1;
  // Réglage de la bibliothèque Wire pour le I2C
  Wire.begin(ADR_NOEUD);
  // Fonction pour traiter la réception de données venant du coordonnateur
  Wire.onReceive(i2c_receiveEvent);
  // Fonction pour traiter une requête de données venant du coordonnateur
  Wire.onRequest(i2c_requestEvent);

  // Indiquer que le noeud est prêt
  Serial.print(F("Noeud à l'adresse 0x"));
  Serial.print(ADR_NOEUD, HEX);
  Serial.println(F(" prêt à recevoir des commandes"));
}

/* ------------------------------------------------------------------
   Boucle principale
   ------------------------------------------------------------------ */
void loop()
{
  // Lire Leq interne si la commande est Go et le temps cr.champs.Ts
  // est écoulé depuis la dernière lecutre
  static unsigned long start = millis();

  if (cmd == CMD::Go)
  {
    // L'objet leq "sait" à quel moment il doit accumuler les valeurs
    // du signal sonore. Accumulate est applé toujours alors
    leq.Accumulate();
    // L'objet leq sait à quels moments il faut calculer Vrms, Li et Leq
    leq.Compute();

    if (millis() - start >= cr.champs.Ts * 1000)
    {
      start = millis();

      Leq = leq.GetLeq();

      // Section critique: empêcher les interruptions lors de l'assignation
      // de la valeur de Leq à la variable dans la carte des registres.
      // Recommandation: réaliser la tâche la plus rapidement que possible dans
      //                 la section critique.
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        // Assigner Leq lue dans cr.champs.Leq
        cr.champs.Leq = Leq;
        // Augmenter le compte du nombre d'échantillons
        cr.champs.nb_echantillons++;
      }
      Serial.print(F("#"));
      Serial.print(cr.champs.nb_echantillons);
      Serial.print(F("  "));
      Serial.println(cr.champs.Leq);
    }
  }
  else if (cmd == CMD::Pause && millis() >= wakeUpTime)
  {
    cmd = CMD::Go;
    Serial.println(F("Redémarrer Arduino après pause"));
  }
}

/* ------------------------------------------------------------------
   i2c_receiveFunc(int n)
   Cette fonction est exécutée à la réception des données venant
   du coordonnateur. Le paramètre n indique le nombre d'octets reçus.
   ------------------------------------------------------------------
   Note: Normalement on ne doit pas afficher des messages utilisant
         le port série - il y a risque de conflit entre les inter-
         ruptions. Donc, après débogage, n'oubliez pas de mettre les
         Serial print en commentaires ;-).
   ------------------------------------------------------------------ */
void i2c_receiveEvent(int n)
{
  // Traiter les commandes ou le numéro d'un registre (1 octet)
  if (n == 1)
  {
    // Un seul octet reçu. C'est soit une commande, soit le
    // numéro d'un registre demandé par le coordonnateur.
    uint8_t data = Wire.read();

    switch (data)
    {
    case static_cast<uint8_t>(CMD::Stop):
      cmd = CMD::Stop;
      Serial.println(F("commande 'Arrêter' reçue"));
      break;
    case static_cast<uint8_t>(CMD::Go):
      cmd = CMD::Go;
      cr.champs.nb_echantillons = 0;
      Serial.println(F("Commande 'Démarrer' reçue"));
      break;
    case static_cast<uint8_t>(CMD::Reset):
      // cmd n'est pas mise à CMD::Reset parce que l'execution
      // doit continuer comme avant après Reset
      Serial.println(F("commande 'Reset' reçue"));
      // Reset tout
      cr.champs.Ts = MIN_TS_SEC;
      cr.champs.nb_echantillons = 0;
      cr.champs.Leq = -1;
      Leq = -1;
      break;
    default:
      // Sinon, c'est probablement un numéro de registre
      if ((data >= 0) && (data < NB_REGISTRES))
      {
        adrReg = data;
      }
      else
      {
        adrReg = -1; // Il y a sans doute une erreur!
      }
    }
  }
  else if (n == 2)
  {
    // Deux octets reçus. C'est probablement pour changer le
    // taux d'échantillonnage.
    uint8_t data1 = Wire.read();
    uint8_t data2 = Wire.read();

    if ((data1 == static_cast<uint8_t>(CMD::ChangeTs)) && (data2 >= MIN_TS_SEC) && (data2 <= MAX_TS_SEC))
    {
      Serial.println(F("Commande 'Changer Ts' reçue"));
      cr.champs.Ts = data2;
      Serial.print(F("La nouvelle valeur est: "));
      Serial.print(cr.champs.Ts);
      Serial.println(F(" secondes"));
    }
    else if ((data1 == static_cast<uint8_t>(CMD::Pause)) && (data2 >= MIN_PAUSE_SEC) && (data2 <= MAX_PAUSE_SEC))
    {
      cmd = CMD::Pause;
      Serial.println(F("Commande 'Pause' reçue"));
      uint8_t pauseTimeSec = data2;
      Serial.print(F("Mettre Arduino en pause pendant : "));
      Serial.print(pauseTimeSec);
      Serial.println(F(" secondes"));

      wakeUpTime = millis() + pauseTimeSec * 1000;
    }
  }
  else
  {
    // Ignorer la réception n > 2 octets.
    Serial.println(F("Erreur: ce noeud n'accepte\
    pas de communication/commande à trois octets"));
  }
}

/* ------------------------------------------------------------------
   i2c_requestEvent()
   Cette fonction est exécutée à la réception d'une requête de
   données venant du coordonnateur.
   ------------------------------------------------------------------
   Note: Normalement on ne doit pas afficher des messages utilisant
         le port série - il y a risque de conflit entre les inter-
         ruptions. Donc, après débogage, n'oubliez pas de mettre les
         Serial print en commentaires ;-).
   ------------------------------------------------------------------ */
void i2c_requestEvent()
{
  // Le coordonnateur veut la valeur d'un registre. L'adresse du
  // registre a été reçue précédemment.
  if ((adrReg >= 0) && (adrReg < NB_REGISTRES))
  {
    Serial.print("");
    // Envoyer le contenu du registre au coordonnateur
    Wire.write(cr.regs[adrReg]);
  }
}