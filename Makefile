CC = gcc
FLEX = flex
BISON = bison
OUTPUT = cml.out

# Optional flags passed via ARGS
ARGS ?=
# ARGS=--ts --tp --ta

CXXFLAGS = -std=c2x -pedantic -Wall -Wextra -Wconversion -g -Isrc
CXXFLAGS_WNO = -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion
LDFLAGS = -lm -lfl
# CXXFLAGS = -Werror -Wdouble-promotion -g3 -Wfloat-equal -fsanitize=leak,address -fsanitize-trap=undefined
EXTRAFLAGS= -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L

SRC_DIR := src
LEX_C   := $(SRC_DIR)/lex.yy.c
BISON_C := $(SRC_DIR)/parser.tab.c
BISON_H := $(SRC_DIR)/parser.tab.h
BISON_O := $(SRC_DIR)/parser.output
ALL_C := $(shell find $(SRC_DIR) -type f -name '*.c')
SRC   := $(filter-out $(LEX_C) $(BISON_C),$(ALL_C))
HEADERS := $(wildcard src/*.h)
EXAMPLES := $(wildcard example/*.c example/*.cm)

# Default rule
all: $(OUTPUT)

$(LEX_C): $(SRC_DIR)/scan.l
	$(FLEX) -o $@ $<

$(BISON_C) $(BISON_H): $(SRC_DIR)/parser.y
	$(BISON) -v -d -o $(BISON_C) $<

$(OUTPUT): $(SRC) $(HEADERS) $(LEX_C) $(BISON_H) $(BISON_C)
	$(CC) -o $@ $(SRC) $(LEX_C) $(BISON_C) $(CXXFLAGS) $(CXXFLAGS_WNO) $(LDFLAGS) $(EXTRAFLAGS)

# Run the program with a single file from the "example" directory
example-file: $(OUTPUT)
	@indir="example"; \
	read -p "Enter filename (in '$$indir'): " filename; \
	file="$$indir/$$filename"; \
	if [ -f "$$file" ]; then \
		echo "=== Running: ./$(OUTPUT) $(ARGS) $$file ==="; \
		./$(OUTPUT) $(ARGS) "$$file"; \
	else \
		echo "File not found: $$file"; \
	fi

# Run the program with a single file chosen from the "example" directory
run-file: $(OUTPUT)
	@files="$(EXAMPLES)"; \
	count=1; \
	for f in $$files; do \
		echo "[$$count] $$f"; \
		count=$$((count+1)); \
	done; \
	read -p "Select a file number: " choice; \
	selected=$$(echo $$files | cut -d' ' -f$$choice); \
	if [ -z "$$selected" ]; then \
		echo "Invalid choice"; \
		exit 1; \
	fi; \
	mkdir -p asm; \
	basefile=$$(basename $$selected); \
	outflag="-o asm/$${basefile%.*}.asm"; \
	echo "=== Running: ./$(OUTPUT) $$outflag $$selected ==="; \
	./$(OUTPUT) $$outflag "$$selected"

# Run the program with Valgrind on a selected file from "example"
debug: $(OUTPUT)
	@files="$(EXAMPLES)"; \
	count=1; \
	for f in $$files; do \
		echo "[$$count] $$f"; \
		count=$$((count+1)); \
	done; \
	read -p "Select a file number: " choice; \
	selected=$$(echo $$files | cut -d' ' -f$$choice); \
	if [ -z "$$selected" ]; then \
		echo "Invalid choice"; \
		exit 1; \
	fi; \
	mkdir -p asm; \
	basefile=$$(basename $$selected); \
	outflag="-o asm/$${basefile%.*}.asm"; \
	echo "=== Debugging with Valgrind: valgrind --leak-check=full --track-origins=yes ./$(OUTPUT) $$outflag $$selected ==="; \
	valgrind --leak-check=full --track-origins=yes ./$(OUTPUT) $$outflag "$$selected"

# Run the program on all files in the "example" directory, optionally saving output
example: $(OUTPUT)
	@indir="example"; \
	outdir="results"; \
	if [ -d "$$indir" ]; then \
		mkdir -p "asm"; \
		if [ "$(SAVE)" = "1" ]; then mkdir -p "$$outdir"; fi; \
		for f in "$$indir"/*.c "$$indir"/*.cm; do \
			if [ -f "$$f" ]; then \
				basefile=$$(basename "$$f"); \
				outflag="-o asm/$${basefile%.*}.asm"; \
				if [ "$(SAVE)" = "1" ]; then \
					outfile="$$outdir/$${basefile%.*}.txt"; \
					echo "=== Saving: ./$(OUTPUT) $(ARGS) $$outflag $$f > $$outfile ==="; \
					./$(OUTPUT) $(ARGS) $$outflag "$$f" > "$$outfile"; \
				else \
					echo "=== Running: ./$(OUTPUT) $(ARGS) $$outflag $$f ==="; \
					./$(OUTPUT) $(ARGS) $$outflag "$$f"; \
				fi; \
			fi; \
		done \
	else \
		echo "Directory not found: $$indir"; \
	fi

# Clean build files
clean:
	rm -f $(OUTPUT) $(LEX_C) $(BISON_C) $(BISON_H) $(BISON_O) $(OUTPUT)
	rm -rf asm results

.PHONY: all run-file run-dir clean example
