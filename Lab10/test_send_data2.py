#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''Ce programme écrit cinq (5) données dans les champs d'un canal sur ThingSpeak.

Requête REST à réaliser:
GET https://api.thingspeak.com/update?api_key=cle_ecriture_canal&field1=V1&field2=V2&field3=V3

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
import requests, time

# -------------------------------------------------------------------
# ThingSpeak
# -------------------------------------------------------------------
# Méthode: GET
# -------------------------------------------------------------------
# URI pour écrire des donnée:
# https://api.thingspeak.com/update.<format>
#--------------------------------------------------------------------
# Référence API REST de ThingSpeak:
# https://www.mathworks.com/help/thingspeak/rest-api.html
# -------------------------------------------------------------------
THINGSPK_URL = 'https://api.thingspeak.com/update'
THINGSPK_API_KEY = 'OUEJ9S1WGSZGG47B'
DELAY = 20              # interval de transmission

# -------------------------------------------------------------------
# Données à envoyer à ThingSpeak
# -------------------------------------------------------------------
DATA = ((32, 27), (34, 30), (28, 25), (30, 31), (33, 26))

# -------------------------------------------------------------------
# Fonction principale
# -------------------------------------------------------------------
def main():
  # Note: f-string disponible depuis Python 3.6
  print(f'Envoyer {len(DATA)} données dans les {len(DATA[0])} champs du canal (interval de {DELAY} sec)')
  for d in DATA:
    try:
      print(f'Écrire {d[0]} et {d[1]} dans les champs...', end=' ')         
      resp = requests.get(THINGSPK_URL,
                          # 10 secondes pour connection et read timeout
                          timeout = (10, 10),
                          # Parmaètres de cette requête
                          params = { "api_key" : THINGSPK_API_KEY,
                                      "field1"  : d[0],
                                      "field2"  : d[1]}
                          )

      print(f"ThingSpeak GET response: {resp.status_code}")
      # Vérifier la réponse de ThingSpeak
      if resp.status_code != 200:
        print("Erreur de communication détectée!")

      # Attendre 20 secondes (licence gratuite a un délai de 15 secondes)
      time.sleep(DELAY)

    except requests.ConnectionError:
      print('Erreur de connexion')
      return
    except requests.Timeout:
      print("Exception de timeout reçue (connexion ou écriture)")
      return
    except requests.HTTPError:
      print('Erreur au niveau du protocole HTTP')
      return

    

# ------------------------------------------------------
# Programme principal
# ------------------------------------------------------
if __name__ == '__main__':
  main()
