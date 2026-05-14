#ifndef AST_PRINTER_HPP
#define AST_PRINTER_HPP

#include "semantic/header/ast.hpp"
#include <iostream>
#include <string>

void printAST(ASTNode* node, std::ostream& out, bool decorated = false);

#endif