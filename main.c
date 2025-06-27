#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "global.h"
#include "utils.h"
// #include "scan.h"
#include "parse.h"
#include "analyze.h"

/* allocate global variables */
int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

/* allocate and set tracing flags */
bool TraceScan = false;
bool TraceParse = false;
bool TraceAnalyze = false;
bool TraceCode = false;

bool Error = false;

int main(int argc, char **argv) {
    char program[128];
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
            strcpy(program, argv[i]);
        }
    }

    if (has_multiple_srcs) {
        fprintf(stderr, "Error: Too many input files.");
        print_help(argv[0]);
        return 1;
    }

    source = fopen(program, "r");
    if (!source) {
        fprintf(stderr, "Error opening file %s.", program);
        perror("Error opening file");
        return 1;
    }

    listing = stdout;
    fprintf(listing, "C- COMPILATION: %s\n", argv[1]);

    ASTNode *syntaxTree = NULL;
    if (!Error) {
        syntaxTree = parse();
        if (TraceParse) {
            fprintf(listing, "\nSyntax tree:\n");
            print_tree(syntaxTree, 0);
        }
    }

    if (!Error) {
        if (TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
        build_symtab(syntaxTree);
        if (TraceAnalyze) fprintf(listing,"\nChecking Types...\n");
        type_check(syntaxTree);
        if (TraceAnalyze) fprintf(listing,"\nType Checking Finished\n");
    }

    fclose(source);
    free_ast(syntaxTree);
    return 0;
}
