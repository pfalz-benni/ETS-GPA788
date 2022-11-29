#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''Ce programme lit quatre (4) données des champs d'un canal sur ThingSpeak.

Requête REST à réaliser:
GET https://api.thingspeak.com/channels/numero_canal/feeds.json?api_key=cle_lecture&results=4

Note: on veut la réponse sous forme de la représentation JSON

Le programme utilise le module "requests" pour réaliser la communication HTTP.
C'est la façon moderne et simple d'accéder au services WEB sous Python. Ainsi,
l'accès à une plateforme IoT via leur API REST est devenu très simple.

(voir les notes de cours "Style REST (I)")

Convention PEP 8 (Python Enhencement Proposal 8):
  Variables -> snake_case
  Class -> PascalCase
  Constante -> SNAKE_CASE

GPA788 Conception et intégration des objets connectés
T. Wong
Juin 2018
Juillet 2020
Novembre 2021
'''
# -------------------------------------------------------------------
# Les modules utiles
# -------------------------------------------------------------------
import requests, sys

# -------------------------------------------------------------------
# ThingSpeak
# -------------------------------------------------------------------
# Méthode: GET
# -------------------------------------------------------------------
# URL pour lire des donnée:
# https://api.thingspeak.com/channels/numero_canal/feeds.<format>
#--------------------------------------------------------------------
# Référence API REST de ThingSpeak:
# https://www.mathworks.com/help/thingspeak/rest-api.html
# -------------------------------------------------------------------
THINGSPK_CHANNEL_ID = '1952250'
THINGSPK_URL = 'https://api.thingspeak.com/channels/' + THINGSPK_CHANNEL_ID + '/feeds.json'
THINGSPK_READ_API_KEY = 'VOQX8PFJ4LD0SEVB'

# -------------------------------------------------------------------
# Fonction principale
# -------------------------------------------------------------------
def main():
  # Note: f-string disponible depuis Python 3.6
  print(F'Lire 4 données les plus récentes du canal {THINGSPK_CHANNEL_ID}')
  try:
    resp = requests.get(THINGSPK_URL,
                        # 10 secondes pour connection et read timeout
                        timeout = (10, 10),
                        # Paramètres de cette requête
                        params = { "api_key" : THINGSPK_READ_API_KEY, "results"  : 4}
                        )
    print('La requête est:', resp.url)
    print("ThingSpeak GET response: ", resp.status_code)
    if resp.status_code != 200:
        print("Erreur de communication détectée!")
        sys.exit(1)     # # Par exemple, notre convention est 1 = Erreur requête 
    # Décoder la réponse de JSON à Python
    # Voir le document "Représentation JSON" pour un survol
    print('Décodage de la réponse de JSON à Python...')
    try:
        # Voir le document "Représentation JSON" pour connaître
        # le format de la représentation JSON utilisé par ThingSpeak
        r = resp.json()
        for d in r['feeds']:
            print('Point #', d['entry_id'], end = ' ')
            for i in range(2):
                print('champ{}, valeur: {}'.format(i + 1, d['field%i' % (i + 1)]), end = ' ')
            print('')
    except ValueError:
        print('Exception ValueError reçue (décodage JSON)')
        sys.exit(2)     # Par exemple, notre convention est 2 = Erreur JSON 

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

