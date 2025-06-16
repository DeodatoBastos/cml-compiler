# Compiler and tools
FLEX=flex
BISON=bison
CC=gcc

# Files
LEX_SRC=scan.l
BISON_SRC=parser.y
MAIN_SRC=main.c
UTILS_SRC=utils.c
LEX_C=lex.yy.c
BISON_C=parser.tab.c
BISON_H=parser.tab.h
OUTPUT=frontend.out

# Optional flags passed via ARGS
ARGS ?=

# Default rule
all: $(OUTPUT)

# Flex and Bison rules
$(LEX_C): $(LEX_SRC)
	$(FLEX) $(LEX_SRC)

$(BISON_C) $(BISON_H): $(BISON_SRC)
	$(BISON) -v -d $(BISON_SRC)

# Compile the frontend
$(OUTPUT): $(MAIN_SRC) $(UTILS_SRC) $(LEX_C) $(BISON_C) $(BISON_H)
	$(CC) -o $(OUTPUT) $(MAIN_SRC) $(UTILS_SRC) $(LEX_C) $(BISON_C) -lfl

# Run the program with a single file
run-file: $(OUTPUT)
	@read -p "Enter input file path: " file; \
	if [ -f "$$file" ]; then \
		echo "=== Running: ./$(OUTPUT) $(ARGS) $$file ==="; \
		./$(OUTPUT) $(ARGS) "$$file"; \
	else \
		echo "File not found: $$file"; \
	fi

# Run the program on all files in a directory
run-dir: $(OUTPUT)
	@read -p "Enter directory path: " dir; \
	if [ -d "$$dir" ]; then \
		for f in "$$dir"/*.c "$$dir"/*.cm; do \
			if [ -f "$$f" ]; then \
				echo "=== Running: ./$(OUTPUT) $(ARGS) $$f ==="; \
				./$(OUTPUT) $(ARGS) "$$f"; \
			fi; \
		done \
	else \
		echo "Directory not found: $$dir"; \
	fi

# Clean build files
clean:
	rm -f $(OUTPUT) lex.yy.c parser.tab.c parser.tab.h parser.output

.PHONY: all run-file run-dir clean
