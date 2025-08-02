# Compiler and tools
FLEX=flex
BISON=bison
CC=gcc

# Files
LEX_SRC=scan.l
BISON_SRC=parser.y
MAIN_SRC=main.c
UTILS_SRC=utils.c stack.c queue.c
ANALYSIS_SRC=symtab.c analyze.c
CODE_SRC=ir.c cgen.c
LEX_C=lex.yy.c
BISON_C=parser.tab.c
BISON_H=parser.tab.h
OUTPUT=cmc.out

# Optional flags passed via ARGS
# ARGS ?=
ARGS=--ts --tp --ta

CXXFLAGS = -std=c2x -pedantic -Wall -Wextra -Wconversion -g
CXXFLAGS_WNO = -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion
LDFLAGS = -lm -lfl
# CXXFLAGS = -Werror -Wdouble-promotion -g3 -Wfloat-equal -fsanitize=leak,address -fsanitize-trap=undefined
EXTRAFLAGS= -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L

# Default rule
all: $(OUTPUT)

# Flex and Bison rules
$(LEX_C): $(LEX_SRC)
	$(FLEX) $(LEX_SRC)

$(BISON_C) $(BISON_H): $(BISON_SRC)
	$(BISON) -v -d $(BISON_SRC)

# Compile the frontend
$(OUTPUT): $(MAIN_SRC) $(UTILS_SRC) $(CODE_SRC) $(ANALYSIS_SRC) $(LEX_C) $(BISON_C) $(BISON_H)
	$(CC) -o $(OUTPUT) $(MAIN_SRC) $(UTILS_SRC) $(CODE_SRC) $(ANALYSIS_SRC) $(LEX_C) $(BISON_C) $(CXXFLAGS) $(CXXFLAGS_WNO) $(LDFLAGS) $(EXTRAFLAGS)

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
	@indir="example"; \
	i=0; \
	for f in "$$indir"/*.c "$$indir"/*.cm; do \
		[ -f "$$f" ] || continue; \
		echo "[$$i] $$f"; \
		eval "file_$$i='$$f'"; \
		i=$$((i+1)); \
	done; \
	if [ "$$i" -eq 0 ]; then \
		echo "No .c or .cm files found in $$indir."; \
		exit 1; \
	fi; \
	read -p "Enter the number of the file to run: " idx; \
	eval "selected=\$$file_$$idx"; \
	if [ -n "$$selected" ]; then \
		echo "=== Running: ./$(OUTPUT) $(ARGS) $$selected ==="; \
		./$(OUTPUT) $(ARGS) "$$selected"; \
	else \
		echo "Invalid selection."; \
		exit 1; \
	fi

# Run the program with Valgrind on a selected file from "example"
run-valgrind: $(OUTPUT)
	@indir="example"; \
	i=0; \
	for f in "$$indir"/*.c "$$indir"/*.cm; do \
		[ -f "$$f" ] || continue; \
		echo "[$$i] $$f"; \
		eval "file_$$i='$$f'"; \
		i=$$((i+1)); \
	done; \
	if [ "$$i" -eq 0 ]; then \
		echo "No .c or .cm files found in $$indir."; \
		exit 1; \
	fi; \
	read -p "Enter the number of the file to run with Valgrind: " idx; \
	eval "selected=\$$file_$$idx"; \
	if [ -n "$$selected" ]; then \
		echo "=== Running under Valgrind: ./$(OUTPUT) $(ARGS) $$selected ==="; \
		valgrind --leak-check=full --track-origins=yes ./$(OUTPUT) $(ARGS) "$$selected"; \
	else \
		echo "Invalid selection."; \
		exit 1; \
	fi

# Run the program on all files in the "example" directory, optionally saving output
example: $(OUTPUT)
	@indir="example"; \
	outdir="results"; \
	if [ -d "$$indir" ]; then \
		if [ "$(SAVE)" = "1" ]; then mkdir -p "$$outdir"; fi; \
		for f in "$$indir"/*.c "$$indir"/*.cm; do \
			if [ -f "$$f" ]; then \
				basefile=$$(basename "$$f"); \
				if [ "$(SAVE)" = "1" ]; then \
					outfile="$$outdir/$${basefile%.*}.txt"; \
					echo "=== Saving: ./$(OUTPUT) $(ARGS) $$f > $$outfile ==="; \
					./$(OUTPUT) $(ARGS) "$$f" > "$$outfile"; \
				else \
					echo "=== Running: ./$(OUTPUT) $(ARGS) $$f ==="; \
					./$(OUTPUT) $(ARGS) "$$f"; \
				fi; \
			fi; \
		done \
	else \
		echo "Directory not found: $$indir"; \
	fi

# Clean build files
clean:
	rm -f $(OUTPUT) lex.yy.c parser.tab.c parser.tab.h parser.output

.PHONY: all run-file run-dir clean
