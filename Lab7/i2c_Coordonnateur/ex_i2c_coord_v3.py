#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''Ce programme réalise le côté coordonnateur d'une communication I2C.

Version du programme ex_i2c_coord gérant deux (2) noeuds. La gestion
des exceptions/erreurs repose sur le style orienté-objet de Python.

Contexte
-=-=-==-
Dans cet exemple, les noeuds (Arduino) effectuent:
  1) la lecture de la température du CPU;
  2) incrémenter le numéro d'échantillon.
selon la période d'échantillonnage NEW_TS programmé par le coordonnateur (Pi).
À toutes les SAMPLING_TIME, le coordonnateur lit ces deux valeurs des noeuds
et les affiche à la sortie standard (terminal Python). On a NEW_TS != SAMPLE_TIME
pour montrer l'asynchronisme entre les noeuds et le coordonnateur.

(voir les notes de cours "I2C: Pi et Arduino")

GPA788 Conception et intégration des objets connectés
T. Wong
Juin 2018
Juillet 2020
'''

# -------------------------------------------------------------------
# Les modules utiles
# -------------------------------------------------------------------
import smbus            # pour la communication I2C
import time             # pour sleep()
import struct           # pour la conversion octet -> float
from datetime import datetime    # pour l'horodatage des échantillons
import constants as cst # constants du programme

# -------------------------------------------------------------------
# Structures pour la conversion des données reçues
# -------------------------------------------------------------------
''' bytearray
    Liste d'octets qui servira à la conversion octets -> int (numéro d'échantillon)
    et octets -> float (températire interne de l'ATmega328P)

    dictionnaire de dictionnaires
    Structure servant à entreposer les valeurs (no. d'échantillon et tempéraure)
    de chaque des noeuds. Les dictionnaires permettent l'indexage numérique et
    alphanumérique des champs. Ainsi, on utilisera l'adresse I2C des noeuds
    comme l'index principal. Cela facilitera la programmation des accès aux données.
'''
# -------------------------------------------------------------------
# Classe d'exception
# -------------------------------------------------------------------
class CoordException(Exception):
  '''Cette classe représente les exceptions en lien avec l'utilisation
     des fonctions de ce programme.

  Un objet de cette classe d'exception est envoyée au programme principal
  par les fonctions. Elle représente les problèmes détectés l'utilisation
  du bus I2C et les bornes fonctions MAIS pas les erreurs de communication I2C.
  '''
  def __init__(self, *args):
    if args:
      self.message = args[0]
    else:
      self.message = None
      super().__init__(self.message)

  def __str__(self):
    if self.message:
      return self.message
    else:
      return F"Exception CoordException a été lancée."

# ------------------------------------------------------
# Fonctions utilisateurs
# ------------------------------------------------------
def send_stop(bus = None, adr = -1):
  '''Envoyer la commande Arrêt sur le bus i2c au noeud à l'adresse adr.

  Paramètres:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire

  Retour: n/a
  Exceptions possibles: CoordException, IOError
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    bus.write_byte(adr, cst.I2C_CMD_SET_STOP)
  else:
    raise CoordException(F"<send_stop> Bus non initié ou adresse I2C invalide.")

def send_go(bus = None, adr = -1):
  '''Envoyer la commande démarrage sur le bus i2c au noeud à l'adresse adr.

  Arguments:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire

  Retour: n/a
  Exceptions possibles: CoordException, IOError
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    bus.write_byte(adr, cst.I2C_CMD_SET_GO)
  else:
    raise CoordException(F"<send_go> Bus non initié ou adresse I2C invalide.")

def send_Ts(bus = None, adr = -1, ts = -1):
  '''Envoyer la commande pour le changement de période d'échantillonnage
  et sa nouvelle valeur.

  Arguments:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire
  ts (int) -- nouvelle période d'échantillonnage

  Retour: n/a
  Exceptions possibles: CoordException, IOError
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    # Note: la fonction write_i2c_block_data transmet une commande
    #       et des données en bloc. Les données doivent être dans
    #       une liste même si elle n'a qu'une seule données d'où
    #       [ts] avec des crochets.
    bus.write_i2c_block_data(adr, cst.I2C_CMD_SET_TS, [ts])
  else:
    raise CoordException(F"<send_Ts> Bus non initié ou adresse I2C invalide.")

def read_Temp(bus = None, adr = -1):
  '''Lire la température du noeud à l'adresse adr.

  Arguments:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire

  Retour (float): Valeur de la température
  Exceptions possibles: CoordException, IOError, struct.error
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    # On fait la lecture de la température octet par octet.
    # MSB -> adresse haute, LSB -> adresse basse
    # (c'est l'ordre little Endian)
    T = bytearray([0x00, 0x00, 0x00, 0x00])
    bus.write_byte(adr, cst.I2C_NODE_TEMP_MSB1)
    T[3] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_TEMP_MSB0)
    T[2] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_TEMP_LSB1)
    T[1] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_TEMP_LSB0)
    T[0] = bus.read_byte(adr)
    # Convertir les 4 octets représentant la température de format binary32 
    # en nombre virgule flottante
    temperature = struct.unpack('f', T)
    # Note: la conversion par struct produit des listes d'objets dans nb_echan 
    #       et temperature. C'est pourquoi on prend uniquement le premier élément
    #       avec la syntaxe [0].
    return temperature[0]
  else:
    raise CoordException(F"<read_Temp> Bus non initié ou adresse I2C invalide.")

def read_Hum(bus = None, adr = -1):
  '''Lire l'humidité du noeud à l'adresse adr.

  Arguments:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire

  Retour (float): Valeur de la humidité
  Exceptions possibles: CoordException, IOError, struct.error
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    # On fait la lecture de l'humidité octet par octet.
    # MSB -> adresse haute, LSB -> adresse basse
    # (c'est l'ordre little Endian)
    H = bytearray([0x00, 0x00, 0x00, 0x00])
    bus.write_byte(adr, cst.I2C_NODE_HUM_MSB1)
    H[3] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_HUM_MSB0)
    H[2] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_HUM_LSB1)
    H[1] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_HUM_LSB0)
    H[0] = bus.read_byte(adr)
    # Convertir les 4 octets représentant l'humidité de format binary32 
    # en nombre virgule flottante
    humidity = struct.unpack('f', H)
    # Note: la conversion par struct produit des listes d'objets dans nb_echan 
    #       et humidité. C'est pourquoi on prend uniquement le premier élément
    #       avec la syntaxe [0].
    return humidity[0]
  else:
    raise CoordException(F"<read_Hum> Bus non initié ou adresse I2C invalide.")

def read_Leq(bus = None, adr = -1):
  '''Lire le Leq du noeud à l'adresse adr.

  Arguments:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire

  Retour (float): Valeur du Leq
  Exceptions possibles: CoordException, IOError, struct.error
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    # On fait la lecture du Leq octet par octet.
    # MSB -> adresse haute, LSB -> adresse basse
    # (c'est l'ordre little Endian)
    L = bytearray([0x00, 0x00, 0x00, 0x00])
    bus.write_byte(adr, cst.I2C_NODE_LEQ_MSB1)
    L[3] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_LEQ_MSB0)
    L[2] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_LEQ_LSB1)
    L[1] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_LEQ_LSB0)
    L[0] = bus.read_byte(adr)
    # Convertir les 4 octets représentant le Leq de format binary32 
    # en nombre virgule flottante
    Leq = struct.unpack('f', L)
    # Note: la conversion par struct produit des listes d'objets dans nb_echan 
    #       et du Leq. C'est pourquoi on prend uniquement le premier élément
    #       avec la syntaxe [0].
    return Leq[0]
  else:
    raise CoordException(F"<read_Leq> Bus non initié ou adresse I2C invalide.")


def read_SNumber(bus = None, adr = -1):
  '''Lire le numéro de l'échantillon de la température du noeud à l'adresse adr.

  Arguments:
  bus -- objet SMBUS déjà initialisé
  adr (int) -- adresse du noeud destinataire

  Retour (int): Numéro de l'échantillon
  Exceptions possibles: CoordException, IOError, struct.error
  '''
  if bus != None and (adr >= cst.I2C_MIN_ADR and adr <= cst.I2C_MAX_ADR):
    # Lire le numéro d'échantillon acquis par le noeud depuis son démarrage.
    # On fait cette lecture octet par octet (ordre little Endian)
    N = bytearray([0x00, 0x00])
    bus.write_byte(adr, cst.I2C_NODE_NS_MSB)
    N[1] = bus.read_byte(adr)
    bus.write_byte(adr, cst.I2C_NODE_NS_LSB)
    N[0] = bus.read_byte(adr)
    # Convertir les 2 octets du numéro d'échantillon en valeur entière
    nb_echan = struct.unpack('H', N)
    # Note: la conversion par struct produit des listes d'objets dans nb_echan 
    #       et temperature. C'est pourquoi on prend uniquement le premier élément
    #       avec la syntaxe [0].
    return nb_echan[0]
  else:
    raise CoordException(F"<read_SNumber> Bus non initié ou adresse I2C invalide.")

# ------------------------------------------------------
# Fonction principale
# ------------------------------------------------------
def main():

  # Stocker les données reçues dans un dictionnaire:
  #    clés -> adresses I2C des noeuds
  # valeurs -> dictionnaires contenant deux chmaps 'Température' et 'Sample_Num'
  Sensor_Data = {
    cst.I2C_ADDRESS[0] : {'Temperature' : -1.0,'Humidity' : -1.0, 'Sample_Num' : 0},
    cst.I2C_ADDRESS[1] : {'Leq' : -1.0, 'Sample_Num' : 0}
  }

  # Bon. Indiquer que le coordonnateur est prêt...
  print("Coordonnateur (Pi) en marche avec Ts =", cst.SAMPLING_TIME, "sec.")
  print("ctrl-c pour terminer le programme.")

  # Instancier un objet de type SMBus et le lié au port i2c-1
  bus = smbus.SMBus(1)
  try:
    # 1) C'est une bonne pratique d'arrêter le noeud avant de
    #    lancer des commandes.
    print('Arrêter les noeuds.')
    for adr in cst.I2C_ADDRESS:
      send_stop(bus, adr)

    # 2) Régler le temps d'échantillonnage du noeud à NEW_TS secondes
    print("Assigner une nouvelle Ts =",  cst.NEW_TS, "aux noeuds.")
    for adr in cst.I2C_ADDRESS:
      send_Ts(bus, adr, cst.NEW_TS)
      time.sleep(0.1)         # attendre avant de continuer l'écriture

    # 3) Ok. Demander au noeud de démarrer/continuer son échantillonnage
    print("Demander aux noeuds de démarrer l'échantillonnage.")
    for adr in cst.I2C_ADDRESS:
      send_go(bus, adr)

    # 4) Le coordonnateur demande et reçoit des données du noeud
    #    jusqu'à ce que l'utilisateur arrête le programme par ctrl-c.
    while True:         # boucle infinie
      # 4.1) attendre la fin de la période d'échantillonnage
      #      du coordonateur
      time.sleep(cst.SAMPLING_TIME)

      # 4.2) Lire la température,l'humidité et le Leq interne du noeud.
      Sensor_Data[cst.I2C_ADDRESS[0]]['Temperature'] = read_Temp(bus, cst.I2C_ADDRESS[0])
      Sensor_Data[cst.I2C_ADDRESS[0]]['Humidity'] = read_Hum(bus, cst.I2C_ADDRESS[0])
      Sensor_Data[cst.I2C_ADDRESS[1]]['Leq'] = read_Leq(bus, cst.I2C_ADDRESS[1])

      # 4.3) Lire le temps local comme l'horodatage (timestamp)
      #      N'oubliez pas de régler le temps du Pi s'il n'est pas relié au réseau.
      temps = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

      # 4.4) Lire le nombre d'échantillon acquis par le noeud
      for adr in cst.I2C_ADDRESS:
        Sensor_Data[adr]['Sample_Num'] = read_SNumber(bus, adr)


      # 4.5) Afficher les données reçues à la sortie standard
      print("\n<Temps: ", temps, ">")
    
      print("Noeud: {0}, Échantillon: {1}, Température: {2:.2f},  Humidity: {3:.2f}".format(hex(cst.I2C_ADDRESS[0]),
      Sensor_Data[cst.I2C_ADDRESS[0]]['Sample_Num'], Sensor_Data[cst.I2C_ADDRESS[0]]['Temperature'],Sensor_Data[cst.I2C_ADDRESS[0]]['Humidity']))
      print("Noeud: {0}, Échantillon: {1}, Leq: {2:.2f}".format(hex(cst.I2C_ADDRESS[1]),
      Sensor_Data[cst.I2C_ADDRESS[1]]['Sample_Num'], Sensor_Data[cst.I2C_ADDRESS[1]]['Leq']))


  except KeyboardInterrupt:
    # 5) Ctrl-c reçu alors arrêter l'échantillonnage
    for adr in cst.I2C_ADDRESS:
      send_stop(bus, adr)
  except IOError as io_e:
    print("Erreur détectée sur le bus i2c.") 
    print("Message d'erreur: ", io_e)
  except struct.error as conv_e:
    print("Erreur détectée lors de la conversion des données.") 
    print("Message d'erreur: ", conv_e)
  except CoordException as ce:
    print("Problème détecté dans l'utilisation des fonctions.")
    print(F"Message d'erreur: {ce}")

#
# Il faut aussi gérer les autres exceptions!   
#


# ------------------------------------------------------
# Programme principal
# ------------------------------------------------------
if __name__ == '__main__':
  main()