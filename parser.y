%{

#define YYPARSER /* distinguishes Yacc output from other code files */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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
    TokenType tval;
}

%define parse.trace
%start program

%left MUL DIV MOD
%left ADD SUB

%token WRITE READ WHILE IF ELSE RETURN
%token <ival> NUM
%token INT VOID
%token <sval> ID INCLUDE
%token LE GE EQ NE GT LT
%token ADD SUB MUL DIV MOD
%token LPAREN RPAREN LBRACE RBRACE LBRACK RBRACK SEMICOLON COMMA ASSIGN
%token ERROR

%type  <node> program headers decl_list decl var_decl func_decl type_spec compound_stmt params param_list
%type  <node> param local_decl stmt_list stmt expr_stmt sel_stmt iter_stmt return_stmt read_stmt write_stmt
%type  <node> expr var simple_expr add_expr term factor func_call args arg_list
%type  <tval> relop addop mulop

%%

program: headers decl_list {
            $$ = new_node(NODE_PROGRAM, NULL);
            $$->child[0] = $1;
            $$->child[1] = $2;
            root = $$;
       }
;

headers: headers INCLUDE {
            $$ = new_node(NODE_HEADER, $2);
            $$->child[0] = $1;
       }
       | /* empty */ {
            $$ = NULL;
       }
;

decl_list: decl_list decl {
            ASTNode *n = $1;
            if (n != NULL) {
                while(n->sibling != NULL)
                    n = n->sibling;
                n->sibling = $2;
                $$ = $1;
            } else $$ = $2;
         }
         | decl {
            $$ = $1;
         }
;

decl: var_decl {
      $$ = $1;
    }
    | func_decl {
      $$ = $1;
    }
;

var_decl: type_spec ID SEMICOLON {
            $$ = new_node(NODE_VAR_DECL, $2);
            $$->child[0] = $1;
        }
        | type_spec ID LBRACK NUM RBRACK SEMICOLON {
            $$ = new_node(NODE_ARR_DECL, $2);
            $$->child[0] = $1;
            $$->child[1] = new_num($4);
        }
;

type_spec: INT {
            $$ = new_node(NODE_TYPE, "int");
            $$->type = Integer;
         }
         | VOID {
            $$ = new_node(NODE_TYPE, "void");
            $$->type = Void;
         }
;

func_decl: type_spec ID LPAREN params RPAREN compound_stmt {
            $$ = new_node(NODE_FUNC_DECL, $2);
            $$->child[0] = $1;
            $$->child[1] = $4;
            $$->child[2] = $6;
         }
;

params: param_list { $$ = $1; }
      | VOID { $$ = NULL; }
;

param_list: param_list COMMA param {
            ASTNode *n = $1;
            if (n != NULL) {
                while(n->sibling != NULL)
                    n = n->sibling;
                n->sibling = $3;
                $$ = $1;
            } else $$ = $3;
          }
          | param {
            $$ = $1;
          }
;

param: type_spec ID {
        $$ = new_node(NODE_PARAM, $2);
        $$->child[0] = $1;
     }
     | type_spec ID LBRACK RBRACK {
        $$ = new_node(NODE_PARAM_ARR, $2);
        $$->child[0] = $1;
     }
;

compound_stmt: LBRACE local_decl stmt_list RBRACE {
                $$ = new_node(NODE_COMPOUND, NULL);
                $$->child[0] = $2;
                $$->child[1] = $3;
             }
;

local_decl: local_decl var_decl {
            ASTNode *n = $1;
            if (n != NULL) {
                while(n->sibling != NULL)
                    n = n->sibling;
                n->sibling = $2;
                $$ = $1;
            } else $$ = $2;
          }
          | /* empty */ {
            $$ = NULL;
          }
;

stmt_list: stmt_list stmt {
            ASTNode *n = $1;
            if (n != NULL) {
                while(n->sibling != NULL)
                    n = n->sibling;
                n->sibling = $2;
                $$ = $1;
            } else $$ = $2;
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
    | error { $$ = NULL; }
;

expr_stmt: expr SEMICOLON {
            $$ = $1;
         }
         | SEMICOLON {
            $$ = NULL;
         }
;

sel_stmt: IF LPAREN expr RPAREN stmt {
            $$ = new_node(NODE_SEL_STMT, "if");
            $$->child[0] = $3;
            $$->child[1] = $5;
        }
        | IF LPAREN expr RPAREN stmt ELSE stmt {
            $$ = new_node(NODE_SEL_STMT, "if-else");
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->child[2] = $7;
        }
;

iter_stmt: WHILE LPAREN expr RPAREN stmt {
            $$ = new_node(NODE_ITER_STMT, "while");
            $$->child[0] = $3;
            $$->child[1] = $5;
         }
;

return_stmt: RETURN SEMICOLON {
            $$ = new_node(NODE_RETURN_STMT, NULL);
           }
           | RETURN expr SEMICOLON {
            $$ = new_node(NODE_RETURN_STMT, NULL);
            $$->child[0] = $2;
           }
;

read_stmt: var ASSIGN READ LPAREN RPAREN SEMICOLON {
            $$ = new_node(NODE_READ, NULL);
            $$->child[0] = $1;
         }
;

write_stmt: WRITE LPAREN simple_expr RPAREN SEMICOLON {
            $$ = new_node(NODE_WRITE, NULL);
            $$->child[0] = $3;
          }
;

expr: var ASSIGN expr {
        $$ = new_node(NODE_ASSIGN, "=");
        $$->child[0] = $1;
        $$->child[1] = $3;
    }
    | simple_expr {
        $$ = $1;
    }
;

var: ID {
    $$ = new_node(NODE_VAR, $1);
   }
   | ID LBRACK expr RBRACK {
    $$ = new_node(NODE_ARR, $1);
    $$->child[0] = $3;
   }
;

simple_expr: add_expr relop add_expr {
            $$ = new_node(NODE_RELOP, NULL);
            $$->child[0] = $1;
            $$->child[1] = $3;
            $$->attr.op = $2;
           }
           | add_expr {
            $$ = $1;
           }
;

relop: LE { $$ = LE; }
     | LT { $$ = LT; }
     | GT { $$ = GT; }
     | GE { $$ = GE; }
     | EQ { $$ = EQ; }
     | NE { $$ = NE; }
;

add_expr: add_expr addop term {
            $$ = new_node(NODE_ADD_EXPR, NULL);
            $$->child[0] = $1;
            $$->child[1] = $3;
            $$->attr.op = $2;
        }
        | term {
            $$ = $1;
        }
;

addop: ADD { $$ = ADD; }
     | SUB { $$ = SUB; }
;

term: term mulop factor {
        $$ = new_node(NODE_TERM, NULL);
        $$->child[0] = $1;
        $$->child[1] = $3;
        $$->attr.op = $2;
    }
    | factor {
        $$ = $1;
    }
;

mulop: MUL { $$ = MUL; }
     | DIV { $$ = DIV; }
     | MOD { $$ = MOD; }
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
      | error {
        $$ = NULL;
      }
;

func_call: ID LPAREN args RPAREN {
            $$ = new_node(NODE_CALL, $1);
            $$->child[0] = $3;
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
            ASTNode *n = $1;
            if (n != NULL) {
                while(n->sibling != NULL)
                    n = n->sibling;
                n->sibling = $3;
                $$ = $1;
            } else $$ = $3;
        }
        | expr {
            $$ = $1;
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
