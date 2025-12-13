# Compilateur
CC = gcc

# Options : -Isrc pour trouver les headers
CFLAGS = -Isrc -O3

# Exécutable
EXEC = c-wildwater

# Sources
SRC = src/main.c src/avl.c src/histo.c src/fuites.c
OBJ = $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(EXEC) graphs/* sortie/*

re: clean all

.PHONY: all clean re