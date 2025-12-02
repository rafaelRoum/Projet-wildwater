#!/bin/bash

# --- Configuration ---
CSV_FILE="data/input.dat" # Assuming the input file is named input.dat in data/
OUTPUT_FILE="data/output_histo.dat"
LEAKS_HISTORY="data/leaks_history.dat"
START_TIME=$(date +%s)
# ---------------------

# 1. Check folder structure and create if missing
mkdir -p src data graphs

# 2. Check arguments
if [ $# -lt 2 ]; then
    echo "Usage: $0 <mode> [arg]"
    echo "Example: $0 histo max"
    exit 1
fi

MODE=$1
ARG=$2

# 3. Compilation check
if [ ! -f "c-water" ]; then
    echo "Executable 'c-water' not found. Compiling..."
    make
    if [ $? -ne 0 ]; then echo "Compilation failed."; exit 1; fi
fi

# 4. Execute Mode
if [ "$MODE" = "histo" ]; then
    if [[ "$ARG" != "max" && "$ARG" != "src" && "$ARG" != "real" ]]; then
        echo "Invalid argument for histo. Use: max, src, or real."
        exit 1
    fi

    echo "Running Histo analysis ($ARG)..."
    ./c-water "$CSV_FILE" "histo"
    CODE_RET=$?

    if [ $CODE_RET -ne 0 ]; then echo "C program failed."; exit 1; fi
    if [ ! -f "$OUTPUT_FILE" ]; then echo "Output file not found."; exit 1; fi

    # Gnuplot preparation (ID;Cap;Src;Real)
    COL=2
    if [ "$ARG" = "src" ]; then COL=3; fi
    if [ "$ARG" = "real" ]; then COL=4; fi

    tail -n +2 "$OUTPUT_FILE" | sort -t';' -k${COL}n > data/sorted.tmp
    
    # Get Top 5 and Flop 5
    head -n 5 data/sorted.tmp > graphs/plot.dat
    tail -n 5 data/sorted.tmp >> graphs/plot.dat

    # Plot
    gnuplot -persist <<-EOFMarker
        set terminal png size 1000,600
        set output 'graphs/histo_$ARG.png'
        set title "Factory Overview ($ARG)"
        set style data histograms
        set style fill solid
        set datafile separator ";"
        set ylabel "Volume (m3)"
        set xtic rotate by -45 scale 0
        plot 'graphs/plot.dat' using $COL:xtic(1) title '$ARG' lc rgb "blue"
EOFMarker
    
    echo "Graph generated: graphs/histo_${ARG}.png"
    rm data/sorted.tmp graphs/plot.dat

elif [ "$MODE" = "leaks" ]; then
    if [ -z "$ARG" ]; then echo "Missing Factory ID."; exit 1; fi
    
    echo "Calculating leaks for $ARG..."
    RES=$(./c-water "$CSV_FILE" "leaks" "$ARG")
    CODE_RET=$?
    
    if [ "$RES" = "-1" ]; then
        echo "Error: Factory ID '$ARG' not found in data."
        VAL="-1.0"
    else
        # Output is in M.m3 according to project specification
        LEAK_VOL=$(echo "$RES / 1000000" | bc -l)
        echo "Total estimated leakage: $RES m3"
        echo "Reported volume (M.m3): $LEAK_VOL"
        VAL="$LEAK_VOL"
    fi
    
    # Update history file
    echo "$ARG;$VAL" >> "$LEAKS_HISTORY"
    echo "Leak data logged to $LEAKS_HISTORY."

else
    echo "Unknown command."
fi

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
echo "Total execution time: ${DURATION}s"