# Ce script vérifie les arguments entrés, compile le programme C, 
# appelle le programme C (histo ou fuites), génère les fichiers et graphiques,
# mesure le temps total d'exécution


# Démarrage du chronomètre en milisecondes
start_time=$(date +%s%3N) # Prend en compte le temps actuel

# Fonction pour temps d'execution
finish() {
    end_time=$(date +%s%3N)
    duration=$((end_time - start_time))
    echo "Durée totale : ${duration} ms"
}


# Garantir l'exécution de 'finish' même en cas d'erreur (exit)
trap finish EXIT

# Vérification des arguments 

if [ $# -lt 2 ]; then 
    echo "Erreur : Arguments insuffisants."
    echo "Usage : $0 <chemin_data.csv> histo <max|src|real|all>"
    echo "Usage : $0 <chemin_data.csv> leaks <Identifiant_Usine>"
    exit 1
fi

# Récupération des arguments

FICHIER_DATA="$1" # $1 = fichier de données
COMMANDE="$2" # $2 = Commande (histo ou fuites)
OPTION="$3" # $3 = Option (type histo ou ID usine)

# On vérifie que le fichier de données existe bien 
if [ ! -f "$FICHIER_DATA" ]; then
    echo "Erreur : Le fichier '$FICHIER_DATA' est introuvable."
    exit 1
fi

# Création des dossiers de sortie si inexistants
mkdir -p sortie 
mkdir -p graphs

# Compilation du programme C
EXECUTABLE="./c-wildwater"

# Dans le cas où l'exécutable n'existe pas, on lance le make
if [ ! -f "$EXECUTABLE" ]; then
    echo "Compilation en cours..."
    make


    # Vérification erreur compilation
    if [ $? -ne 0 ]; then
        echo "Erreur : Échec de la compilation."
        exit 1
    fi
fi

# Traitement selon la commande 
# Mode histogramme
if [ "$COMMANDE" = "histo" ]; then

    # vérification de l'option histo
    if [[ "$OPTION" != "max" && "$OPTION" != "src" && "$OPTION" != "real" && "$OPTION" != "all" ]]; then
        echo "Erreur : Option invalide pour histo (max, src, real, all)."
        exit 1
    fi

    # Appel du programme C
    $EXECUTABLE "$FICHIER_DATA" histo "$OPTION"
    if [ $? -ne 0 ]; then exit 1; fi

    NOM_FICHIER_DAT="histo_$OPTION.dat"

    # Vérification et déplacement du fichier généré
    if [ -f "$NOM_FICHIER_DAT" ]; then
        mv "$NOM_FICHIER_DAT" "sortie/$NOM_FICHIER_DAT"
    else
        echo "Erreur de génération du fichier .dat"
        exit 1
    fi

    # Génération des graphiques  
    if [ "$OPTION" != "all" ]; then
        echo "Génération des graphiques..."
        INPUT="sortie/$NOM_FICHIER_DAT"
        
        # Suppresion de l'en-tête puis tri des données
        # Top 10 des plus grandes valeurs 
        # Trier décroissant 
        tail -n +2 "$INPUT" | sort -t';' -k2nr | head -n 10 > sortie/top10.dat
        
        # Top 50 des plus petites valeurs 
        # Trier croissant 
        tail -n +2 "$INPUT" | sort -t';' -k2n | head -n 50 > sortie/min50.dat

        # Graphique 1 : 10 Plus Grandes usines
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

        # Graphique 2 : 50 Plus Petites usines 
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

        # Nettoyage temporaire des fichiers 
        rm sortie/top10.dat sortie/min50.dat
    fi
#Mode fuites
elif [ "$COMMANDE" = "fuites" ]; then

    # Vérification de l'identifiant
    if [ -z "$OPTION" ]; then
        echo "Erreur : Il faut l'ID de l'usine."
        exit 1
    fi

    # Appel du programme C qui calcul des fuites 
    $EXECUTABLE "$FICHIER_DATA" fuites "$OPTION"
    
    # Gestion du fichier de sortie généré par le C
    if [ -f "fuites_output.dat" ]; then
        LOG="sortie/fuites.log"

        # Création de l'en-tête si le fichier n'existe pas encore 
        if [ ! -f "$LOG" ]; then
             head -n 1 "fuites_output.dat" > "$LOG"
        fi

        # Ajout du dernier résultat
        tail -n 1 "fuites_output.dat" >> "$LOG"
        rm "fuites_output.dat"
    else
        # Si le fichier n'est pas créé, c'est que l'usine n'existe pas ou erreur
        # Le script C a dû afficher "0" sur la sortie standard
        echo "Aucune fuite détectée ou usine introuvable."
    fi

# Commmande invalide
else
    echo "Erreur : Commande '$COMMANDE' inconnue."
    exit 1
fi

# Le 'trap finish EXIT' gérera l'affichage du temps ici 
