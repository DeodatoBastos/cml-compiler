%{

#define YYPARSER /* distinguishes Yacc output from other code files */

#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "global.h"
#include "utils.h"
#include "scan.h"
#include "parse.h"

int yyerror(const char *s);
static int yylex();

static ASTNode *root;
// static char *savedName; /* for use in assignments */
// static int savedLineNo;  /* ditto */

%}

%union {
    int ival;
    char* sval;
    ExprType type;
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
%token <sval> ID
%token LE GE EQ NE GT LT
%token ADD SUB MUL DIV MOD
%token LPAREN RPAREN LBRACE RBRACE LBRACK RBRACK SEMICOLON COMMA ASSIGN
%token ERROR

%type  <node> program decl_list decl var_decl func_decl compound_stmt params param_list
%type  <node> param local_decl stmt_list stmt expr_stmt sel_stmt iter_stmt return_stmt read_stmt write_stmt
%type  <node> expr var simple_expr add_expr term factor func_call args arg_list
%type  <tval> relop addop mulop
%type  <type> type_spec

%%

program: decl_list { root = $1; }
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

decl: var_decl  { $$ = $1; }
    | func_decl { $$ = $1; }
;

var_decl: type_spec ID SEMICOLON {
            $$ = new_expr_node(VarDecl, $2);
            $$->type = $1;
        }
        | type_spec ID LBRACK NUM RBRACK SEMICOLON {
            $$ = new_expr_node(ArrDecl, $2);
            $$->type = $1;
            $$->child[0] = new_expr_node(Const, NULL);
            $$->child[0]->attr.val = $4;
        }
;

type_spec: INT {
            $$ = Integer;
         }
         | VOID {
            $$ = Void;
         }
;

func_decl: type_spec ID LPAREN params RPAREN compound_stmt {
            $$ = new_expr_node(FuncDecl, $2);
            $$->type = $1;
            $$->child[0] = $4;
            $$->child[1] = $6;
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
        $$ = new_expr_node(ParamVar, $2);
        $$->type = $1;
     }
     | type_spec ID LBRACK RBRACK {
        $$ = new_expr_node(ParamArr, $2);
        $$->type = $1;
     }
;

compound_stmt: LBRACE local_decl stmt_list RBRACE {
                $$ = new_stmt_node(Compound, NULL);
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
            $$ = new_stmt_node(If, NULL);
            $$->child[0] = $3;
            $$->child[1] = $5;
        }
        | IF LPAREN expr RPAREN stmt ELSE stmt {
            $$ = new_stmt_node(If, NULL);
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->child[2] = $7;
        }
;

iter_stmt: WHILE LPAREN expr RPAREN stmt {
            $$ = new_stmt_node(While, NULL);
            $$->child[0] = $3;
            $$->child[1] = $5;
         }
;

return_stmt: RETURN SEMICOLON {
            $$ = new_stmt_node(Return, NULL);
           }
           | RETURN expr SEMICOLON {
            $$ = new_stmt_node(Return, NULL);
            $$->child[0] = $2;
           }
;

read_stmt: var ASSIGN READ LPAREN RPAREN SEMICOLON {
            $$ = new_stmt_node(Read, $1->attr.name);
            $$->child[0] = $1;
         }
;

write_stmt: WRITE LPAREN simple_expr RPAREN SEMICOLON {
            $$ = new_stmt_node(Write, NULL);
            $$->child[0] = $3;
          }
;

expr: var ASSIGN expr {
        $$ = new_stmt_node(Assign, $1->attr.name);
        $$->child[0] = $1;
        $$->child[1] = $3;
    }
    | simple_expr {
        $$ = $1;
    }
;

var: ID {
    $$ = new_expr_node(Var, $1);
    $$->type = Integer;
   }
   | ID LBRACK expr RBRACK {
    $$ = new_expr_node(Arr, $1);
    $$->child[0] = $3;
    $$->type = Integer;
   }
;

simple_expr: add_expr relop add_expr {
            $$ = new_expr_node(Op, NULL);
            $$->child[0] = $1;
            $$->child[1] = $3;
            $$->attr.op = $2;
            $$->type = Boolean;
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
            $$ = new_expr_node(Op, NULL);
            $$->child[0] = $1;
            $$->child[1] = $3;
            $$->attr.op = $2;
            $$->type = Integer;
        }
        | term {
            $$ = $1;
        }
;

addop: ADD { $$ = ADD; }
     | SUB { $$ = SUB; }
;

term: term mulop factor {
        $$ = new_expr_node(Op, NULL);
        $$->child[0] = $1;
        $$->child[1] = $3;
        $$->attr.op = $2;
        $$->type = Integer;
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
        $$ = new_expr_node(Const, NULL);
        $$->attr.val = $1;
        $$->type = Integer;
      }
      | error {
        $$ = NULL;
      }
;

func_call: ID LPAREN args RPAREN {
            $$ = new_expr_node(FuncCall, $1);
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
