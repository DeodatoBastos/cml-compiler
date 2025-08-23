#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdbool.h>
#include "utils/ast.h"

#ifndef YYPARSER

#include "parser.tab.h"
/* ENDFILE is implicitly defined by Yacc/Bison,
 * and not included in the tab.h file
 */
#define ENDFILE 0

#endif

extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

extern int yylineno; /* source line number for listing */

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern bool TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern bool TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern bool TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern bool TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern bool Error;

#endif
