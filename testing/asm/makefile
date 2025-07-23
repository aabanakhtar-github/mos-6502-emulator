# 6502 build Makefile for ca65 / ld65

# === File names ===
SRC     := basic_file.S
OBJ     := basic_file.o
BIN     := basic_file.bin
CFG     := link.cfg

# === Tools ===
CA65    := ca65
LD65    := ld65

# === Targets ===

all: $(BIN)

$(OBJ): $(SRC)
	$(CA65) $(SRC) -o $(OBJ)

$(BIN): $(OBJ) $(CFG)
	$(LD65) -C $(CFG) $(OBJ) -o $(BIN)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
