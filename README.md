# C Minus Language (CML) Compiler

<!--toc:start-->
- [C Minus Language (CML) Compiler](#c-minus-language-cml-compiler)
  - [Requirements](#requirements)
  - [Building the Compiler](#building-the-compiler)
  - [Usage](#usage)
    - [Basic Compilation](#basic-compilation)
    - [Command-Line Options](#command-line-options)
    - [Examples](#examples)
  - [Development Tools](#development-tools)
    - [Running Examples](#running-examples)
    - [Debugging](#debugging)
    - [Testing Individual Files](#testing-individual-files)
  - [Compilation Pipeline](#compilation-pipeline)
  - [C- Language](#c-language)
<!--toc:end-->

A compiler for the [C minus language](https://www.cs.sjsu.edu/~louden/cmptext/)

## Requirements

Install necessary packages:
```bash
sudo apt install flex bison make
```

## Building the Compiler
```bash
# Basic build
make

# Clean build artifacts
make clean
```

## Usage

### Basic Compilation

```bash
# Compile a C- source file
./cml.out source.cm

# Specify output file
./cml.out -o output.asm source.cm

```

### Command-Line Options

- `--ts` : Enable trace scanning (lexical analysis debugging)
- `--tp` : Enable trace parsing (syntax tree output)
- `--ta` : Enable trace analysis (symbol table and type checking)
- `--tc` : Enable trace code generation
- `-o <file>` : Specify output assembly file
- `--help` : Display help information

### Examples

```bash
# Compile with all tracing enabled
./cml.out --ts --tp --ta --tc example/program.cm

# Compile with specific output location
./cml.out -o myprogram.asm example/program.cm

# Compile with parse tree visualization
./cml.out --tp example/program.cm
```
## Development Tools

### Running Examples

```bash
# Interactive file selection from examples
make run-file

# Run all examples
make example

# Run all examples and save output
make example SAVE=1

# Run with custom arguments
make run-file ARGS="--tp --ta"
```

### Debugging

```bash
# Run with Valgrind memory checking
make debug

# This will prompt for file selection and run with:
# - Full memory leak checking
# - Origin tracking for uninitialized values
# - All leak kinds reporting
```

### Testing Individual Files

```bash
# Prompt for filename in example directory
make example-file
```

## Compilation Pipeline

1. **Lexical Analysis**: Tokenizes the source code using Flex-generated scanner
2. **Parsing**: Builds an Abstract Syntax Tree (AST) using Bison-generated parser
3. **Semantic Analysis**:
   - Symbol table construction
   - Type checking and validation
4. **IR Generation**: Converts AST to intermediate representation
5. **Register Allocation**: Assigns virtual registers to physical registers
6. **Code Generation**: Produces target assembly code (RISC-V)

## C- Language

C- (C-minus) is a simplified subset of the C programming language. It includes:
- Integer variables
- Functions and procedures
- Control flow statements
- Arrays
- Expressions and operators
