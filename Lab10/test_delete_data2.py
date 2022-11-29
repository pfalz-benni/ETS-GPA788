#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''Ce programme toutes les données de tous les champs d'un canal sur ThingSpeak.

Requête REST à réaliser:
DELETE https://api.thingspeak.com/channels/numero_canal/feeds.json?api_key=XXXXXX

Note: on veut la réponse sous forme de la représentation JSON

AVERTISSEMENT
Cette requête efface TOUTES les données de TOUS les champs d'un canal.  À utiliser
avec prudence. Exporter les données AVAN de lancer cette requête.

Le programme utilise le module "requests" pour réaliser la communication HTTP.
C'est la façon moderne et simple d'accéder au services WEB sous Python. Ainsi,
l'accès à une plateforme IoT via leur API REST est devenu très simple.

(voir les notes de cours "Style REST (I)")

GPA788 Conception et intégration des objets connectés
T. Wong
Juin 2018
Juillet 2020
'''
# -------------------------------------------------------------------
# Les modules utiles
# -------------------------------------------------------------------
import requests, sys

# -------------------------------------------------------------------
# ThingSpeak
# -------------------------------------------------------------------
# Méthode: DELETE
# -------------------------------------------------------------------
# URI effacer toutes des donnée:
# https://api.thingspeak.com/channels/Numero_canal/feeds.<format>
#--------------------------------------------------------------------
# Référence API REST de ThingSpeak:
# https://www.mathworks.com/help/thingspeak/rest-api.html
# -------------------------------------------------------------------
THINGSPK_CHANNEL_ID = '1952250'
THINGSPK_URL = 'https://api.thingspeak.com/channels/' + THINGSPK_CHANNEL_ID + '/feeds.json'
# Utiliser le USER API Key qui se trouve dans votre profile
THINGSPK_USER_API_KEY = '68RWX4R1GBL86K35'


# -------------------------------------------------------------------
# Fonction principale
# -------------------------------------------------------------------
def main():
  # Note: f-string disponible depuis Python 3.6
  print(F'Effacer les donnnés du canal {THINGSPK_CHANNEL_ID}')
  try:
    resp = requests.delete(THINGSPK_URL,
                          # 10 secondes pour connection et read timeout
                          timeout = (10, 10),
                          # Paramètres de cette requête
                          params = { "api_key" : THINGSPK_USER_API_KEY}
                          )
    print(f'La requête est:\n{resp.url}')
    print("ThingSpeak response: {resp.status_code}")
    if resp.status_code != 200:
      print("Erreur de communication détectée!")
      sys.exit(1)
    # Afficher la réponse de ThingSpeak 
    # Voir le document "Représentation JSON" pour un survol
    try:
      print('La réponse de ThingSpeak...')
      print(resp.json())
    except ValueError:
      print('Exception ValueError reçue (décodage JSON)')
      sys.exit(2)
  except requests.ConnectionError:
    print('Erreur de connexion')
    sys.exit(3)         # 3 = Erreur de connexion
  except requests.Timeout:
    print("Exception de timeout reçue (connexion ou écriture)")
    sys.exit(4)         # 4 = Erreur de timeout
  except requests.HTTPError:
    print('Erreur au niveau du protocole HTTP')
    sys.exit(5)         # 5 = Erreur HTTP

# ------------------------------------------------------
# Programme principal
# ------------------------------------------------------
if __name__ == '__main__':
  main()


