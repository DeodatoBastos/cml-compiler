#ifndef CGEN_H
#define CGEN_H

#include "ast.h"
#include "ir.h"
#include "symtab.h"

IR *gen_ir(ASTNode *node);

#endif
