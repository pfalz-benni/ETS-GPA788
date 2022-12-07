#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''Ce programme affiche la version des modules/packages nécessaires pour la partie ML du cours.

Note: Le programme doit être exécuté dans l'environnement virtuel gpa788-lstm et la version des
programmes doit être celle indiquée ci-dessous:

scipy: 1.2.1
numpy: 1.15.4
matplotlib: 3.0.2
pandas: 0.24.1
statsmodels: 0.9.0
scikit-learn: 0.20.2
tensorflow: 1.12.0
keras: 2.1.6-tf

GPA788 Conception et intégration des objets connectés
T. Wong
Juin 2018
Septembre 2020
Novembre 2021
'''
import scipy
print(F'scipy: {scipy.__version__}')
import numpy
print(F'numpy: {numpy.__version__}')
import matplotlib
print(F'matplotlib: {matplotlib.__version__}')
import pandas
print(F'pandas: {pandas.__version__}')
import statsmodels
print(F'statsmodels: {statsmodels.__version__}')
import sklearn
print(F'scikit-learn: {sklearn.__version__}')
import tensorflow
print(F'tensorflow: {tensorflow.__version__}')
# Note technique
# L'intelliSense de VSCode est incapable d'identifier
# correctement l'importation suivante:
#            import tensorflow.keras as keras
# On peut contourner le problème par
#            import tensorflow.python.keras as keras
# ou encore
from tensorflow.python import keras
print(F'keras: {keras.__version__}')