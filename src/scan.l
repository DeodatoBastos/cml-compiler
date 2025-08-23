%{

#include <stdbool.h>
#include "global.h"
#include "utils.h"
#include "scan.h"

TokenType get_token(void);

char tokenString[MAXTOKENLEN+1];

%}

%option yylineno
%option noyywrap

alpha      [a-zA-Z]
digit      [0-9]
identifier [a-zA-Z_][a-zA-Z0-9_]*

%%

"output"                         { return WRITE; }
"input"                          { return READ; }
"return"                         { return RETURN; }
"while"                          { return WHILE; }
"if"                             { return IF; }
"else"                           { return ELSE; }

"int"                            { return INT; }
"void"                           { return VOID; }

[-]?{digit}+                     { yylval.ival = atoi(yytext); return NUM; }
{identifier}                     {
                                    yylval.sval = malloc(yyleng + 1);
                                    memcpy(yylval.sval, yytext, yyleng + 1);
                                    return ID;
                                 }

"<="                             { return LE; }
">="                             { return GE; }
"=="                             { return EQ; }
"!="                             { return NE; }
">"                              { return GT; }
"<"                              { return LT; }

"+"                              { return PLUS; }
"-"                              { return MINUS; }
"*"                              { return TIMES; }
"/"                              { return OVER; }
"%"                              { return MOD; }

"{"                              { return LBRACE; }
"}"                              { return RBRACE; }
"["                              { return LBRACK; }
"]"                              { return RBRACK; }
"("                              { return LPAREN; }
")"                              { return RPAREN; }

";"                              { return SEMICOLON; }
","                              { return COMMA; }
"="                              { return ASSIGN; }

"//".*                          { /* skip comment */ }
"/*"([^*]|\*+[^*/])*"*"+"/"     { /* ignore multi-line comment */}
[ \t\r]+                        { /* skip whitespace */ }
\n                              { /* skip new line */ }

.                               {
                                    fprintf(stderr, "\033[1;31mLexical Error\033[0m at line %d: Unknown character: '%s'\n", yylineno, yytext);
                                    Error = true;
                                    return ERROR;
                                }

%%

TokenType get_token(void) {
    static int firstTime = true;
    TokenType currentToken;
    if (firstTime) {
        firstTime = false;
        yyin = source;
        yyout = listing;
    }
    currentToken = yylex();
    strncpy(tokenString, yytext, MAXTOKENLEN);
    if (TraceScan) {
        fprintf(listing, "\t%d: ", yylineno);
        print_token(currentToken, yytext);
    }
    return currentToken;
}
