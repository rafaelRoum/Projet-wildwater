# 🌊 C-WildWater Project

Ce projet consiste à développer un outil d'analyse et de synthèse de données pour un système de distribution d'eau, entièrement codé en langage C et piloté par un script Shell.
L'objectif est de traiter un volume massif de données (CSV) pour en extraire des statistiques clés :
    Histogrammes : Classement des stations (usines, stations de traitement) selon leurs capacités ou volumes.
    Détection de fuites : Analyse du réseau (modélisé par un graphe) pour calculer les pertes d'eau en aval d'une usine spécifique.

Le projet met en pratique des structures de données avancées (Arbres AVL, Graphes, Listes chaînées) et l'automatisation via Shell et Makefile.

## 📁 Contenu du Projet

Le répertoire est structuré comme suit :
    src/ : Contient tous les fichiers sources (.c) et les en-têtes (.h).
    data.csv : Le fichier de données brut (entrée du programme).
    script.sh : Le script d'automatisation principal (lance la compilation, le traitement et la génération de graphiques).
    Makefile : Fichier de configuration pour la compilation automatique du code C.
    graphs/ : Dossier généré contenant les graphiques au format .png (Gnuplot).
    sortie/ : Dossier généré contenant les fichiers de résultats .dat et le journal des fuites.

## 🛠️ Pré-requis

Pour fonctionner, ce projet nécessite un environnement Linux (ou WSL sous Windows) avec les paquets suivants :

    gcc (Compilateur C)
    make
    gnuplot (Pour la génération des graphiques)

#### Installation des dépendances (LINUX) :

    sudo apt update
    sudo apt install build-essential gnuplot-qt

## 🚀 Installation

#### Clonez le dépôt :
#####  git clone https://github.com/rafaelRoum/Projet-wildwater.git
#####  cd ./chemin/Projet-wildwater

#### Donnez les droits d'exécution au script principal :
 ##### chmod +x myScript.sh

## ⚙️ Utilisation

#### 1. Générer des Histogrammes (histo)
Options disponibles :

    max : Trie selon la capacité maximale des stations.
    src : Trie selon le volume de source (captage).
    real : Trie selon le volume réellement traité.
    all : Génère un fichier de données complet (Bonus).

##### Exemple :
##### ./myScript.sh data.csv histo max

#### 2. Calculer les Fuites (fuites)

##### ./myScript.sh data.csv fuites "Identifiant_Usine"

##### Exemple :

##### ./myScript.sh data.csv fuites "Plant #42"

#### 3. Nettoyage

Pour supprimer les fichiers temporaires de compilation (objets et exécutable) :

###### make clean

Pour supprimer les fihciers de sortie (graphs et sortie):

##### make clear

## 📊 Résultats et Sorties

    Histogrammes : Les images sont sauvegardées dans graphs/ (ex: histo_max_top10.png).

    Log des fuites : Les résultats sont ajoutés au fichier sortie/fuites.log.

    Temps d'exécution : La durée totale du traitement est affichée en millisecondes à la fin de chaque commande.

## 👥 Auteurs

    Rafael Roumiantsev

    Neira Bahri

    Yassine Dhahak
    
Projet de preing 2
