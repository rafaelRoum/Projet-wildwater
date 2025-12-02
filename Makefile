CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lm
EXEC = c-water

# Source and Object files definitions
SRC_DIR = src
OBJ_DIR = .
SRC_FILES = main.c avl.c leaks.c
OBJ_FILES = $(patsubst %.c, %.o, $(SRC_FILES))

# Main target (linking step)
all: $(EXEC)

$(EXEC): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile objects in the src/ directory
%.o: $(SRC_DIR)/%.c $(SRC_DIR)/avl.h $(SRC_DIR)/leaks.h
	$(CC) $(CFLAGS) -c $< -o $@

# Phony targets
clean:
	rm -f *.o $(EXEC) $(OBJ_DIR)/*.o
	rm -f data/*.dat graphs/*.png

.PHONY: all clean