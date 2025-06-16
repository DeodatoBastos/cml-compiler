%{

#define YYPARSER /* distinguishes Yacc output from other code files */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ast.h"
#include "global.h"
#include "utils.h"
#include "scan.h"
#include "parse.h"

// #define YYSTYPE ASTNode*

int yyerror(const char *s);
static int yylex();

static ASTNode *root;
// static char *savedName; /* for use in assignments */
// static int savedLineNo;  /* ditto */

%}

%union {
    int ival;
    char* sval;
    ASTNode* node;
}

%define parse.trace
%start program

%left MUL DIV MOD
%left ADD SUB

%token WRITE READ INT VOID WHILE IF ELSE RETURN
%token <ival> NUM
%token <sval> ID INCLUDE
%token <sval> LE GE EQ NE GT LT
%token <sval> ADD SUB MUL DIV MOD
%token LPAREN RPAREN LBRACE RBRACE LBRACK RBRACK SEMICOLON COMMA ASSIGN
%token ERROR

%type  <node> program headers decl_list decl var_decl func_decl type_spec compound_stmt params param_list
%type  <node> param local_decl stmt_list stmt expr_stmt sel_stmt iter_stmt return_stmt read_stmt write_stmt
%type  <node> expr var simple_expr add_expr relop addop term mulop factor func_call args arg_list


%%

program: headers decl_list {
            $$ = new_node(NODE_PROGRAM, $1, $2, NULL);
            root = $$;
       }
;

headers: headers INCLUDE {
       ASTNode *headerNode = new_id($2);
        $$ = new_node(NODE_HEADER, $1, headerNode, NULL);
       }
       | /* empty */ {
            $$ = NULL;
       }
;

decl_list: decl_list decl {
            $$ = new_node(NODE_DECL_LIST, $1, $2, NULL);
         }
         | decl {
            $$ = new_node(NODE_DECL_LIST, $1, NULL, NULL);
         }
;

decl: var_decl {
      $$ = new_node(NODE_DECL, $1, NULL, NULL);
    }
    | func_decl {
      $$ = new_node(NODE_DECL, $1, NULL, NULL);
    }
;

var_decl: type_spec ID SEMICOLON {
            ASTNode *idNode = new_id($2);
            $$ = new_node(NODE_VAR_DECL, $1, idNode, NULL);
        }
        | type_spec ID LBRACK NUM RBRACK SEMICOLON {
            ASTNode *idNode = new_id($2);
            ASTNode *numNode = new_num($4);
            ASTNode *arraySizeNode = new_node(NODE_NUM, NULL, NULL, NULL);
            arraySizeNode->value = numNode->value;
            ASTNode *varNode = new_node(NODE_VAR_DECL, $1, idNode, NULL);

            ASTNode *arrayNode = new_node(NODE_VAR_DECL, idNode, numNode, NULL);
            $$ = new_node(NODE_VAR_DECL, $1, arrayNode, NULL);
        }
;

type_spec: INT {
            $$ = new_node(NODE_TYPE, NULL, NULL, "int");
         }
         | VOID {
            $$ = new_node(NODE_TYPE, NULL, NULL, "void");
         }
;

func_decl: type_spec ID LPAREN params RPAREN compound_stmt {
            ASTNode *idNode = new_id($2);
            ASTNode *funcNode = new_node(NODE_FUNC_DECL, idNode, $4, NULL);
            $$ = new_node(NODE_FUNC_DECL, $1, funcNode, NULL);
            funcNode->right = $6;
         }
;

params: param_list { $$ = $1; }
      | VOID { $$ = NULL; }
;

param_list: param_list COMMA param {
            $$ = new_node(NODE_PARAM_LIST, $1, $3, NULL);
          }
          | param {
            $$ = new_node(NODE_PARAM_LIST, $1, NULL, NULL);
          }
;

param: type_spec ID {
        ASTNode *idNode = new_id($2);
        $$ = new_node(NODE_PARAM, $1, idNode, NULL);
     }
     | type_spec ID LBRACK RBRACK {
            ASTNode *idNode = new_id($2);
            ASTNode *arrayNode = new_node(NODE_PARAM, idNode, NULL, "array");
            $$ = new_node(NODE_PARAM, $1, arrayNode, NULL);
     }
;

compound_stmt: LBRACE local_decl stmt_list RBRACE {
                ASTNode *compound = new_node(NODE_COMPOUND, $2, $3, NULL);
                $$ = compound;
             }
;

local_decl: local_decl var_decl {
            $$ = new_node(NODE_LOCAL_DECL, $1, $2, NULL);
          }
          | /* empty */ {
            $$ = NULL;
          }
;

stmt_list: stmt_list stmt {
            $$ = new_node(NODE_STMT_LIST, $1, $2, NULL);
         }
         | /* empty */ {
            $$ = NULL;
         }
;

stmt: expr_stmt { $$ = $1; }
    | compound_stmt { $$ = $1; }
    | sel_stmt { $$ = $1; }
    | iter_stmt { $$ = $1; }
    | return_stmt { $$ = $1; }
    | read_stmt { $$ = $1; }
    | write_stmt { $$ = $1; }
    | ERROR { $$ = NULL; }
;

expr_stmt: expr SEMICOLON {
            $$ = new_node(NODE_EXPR_STMT, $1, NULL, NULL);
         }
         | SEMICOLON {
            $$ = NULL;
         }
;

sel_stmt: IF LPAREN expr RPAREN stmt {
            $$ = new_node(NODE_SEL_STMT, $3, $5, "if");
        }
        | IF LPAREN expr RPAREN stmt ELSE stmt {
            ASTNode *ifNode = new_node(NODE_SEL_STMT, $3, $5, "if");
            $$ = new_node(NODE_SEL_STMT, ifNode, $7, "else");
        }
;

iter_stmt: WHILE LPAREN expr RPAREN stmt {
            $$ = new_node(NODE_ITER_STMT, $3, $5, NULL);
         }
;

return_stmt: RETURN SEMICOLON {
            $$ = new_node(NODE_RETURN_STMT, NULL, NULL, NULL);
           }
           | RETURN expr SEMICOLON {
            $$ = new_node(NODE_RETURN_STMT, $2, NULL, NULL);
           }
;

read_stmt: var ASSIGN READ LPAREN RPAREN SEMICOLON {
            $$ = new_node(NODE_READ, $1, NULL, NULL);
         }
;

write_stmt: WRITE LPAREN simple_expr RPAREN SEMICOLON {
            $$ = new_node(NODE_WRITE, $3, NULL, NULL);
          }
;

expr: var ASSIGN expr {
        $$ = new_node(NODE_ASSIGN, $1, $3, "=");
    }
    | simple_expr {
        $$ = $1;
    }
;

var: ID {
    $$ = new_id($1);
   }
   | ID LBRACK expr RBRACK {
    ASTNode *idNode = new_id($1);
    $$ = new_node(NODE_VAR, idNode, $3, NULL);
   }
;

simple_expr: add_expr relop add_expr {
            ASTNode *opNode = new_node(NODE_RELOP, $2, NULL, NULL);
            $$ = new_node(NODE_SIMPLE_EXPR, $1, $3, NULL);

            $$ = new_node(NODE_RELOP, $1, $3, $2->name);
           }
           | add_expr {
            $$ = $1;
           }
;

relop: LE { $$ = new_id("<="); }
     | LT { $$ = new_id("<"); }
     | GT { $$ = new_id(">"); }
     | GE { $$ = new_id(">="); }
     | EQ { $$ = new_id("=="); }
     | NE { $$ = new_id("!="); }
;

add_expr: add_expr addop term {
            $$ = new_node(NODE_ADD_EXPR, $1, $3, $2->name);
        }
        | term {
            $$ = $1;
        }
;

addop: ADD { $$ = new_id("+"); }
     | SUB { $$ = new_id("-"); }
;

term: term mulop factor {
        $$ = new_node(NODE_TERM, $1, $3, $2->name);
    }
    | factor {
        $$ = $1;
    }
;

mulop: MUL { $$ = new_id("*"); }
     | DIV { $$ = new_id("/"); }
     | MOD { $$ = new_id("%"); }
;

factor: LPAREN expr RPAREN {
        $$ = $2;
      }
      | var {
        $$ = $1;
      }
      | func_call {
        $$ = $1;
      }
      | NUM {
        $$ = new_num($1);
      }
      | ERROR {
        $$ = NULL;
      }
;

func_call: ID LPAREN args RPAREN {
            ASTNode *idNode = new_id($1);
            $$ = new_node(NODE_CALL, idNode, $3, NULL);
         }
;

args: arg_list {
        $$ = $1;
    }
    | /* empty */ {
        $$ = NULL;
    }
;

arg_list: arg_list COMMA expr {
            $$ = new_node(NODE_ARG_LIST, $1, $3, NULL);
        }
        | expr {
            $$ = new_node(NODE_ARG_LIST, $1, NULL, NULL);
        }
;

%%

int yyerror(const char *msg) {
    fprintf(listing, "\033[1;31mError\033[0m at line %d: %s\n", lineno, msg);
    fprintf(listing, "Current token: ");
    print_token(yychar, tokenString);
    Error = true;
    return 0;
}

static int yylex(void) {
    return get_token();
}

ASTNode *parse(void) {
    yyparse();
    return root;
}
