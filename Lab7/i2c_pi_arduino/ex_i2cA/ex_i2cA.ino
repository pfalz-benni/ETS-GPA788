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
#include <Wire.h>                      // Pour la communication I2C               
#include <util/atomic.h>               // Pour la section critique
#include "ChipTemp.h"                  // Pour lire la température du microcontrôleur

/* ------------------------------------------------------------------
   Globales pour la classe ChipTemp
   ------------------------------------------------------------------ */
const float DECALAGE{324.31};          // Choisir la bonne: 316.0, 324.31, 332.70, 335.2
const float GAIN{1.22};                // Choisir la bonne: 1.06154, 1.22
ChipTemp ct(GAIN, DECALAGE);           // Instancier un objet de classe ChipTemp


/* ------------------------------------------------------------------
   Globales pour la communication I2C
   ------------------------------------------------------------------ */
const uint8_t ADR_NOEUD{0x44};        // Adresse I2C de ce noeud
const uint8_t NB_REGISTRES{7};        // Nombre de registres de ce noeud

/* La carte des registres ------------------------------------------- */
union CarteRegistres {
  // Cette structure: Utilisée par le noeud pour lire et écrire
  //                  des données.
  struct {
    // Taux d'échantillonnage (1 octet)
    volatile uint8_t Ts;
    // Nombre d'échantillons (2 octets)
    volatile int16_t nb_echantillons;
    // Température interne du processeur ATMEGA en celsius
    // (4 octets)
    volatile float temperature;
  } champs;
  // Ce tableau: Utilisé par le coordonnateur pour lire et écrire
  //             des données.
  uint8_t regs[NB_REGISTRES];
};

union CarteRegistres cr;               // Une carte des registres
float temperature;                     // Variable intermédiaire pour mémoriser la température
uint8_t adrReg;                        // Adresse du registre reçue du coordonnateur
enum class CMD { Stop = 0, Go};        // Commandes venant du coordonnateur
volatile CMD cmd;                      // Go -> échantilloner, Stop -> arrêter
const uint8_t MIN_Ts = 5;              // Période d'échantillonnage min (sec)
const uint8_t MAX_Ts = 200;            // Période d'échantillonnage max (sec)

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
  cr.champs.Ts = MIN_Ts;
  cr.champs.nb_echantillons = 0;
  cr.champs.temperature = -1;
  temperature = -1;
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
  Serial.print(F("Noeud à l'adresse 0x")); Serial.print(ADR_NOEUD, HEX);
  Serial.println(F(" prêt à recevoir des commandes"));
}

/* ------------------------------------------------------------------
   Boucle principale
   ------------------------------------------------------------------ */   
void loop()
{
  // Échantillonner la température interne si la commande est Go
  if (cmd == CMD::Go) {
    temperature = ct.celsius();
    // Section critique: empêcher les interruptions lors de l'assignation
    // de la valeur de la température à la variable dans la carte des registres.
    // Recommandation: réaliser la tâche la plus rapidement que possible dans
    //                 la section critique.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      // Assigner la température lue dans cr.champs.temperature
      cr.champs.temperature = temperature;
      // Augmenter le compte du nombre d'échantillons
      cr.champs.nb_echantillons++;
    }      
    Serial.print(F("#")); Serial.print(cr.champs.nb_echantillons); Serial.print(F("  "));
    Serial.println(cr.champs.temperature);

    // Attendre la prochaine période d'échantillonnage
    waitUntil(cr.champs.Ts * 1000);

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
void i2c_receiveEvent(int n) {
  // Traiter les commandes ou le numéro d'un registre (1 octet)
  if (n == 1) {
    // Un seul octet reçu. C'est probablement une commande.
    uint8_t data = Wire.read();
    switch (data) {
      case 0xA1:
        cmd = CMD::Stop;
        Serial.println(F("commande 'Arrêter' reçue"));
        break;
    case 0xA2:
        cmd = CMD::Go;
        cr.champs.nb_echantillons = 0;
        Serial.println(F("Commande 'Démarrer' reçue"));
        break;
    default:
        // Sinon, c'est probablement un numéro de registre
        if ((data >= 0) && (data < NB_REGISTRES)) {
          adrReg = data;
        }
        else
          adrReg = -1; // Il y sans doute une erreur!
    }
  } 
  else if (n == 2) {
    // Deux octets reçus. C'est probablement pour changer le
    // taux d'échantillonnage.
    uint8_t data1 = Wire.read();
    uint8_t data2 = Wire.read();
    if ((data1 == 0xA0) && (data2 >= MIN_Ts) && (data2 <= MAX_Ts)) {
      Serial.println(F("Commande 'Changer Ts' reçue")); 
      cr.champs.Ts = data2;
      Serial.print(F("La nouvelle valeur est: ")); 
      Serial.print(cr.champs.Ts); Serial.println(F(" secondes"));
    }
  }
  else {
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
void i2c_requestEvent(){
  // Le coordonnateur veut la valeur d'un registre. L'adresse du
  // registre a été reçue précédemment.
  if ((adrReg >= 0) && (adrReg < NB_REGISTRES)){
    Serial.print("");
    // Envoyer le contenu du registre au coordonnateur
    Wire.write(cr.regs[adrReg]);
  }
}

/* ---------------------------------------------------------------
   Fonction pour créer un delai de w millisecondes
   
   La fonction delay() est utilisée dans bien des tutoriels pour
   créer un delai temporel. On peut aussi créer notre propre délai
   et utiliser une unité de temps à notre guise.
   --------------------------------------------------------------- */
void waitUntil(uint32_t w) {
  uint32_t t{millis()};
  // Attendre w millisecondes
  while (millis() < t + w) {}
}
