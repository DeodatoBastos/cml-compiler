#ifndef SCAN_H
#define SCAN_H

/* MAXTOKENLEN is the maximum size of a token */
#define MAXTOKENLEN 40

#include "../utils/ast.h"

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN+1];

/* function get_token returns the 
 * next token in source file
 */
TokenType get_token(void);

#endif
