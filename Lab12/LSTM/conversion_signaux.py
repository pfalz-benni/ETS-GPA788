#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''conversion_signaux convertir les signaux dans le format de décalage (lag method)


-------------------------------------------------------------------------------
Ce code est un moduleservant à convertir les données multidimensionnelles pour
l'apprentissage supervisé par une fenêtre glissante (lag method).
-------------------------------------------------------------------------------

'''

# Pour utiliser pandas ...
from pandas import read_csv
from pandas import DataFrame
from pandas import concat
# Pour utiliser sklearn pour transformer les données
from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import MinMaxScaler

# -----------------------------------------------------------------------------
# signaux_apprentissage_supervise
# -----------------------------------------------------------------------------
# Voir les notes de cours "Keras LSTM III" pour 
# -----------------------------------------------------------------------------
def signaux_apprentissage_supervise(data, largeur_fen_entree = 1, largeur_fen_sortie = 1, enlever_nan = True):
  '''Fonction servant à décaler les données par une fenêtre glissante.

  Entrées: 
    - data: un tableau multidimensionnel numpy
    - largeur_fen_entree: largeur de la fenêtre pour l'entrée
    - largeur_fen_sortie: (largeur de la fenêtre pour la sortie - 1) <<- Attention!!
    - enlever_nan: enlever ou non les nombres NAN

  Sortie:
    - un dataframe contenant les données décalées
  -----------------------------------------------------------------------------
  Voici un exemple à 2 signaux et un déclagage de M = 3 (M est largeur de la fenêtre).


  t  s1  s2  =>  t  s1(t-3)  s2(t-3)  s1(t-2)  s2(t-2)  s1(t-1)  s2(t-1)  s1(t)  s2(t)
  0  a  f        0   NA       NA       NA       NA       NA       NA       a      f
  1  b  g        1   NA       NA       NA       NA       a        f        b      g
  2  c  h        2   NA       NA       a        f        b        g        c      h
  3  d  i        3   a        f        b        g        c        h        d      i
  4  e  j        4   b        g        c        h        d        i        e      j
                 5   c        h        d        i        e        j        NA     NA
                 6   d        i        e        j        NA       NA       NA     NA
                 7   e        j        NA       NA       NA       NA       NA     NA

  Si enlever_nan == True alors les lignes contenant au moins un NA seront enlevés.
  '''

  # Combien de variables (signaux) dans data?
  n_vars = 1 if type(data) is list else data.shape[1]
  # Convertir en dataframe de pandas
  df = DataFrame(data)
  cols, names = list(), list()
  # Pour les entrées: Décaler l'entrée vers le passé (vers le bas).
  # Il y aura (largeur_fen_entree) x (nb. signaux) colonnes.
  # De gauche à droite, chaque groupe de (nb. signaux) colonnes est décalé
  # de largeur_fen_entree à 0 pas vers le bas.
  for i in range(largeur_fen_entree, 0, -1):
    cols.append(df.shift(i))
    # former des étiquettes de colonnes en conséquence...
    names += [('var%d(t-%d)' % (j+1, i)) for j in range(n_vars)]
  # Pour les sorties: Décaler la sortie vers le futur (vers le haut).
  # Il y aura (largeur_fen_sortie) x (nb. signaux à prédire) colonnes.
  # De gauche à droite, chaque groupe de (nb. signaux à prédire) colonnes 
  # est décalé de 0 à largeur_fen_sortie pas vers le haut.
  for i in range(0, largeur_fen_sortie):
    cols.append(df.shift(-i))
    if i == 0:
      names += [('var%d(t)' % (j+1)) for j in range(n_vars)]
    else:
      names += [('var%d(t+%d)' % (j+1, i)) for j in range(n_vars)]
  # Mettre le tout dans un dataframe
  donnees = concat(cols, axis=1)
  donnees.columns = names
  # Doit-on enlever les lignes avec NA?
  if enlever_nan:
    donnees.dropna(inplace = True)
  return donnees
 
# *****************************************************************************
# Tester la fonction de conversion
# Exécuter ce module pour voir un exemple de la fonction de conversion à
# l'oeuvre.
# *****************************************************************************
if __name__ == '__main__':
  # Charger le fichier de données
  data = read_csv('da.csv', header = 0, index_col = 1, encoding = 'ISO-8859-1')
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
	# -------------------------------------------------------------------------
	# On est prêt pour la conversion des données en format convenable pour
	# l'apprentissage supervisé. 
	# Pour cet exemple, on décale l'entrée de 2 et la sortie de 1.
	# -------------------------------------------------------------------------
  decalage_entree = 2; decalage_sortie = 1
  vals_converties = signaux_apprentissage_supervise(vals_normalisees,
                    decalage_entree, decalage_sortie)
	# D'abord montrer les 16 colonnes d'entrée (5 lignes)
  print(F'Données en entrée (décalage de {decalage_entree}):')
  print(vals_converties.iloc[:,0:6])
	# Ensuite montrer les 8 colonnes de sortie
  print(F'Données de sortie (décalage de {decalage_sortie - 1}):')
  print(vals_converties.iloc[:,6:9])
 