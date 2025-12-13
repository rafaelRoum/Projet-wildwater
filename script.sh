#!/bin/bash

# --- 1. Démarrage du chronomètre (Nanosecondes) ---
start_time=$(date +%s%3N)

# Fonction pour gérer la fin du script (calcul du temps)
finish() {
    end_time=$(date +%s%3N)
    duration=$((end_time - start_time))
    echo "Durée totale : ${duration} ms"
}

# On s'assure que 'finish' est appelé même en cas d'erreur (exit)
trap finish EXIT

# --- 2. Vérification des arguments ---
# Usage attendu : ./myScript.sh data.csv commande [option]
if [ $# -lt 2 ]; then
    echo "Erreur : Arguments insuffisants."
    echo "Usage : $0 <chemin_data.csv> histo <max|src|real|all>"
    echo "Usage : $0 <chemin_data.csv> leaks <Identifiant_Usine>"
    exit 1
fi

FICHIER_DATA="$1"
COMMANDE="$2"
OPTION="$3"

if [ ! -f "$FICHIER_DATA" ]; then
    echo "Erreur : Le fichier '$FICHIER_DATA' est introuvable."
    exit 1
fi

# Création des dossiers
mkdir -p sortie
mkdir -p graphs

# --- 3. Compilation ---
EXECUTABLE="./c-wildwater"
if [ ! -f "$EXECUTABLE" ]; then
    echo "Compilation en cours..."
    make
    if [ $? -ne 0 ]; then
        echo "Erreur : Échec de la compilation."
        exit 1
    fi
fi

# --- 4. Traitement ---

if [ "$COMMANDE" = "histo" ]; then
    if [[ "$OPTION" != "max" && "$OPTION" != "src" && "$OPTION" != "real" && "$OPTION" != "all" ]]; then
        echo "Erreur : Option invalide pour histo (max, src, real, all)."
        exit 1
    fi

    # Appel C
    $EXECUTABLE "$FICHIER_DATA" histo "$OPTION"
    if [ $? -ne 0 ]; then exit 1; fi

    NOM_FICHIER_DAT="histo_$OPTION.dat"
    if [ -f "$NOM_FICHIER_DAT" ]; then
        mv "$NOM_FICHIER_DAT" "sortie/$NOM_FICHIER_DAT"
    else
        echo "Erreur de génération du fichier .dat"
        exit 1
    fi

    # --- GNUPLOT : 10 plus grands & 50 plus petits ---
    if [ "$OPTION" != "all" ]; then
        echo "Génération des graphiques..."
        INPUT="sortie/$NOM_FICHIER_DAT"
        
        # 1. Préparer les données
        # Trier décroissant (Top 10)
        tail -n +2 "$INPUT" | sort -t';' -k2nr | head -n 10 > sortie/top10.dat
        # Trier croissant (Top 50 min)
        tail -n +2 "$INPUT" | sort -t';' -k2n | head -n 50 > sortie/min50.dat

        # 2. Graphique 1 : 10 Plus Grandes
        gnuplot -e "
            set terminal png size 1000,600;
            set output 'graphs/histo_${OPTION}_top10.png';
            set title 'Top 10 : Plus grandes capacités ($OPTION)';
            set style data histograms;
            set style fill solid 1.0 border -1;
            set boxwidth 0.7;
            set datafile separator ';';
            set xtics rotate by -45;
            set ylabel 'Volume (M.m3)';
            plot 'sortie/top10.dat' using 2:xtic(1) title 'Volume' lc rgb '#2E86C1';
        "

        # 3. Graphique 2 : 50 Plus Petites
        gnuplot -e "
            set terminal png size 1200,600;
            set output 'graphs/histo_${OPTION}_min50.png';
            set title 'Top 50 : Plus petites capacités ($OPTION)';
            set style data histograms;
            set style fill solid 1.0 border -1;
            set boxwidth 0.7;
            set datafile separator ';';
            set xtics rotate by -90 font ',8';
            set ylabel 'Volume (M.m3)';
            plot 'sortie/min50.dat' using 2:xtic(1) title 'Volume' lc rgb '#E74C3C';
        "

        # Nettoyage temporaire
        rm sortie/top10.dat sortie/min50.dat
    fi

elif [ "$COMMANDE" = "fuites" ]; then
    if [ -z "$OPTION" ]; then
        echo "Erreur : Il faut l'ID de l'usine."
        exit 1
    fi

    $EXECUTABLE "$FICHIER_DATA" fuites "$OPTION"
    
    # Gestion de l'historique dans sortie/
    if [ -f "fuites_output.dat" ]; then
        LOG="sortie/fuites.log"
        if [ ! -f "$LOG" ]; then
             head -n 1 "fuites_output.dat" > "$LOG"
        fi
        tail -n 1 "fuites_output.dat" >> "$LOG"
        rm "fuites_output.dat"
    else
        # Si le fichier n'est pas créé, c'est que l'usine n'existe pas ou erreur
        # Le script C a dû afficher "0" sur la sortie standard
        echo "Aucune fuite détectée ou usine introuvable."
    fi

else
    echo "Erreur : Commande '$COMMANDE' inconnue."
    exit 1
fi

# Le 'trap finish EXIT' gérera l'affichage du temps ici