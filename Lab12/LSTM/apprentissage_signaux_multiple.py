#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''apprentissage_signaux_multiple Apprentissage d'un réseau LSTM.

-------------------------------------------------------------------------------
Prédiction de la pollution de l'air (particules fines < 2.5nm)
Programme pour réaliser l'apprentissage d'un réseau LSTM dont les données sont
multidimensionnelles (signaux temporels multiple).

Le fichier de données pollution.csv contient les signaux suivants:

  date: année-mois-jour heure (utilisé comme indice par pandas)
  pollution: particules fines < 2.5 nm (en ug/cm3)
  dew: point de rosée (en Celsius)
  temp: température (en Celsius)
  press: pression atmosphérique (en hecto Pascal)
  wd: direction du vent (N: nord, S: sud, E: est, W: ouest)
  ws: vitesse cumulée du vent (km/h)
  snow: heure cumulée de neige (en heure)
  rain: heure cumulée de pluie (en heure)

Note: Il y a 24 mesures par jour (1 mesure / heure).

La prédiction est sur la pollution en fonction des autres signaux. Une fenêtre
de +3 pas de temps pour l'entrée, +1 pas de temps pour la sortie. Donc, on veut
prédire la valeur de la pollution en utilisant 3 valeurs antérieures des signaux.
-------------------------------------------------------------------------------

Convention PEP 8 (Python Enhencement Proposal 8):
  Variables -> snake_case
  Class -> PascalCase
  Constante -> SNAKE_CASE

'''

# Utiliser la fonction de conversion des signaux
from conversion_signaux import signaux_apprentissage_supervise as ss
from math import sqrt
from numpy import mean, concatenate
from matplotlib import pyplot
from pandas import read_csv
from pandas import DataFrame
from pandas import concat
from sklearn.preprocessing import MinMaxScaler
from sklearn.preprocessing import LabelEncoder
from sklearn.metrics import mean_absolute_error
from tensorflow.python.keras.models import Sequential
from tensorflow.python.keras.layers import Dense
from tensorflow.python.keras.layers import LSTM
from tensorflow.python.keras.callbacks import EarlyStopping
# Pour mesurer la durée d'exécution de ce programme
import time


# Pour éliminer le message d'avertissement:
# "Your CPU supports instructions that this TensorFlow binary was not compiled to use: AVX AVX2"
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
entree=20
sortie=4
couche=3


# --------------------------------------------------------------------
# traitement_donnees
# --------------------------------------------------------------------
def traitement_donnees(): 
  # --------------------------------------------------------------------
  # Lire le fichier de données et extraire le nom des colonnes
  # --------------------------------------------------------------------
  # Charger le fichier de données
  data = read_csv('da.csv', header = 0, index_col = 0, encoding = 'ISO-8859-1')
  # Prendre les données sous forme d'un tableau numpy
  vals = data.values
  # Il faut encoder la colonne 'wd' qui contient des catégories (format text)
  # On utilise un encodeur de scikit-learn
  encoder = LabelEncoder()
  # Forcer toutes les données en type float à 32 bits
  vals = vals.astype('float32')
  # En bonus, on effectuera la normalisation des donneées entre [0, 1]
  # On utilise une fonction de normalisation de scikit-learn
  scaler = MinMaxScaler(feature_range = (0, 1))
  vals_normalisees = scaler.fit_transform(vals)

  return vals_normalisees

# --------------------------------------------------------------------
# conversion_format
# --------------------------------------------------------------------
def conversion_format(donnees):
  # -------------------------------------------------------------------------
  # On est prêt pour la conversion des données en format convenable pour
  # l'apprentissage supervisé. 
  # Pour cet exemple, on décale l'entrée de 3 et la sortie de 1. Le datafrane
  # aura alors (décalage x nb. signaux) + nb. signaux colonnes. Les colonnes
  # du datafrane vals_converties sont comme suit:
  #   s1(t-3)...s8(t-3) s1(t-2)...s8(t-2) s1(t-1)...s8(t-1) s1(t)...s8(t)
  # -------------------------------------------------------------------------
  print('Décalage en entrée = 3, décalage en sortie = 1 - 1 = 0')
  vals_converties = ss(donnees, entree, sortie)

  return vals_converties


def main():
  # -----------------------------------------------------------------------------
  # 0) Lire et normaliser les données
  # -----------------------------------------------------------------------------
  print("Lire les données du ficher 'da.csv' et les normaliser...")
  donnees_normalisees = traitement_donnees()

  # -----------------------------------------------------------------------------
  # 1) Convertir les données normalisées en format de décalage
  # -----------------------------------------------------------------------------
  print("Convertir les données en format de décalage...")
  donnees_converties = conversion_format(donnees_normalisees)
  # Montrer quelques valeurs des signaux convertis
  print('Colonnes: 0 à 7')
  print(donnees_converties.iloc[:,0:entree*3])
  # Ensuite montrer les 8 colonnes de sortie
  print('Colonnes: 8 à 15')
  print(donnees_converties.iloc[:,entree*3:])
  print('Colonnes: 16 à 23')
  print(donnees_converties.iloc[:,entree*3:])
  print('Sortie (da):')
  print(donnees_converties.iloc[:,(sortie+entree)*3:])
  print(donnees_converties)

  # -------------------------------------------------------------------------
  # 2) Diviser les données en deux ensembles: apprentissage et validation
  # -------------------------------------------------------------------------
  # Note: Ces ensembles seront des tableaux numpy pour être compatible avec
  #       keras
  # ------------------------------------------------------------------------- 
  print("Diviser les données en ensemble d'apprentissage et ensemble de validation")
  apprentissage = donnees_converties.values[:1500, :]
  validation = donnees_converties.values[1500:, :]
  # Pour chaque ensemble, séparer en X (entrée) et Y (sortie)
  apprentissage_X, apprentissage_Y = apprentissage[:, :entree*3], apprentissage[:, entree*3:(entree+sortie)*3]
  validation_X, validation_Y = validation[:, :entree*3], validation[:, entree*3:(entree+sortie)*3]

  # -------------------------------------------------------------------------
  # Keras exige des tenseurs 3D en entrée pour l'apprentissage alors il
  # faut reformater les tableaux selon le shape suivant:
  #   (Taille de l'échantillon, pas de temps, nombre de valeurs dans le pas)
  # ------------------------------------------------------------------------- 
  print('Redimensionner les tenseurs selon le shape suivant:')
  print("(Taille de l'échantillon, pas de temps, nombre de valeurs dans le pas)")
  apprentissage_X = apprentissage_X.reshape((apprentissage_X.shape[0], 1,
                                             apprentissage_X.shape[1]))
  validation_X = validation_X.reshape((validation_X.shape[0], 1, validation_X.shape[1]))
  print(F"Le shape du tenseur d'apprentissage X: {apprentissage_X.shape}")
  print(F"Le shape du tenseur d'apprentissage Y: {apprentissage_Y.shape}")
  print(F"Le shape du tenseur de validation X: {validation_X.shape}")
  print(F"Le shape du tenseur de validation Y: {validation_Y.shape}")

  # --------------------------------------------------------------------
  # 3) Définir et compiler le réseau
  # --------------------------------------------------------------------
  # Pour cet exemple on utilisera 1 couche LSTM à 50 cellules de némoire.
  # --------------------------------------------------------------------
  print('Construire le modèle prédictif...')
  reseau = Sequential()
  reseau.add(LSTM(50, return_sequences = True, input_shape=(apprentissage_X.shape[1], apprentissage_X.shape[2])))
  for i in range(couche-1):
    reseau.add(LSTM(50,return_sequences = True))
  reseau.add(LSTM(50))
  reseau.add(Dense(sortie*3))
  reseau.compile(loss='mae', optimizer='adam')

  # --------------------------------------------------------------------
  # 4) Entraîner le réseau
  # --------------------------------------------------------------------
  # Taille des lots: 72, Nombre d'epoch: 50.
  # Utiliser un ensemble de validation pour évaluer avec la performance
  # de l'apprentissage.
  # --------------------------------------------------------------------
  fonction_es=EarlyStopping(monitor='val_loss',mode='min', verbose=1, patience=30)
  print("Entraîner le modèle prédictif avec des lots de 72, 50 epochs et afficher l'historique...")
  history = reseau.fit(apprentissage_X, apprentissage_Y, epochs = 500, batch_size = 72,\
                      validation_data = (validation_X, validation_Y), verbose = True,\
                      shuffle = False, callbacks=[fonction_es])

  # --------------------------------------------------------------------
  # 5) Valider le réseau
  # --------------------------------------------------------------------
  print("Valider le réseau...")
  mae = 0
  Y_predit = reseau.predict(validation_X, verbose = 0)
  mae += mean_absolute_error(validation_Y, Y_predit)
  print(F'Validation du réseau: MAE = {mean(mae):.3f}')

  return history


# *****************************************************************************
# Fonction principale
# *****************************************************************************
if __name__ == "__main__":
  # -----------------------------------------------------------------------------
  # Démarrer la mesure de la durée d'exécution
  # -----------------------------------------------------------------------------
  start = time.perf_counter()
  hist = main()
  # -----------------------------------------------------------------------------
  # Calculer et afficher la durée d'exécution
  # -----------------------------------------------------------------------------
  elapsed = time.perf_counter() - start
  print(F"<Durée d'exécution : {elapsed:.1f} secondes>")

  # -----------------------------------------------------------------------------
  # Afficher l'historique de l'apprentissage
  # -----------------------------------------------------------------------------
  pyplot.plot(hist.history['loss'], label='apprentissage')
  pyplot.plot(hist.history['val_loss'], label='validation')
  pyplot.legend(), pyplot.title('Erreur absolue moyenne (MAE)')
  pyplot.xlabel('Epoch'), pyplot.ylabel('MAE')
  print('Fermer la fenêtre du graphique pour terminer...')
  pyplot.show()
