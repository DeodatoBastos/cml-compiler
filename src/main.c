#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cgen.h"
#include "global.h"
#include "ir.h"
#include "reg_allocation.h"
#include "symtab.h"
#include "utils.h"
// #include "scan.h"
#include "analyze.h"
#include "cgen.h"
#include "parse.h"

/* allocate global variables */
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */
bool TraceScan = false;
bool TraceParse = false;
bool TraceAnalyze = false;
bool TraceCode = false;

bool Error = false;

extern void yylex_destroy();

int main(int argc, char **argv) {
    char program[128] = {0};
    char out_file[256] = {0};
    bool first_time = true;
    bool has_multiple_srcs = false;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ts") == 0) {
            TraceScan = true;
        } else if (strcmp(argv[i], "--tp") == 0) {
            TraceParse = true;
        } else if (strcmp(argv[i], "--ta") == 0) {
            TraceAnalyze = true;
        } else if (strcmp(argv[i], "--tc") == 0) {
            TraceCode = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                strncpy(out_file, argv[++i], sizeof(out_file) - 1);
            } else {
                fprintf(stderr, "Error: -o flag requires an output file name\n");
                return 1;
            }
        } else if (argv[i][0] == '-') {
            printf("Unknown option: %s\n", argv[i]);
            print_help(argv[0]);
            return 1;
        } else {
            if (first_time)
                first_time = false;
            else {
                has_multiple_srcs = true;
                break;
            }
            strncpy(program, argv[i], sizeof(program) - 1);
        }
    }

    if (has_multiple_srcs) {
        fprintf(stderr, "Error: Too many input files.");
        print_help(argv[0]);
        return 1;
    }

    if (program[0] == '\0') {
        fprintf(stderr, "Error: No input file provided.\n");
        print_help(argv[0]);
        return 1;
    }

    source = fopen(program, "r");
    if (!source) {
        fprintf(stderr, "Error opening file %s.", program);
        perror("Error opening file");
        return 1;
    }

    if (out_file[0] == '\0') {
        char base[128];
        replace_ext(base, program, ".asm");
        snprintf(out_file, sizeof(out_file), "asm/%s", base);

        if (mkdir("asm", 0777) != 0 && errno != EEXIST) {
            perror("Error creating asm directory");
            fclose(source);
            return 1;
        }
    }

    code = fopen(out_file, "w");
    if (!code) {
        fprintf(stderr, "Error opening output file %s.", out_file);
        perror("Error opening file");
        fclose(source);
        return 1;
    }

    listing = stdout;
    fprintf(listing, "C- COMPILATION: %s\n", program);

    ASTNode *tree = parse();
    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        print_tree(tree, 0);
    }

    if (Error) {
        fclose(source);
        fclose(code);
        yylex_destroy();
        return 1;
    }

    if (TraceAnalyze)
        fprintf(listing, "\nBuilding Symbol Table...\n");
    build_symtab(tree);
    if (TraceAnalyze)
        fprintf(listing, "\nChecking Types...\n");
    type_check(tree);
    if (TraceAnalyze)
        fprintf(listing, "\nType Checking Finished\n");

    if (Error) {
        free_symtab();
        free_ast(tree);
        fclose(source);
        fclose(code);
        yylex_destroy();
        return 1;
    }

    IR *ir = NULL;
    ir = gen_ir(tree);
    print_ir(ir, code);
    int *color_map = allocate_registers(ir);

    fclose(code);
    fclose(source);
    free(color_map);
    free_ir(ir);
    free_symtab();
    free_ast(tree);
    yylex_destroy();
    return 0;
}
