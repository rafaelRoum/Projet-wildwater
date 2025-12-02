#!/bin/bash
# ======================================================
# Projet C-WildWater - Script Shell Principal
# Auteur : A COMPLETER
# Rôle : Orchestration des traitements histo et leaks
# ======================================================

# ----------- CONFIGURATION ------------
EXEC="./bin/progC"          # Exécutable du programme C
MAKE_CMD="make"             # Commande de compilation
DATA_FILE="data.csv"       # Fichier CSV d'entrée
HISTO_DIR="histo"           # Dossier de sortie des histogrammes
LEAKS_FILE="leaks.dat"     # Historique des fuites
GNUPLOT_SCRIPT="plot.gp"   # Script gnuplot temporaire
# --------------------------------------

START=$(date +%s)

# ----------- FONCTIONS UTILITAIRES ------------

error_exit() {
    echo "[ERREUR] $1" >&2
    END=$(date +%s)
    echo "Temps d'execution : $((END - START)) secondes"
    exit 1
}

check_exec() {
    if [ ! -f "$EXEC" ]; then
        echo "[INFO] Exécutable absent. Compilation en cours…"
        $MAKE_CMD
        if [ $? -ne 0 ]; then
            error_exit "Echec de la compilation via make"
        fi
    fi
}

create_dirs() {
    if [ ! -d "$HISTO_DIR" ]; then
        mkdir -p "$HISTO_DIR"
    fi
}

# ----------- VERIFICATION DES ARGUMENTS ------------

if [ $# -lt 1 ]; then
    error_exit "Aucun argument fourni"
fi

MODE="$1"

# ----------- MODE HISTO ------------

if [ "$MODE" = "histo" ]; then

    if [ $# -ne 2 ]; then
        error_exit "Usage : $0 histo {max|src|real}"
    fi

    TYPE="$2"

    if [[ "$TYPE" != "max" && "$TYPE" != "src" && "$TYPE" != "real" ]]; then
        error_exit "Type d'histogramme invalide : $TYPE"
    fi

    create_dirs
    check_exec

    OUT_FILE="$HISTO_DIR/histo_$TYPE.dat"
    IMG_FILE="$HISTO_DIR/histo_$TYPE.png"

    echo "[INFO] Generation des donnees histogramme ($TYPE)"
    $EXEC histo "$TYPE" "$DATA_FILE" "$OUT_FILE"

    if [ $? -ne 0 ]; then
        error_exit "Erreur renvoyee par le programme C (histo)"
    fi

    # ----------- GENERATION DU GRAPHIQUE GNUPLOT ------------

    echo "[INFO] Generation du graphique PNG"

    cat > "$GNUPLOT_SCRIPT" << EOF
set terminal png size 1200,800
set output "$IMG_FILE"
set datafile separator ";"
set style data histograms
set style fill solid
set boxwidth 0.9
set grid ytics
set title "Histogramme des usines - $TYPE"
set xlabel "Usines"
set ylabel "Volume"
plot "$OUT_FILE" using 2:xtic(1) title "$TYPE"
EOF

    gnuplot "$GNUPLOT_SCRIPT"
    if [ $? -ne 0 ]; then
        error_exit "Erreur lors de l'execution de gnuplot"
    fi

    rm -f "$GNUPLOT_SCRIPT"

    echo "[OK] Histogramme genere : $IMG_FILE"

# ----------- MODE LEAKS ------------

elif [ "$MODE" = "leaks" ]; then

    if [ $# -ne 2 ]; then
        error_exit "Usage : $0 leaks \"<IDENTIFIANT_USINE>\""
    fi

    IDENT="$2"

    check_exec

    echo "[INFO] Calcul des fuites pour $IDENT"

    RESULT=$($EXEC leaks "$IDENT" "$DATA_FILE")
    RET=$?

    if [ $RET -ne 0 ]; then
        error_exit "Erreur renvoyee par le programme C (leaks)"
    fi

    if [ ! -f "$LEAKS_FILE" ]; then
        echo "identifier ; Leak volume (M.m3.year-1)" > "$LEAKS_FILE"
    fi

    echo "$IDENT ; $RESULT" >> "$LEAKS_FILE"

    echo "[OK] Resultat enregistre dans $LEAKS_FILE"

# ----------- MODE INCONNU ------------

else
    error_exit "Commande inconnue : $MODE"
fi

# ----------- FIN DU SCRIPT ------------

END=$(date +%s)
echo "Temps d'execution : $((END - START)) secondes"
exit 0
