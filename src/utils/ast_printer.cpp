#include "header/ast_printer.hpp"

// Helper: format annotation string like "→ tab_index:34, type:integer, lev:0"
static std::string formatAnnotation(ASTNode* node) {
    std::string ann;
    bool hasContent = false;

    if (auto b = dynamic_cast<BlockNode*>(node)) {
        if (b->block_index != -1) {
            ann += "block_index:" + std::to_string(b->block_index);
            hasContent = true;
        }
    }
    if (node->tab_index != -1) {
        ann += "tab_index:" + std::to_string(node->tab_index);
        hasContent = true;
    }
    if (!node->eval_type.empty()) {
        if (hasContent) ann += ", ";
        ann += "type:" + node->eval_type;
        hasContent = true;
    }
    if (node->lex_level != -1) {
        if (hasContent) ann += ", ";
        ann += "lev:" + std::to_string(node->lex_level);
        hasContent = true;
    }

    if (hasContent) return " \xe2\x86\x92 " + ann; // → character (UTF-8)
    return "";
}

static void printNode(ASTNode* node, std::string prefix, bool isLast, std::ostream& out, bool dec,
                      bool suppressDecls = false) {
    if (!node) return;
    // Use box-drawing characters for the tree structure
    out << prefix << (isLast ? "\xe2\x94\x94\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80 ");

    std::string next_pref = prefix + (isLast ? "   " : "\xe2\x94\x82  ");

    // ======================== PROGRAM ========================
    if (auto n = dynamic_cast<ProgramNode*>(node)) {
        out << "ProgramNode(name: '" << n->name << "')";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->block) {
            bool hasDecls = !n->block->declarations.empty();
            if (hasDecls) {
                out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 " << "Declarations\n";
                std::string decl_pref = next_pref + "\xe2\x94\x82  ";
                for (size_t i = 0; i < n->block->declarations.size(); ++i) {
                    printNode(n->block->declarations[i], decl_pref, i == n->block->declarations.size() - 1, out, dec);
                }
            }
            printNode(n->block, next_pref, true, out, dec, true);
        }
    }
    // ======================== BLOCK ========================
    else if (auto n = dynamic_cast<BlockNode*>(node)) {
        out << "Block";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        // Print declarations section
        if (!suppressDecls && !n->declarations.empty()) {
            bool hasCompound = (n->compound_stmt != nullptr);
            out << next_pref << (hasCompound ? "\xe2\x94\x9c\xe2\x94\x80 " : "\xe2\x94\x94\xe2\x94\x80 ")
                << "Declarations\n";
            std::string decl_pref = next_pref + (hasCompound ? "\xe2\x94\x82  " : "   ");
            for (size_t i = 0; i < n->declarations.size(); ++i) {
                printNode(n->declarations[i], decl_pref, i == n->declarations.size() - 1, out, dec);
            }
        }
        // Print compound statement
        if (n->compound_stmt) {
            if (suppressDecls) {
                for (size_t i = 0; i < n->compound_stmt->statements.size(); ++i) {
                    printNode(n->compound_stmt->statements[i], next_pref, i == n->compound_stmt->statements.size() - 1,
                              out, dec);
                }
            } else {
                printNode(n->compound_stmt, next_pref, true, out, dec);
            }
        }
    }
    // ======================== VAR DECL ========================
    else if (auto n = dynamic_cast<VarDeclNode*>(node)) {
        for (size_t i = 0; i < n->names.size(); ++i) {
            // Determine if this particular variable line is the last child
            bool thisIsLast = (i == n->names.size() - 1) && isLast;
            // For the first variable, use the connector determined by the caller
            // but if there are more variables, it's NOT the last
            if (i == 0) {
                // Already printed "├─ " or "└─ " by the caller
                // But if this VarDecl has multiple names, the first one isn't last
                // unless it's both the first and last (single name)
                // The prefix connector was already printed, so just print content
            } else {
                // For subsequent variables, print their own connector
                out << prefix << (thisIsLast ? "\xe2\x94\x94\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80 ");
            }
            out << "VarDecl('" << n->names[i] << "')";
            if (dec) {
                // Look up type info from the type node
                std::string typeName = "unknown";
                if (auto tn = dynamic_cast<NamedTypeNode*>(n->type)) {
                    typeName = tn->name;
                } else if (dynamic_cast<ArrayTypeNode*>(n->type)) {
                    typeName = "array";
                } else if (dynamic_cast<RecordTypeNode*>(n->type)) {
                    typeName = "record";
                }
                // Use per-variable tab_index from tab_indices vector
                int var_tab_idx = (i < n->tab_indices.size()) ? n->tab_indices[i] : n->tab_index;
                int var_lev = n->lex_level;
                out << " \xe2\x86\x92 tab_index:" << var_tab_idx << ", type:" << typeName << ", lev:" << var_lev;
            }
            out << "\n";
        }
    }
    // ======================== CONST DECL ========================
    else if (auto n = dynamic_cast<ConstDeclNode*>(node)) {
        out << "ConstDecl('" << n->name << "')";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->value) printNode(n->value, next_pref, true, out, dec);
    }
    // ======================== TYPE DECL ========================
    else if (auto n = dynamic_cast<TypeDeclNode*>(node)) {
        out << "TypeDecl('" << n->name << "')";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->type) printNode(n->type, next_pref, true, out, dec);
    }
    // ======================== SUBPROGRAM DECL ========================
    else if (auto n = dynamic_cast<SubprogramDeclNode*>(node)) {
        std::string kind = n->is_function ? "FuncDecl" : "ProcDecl";
        out << kind << "('" << n->name << "')";
        if (n->is_function && !n->return_type_name.empty()) {
            out << " : " << n->return_type_name;
        }
        if (dec) out << formatAnnotation(n);
        out << "\n";
        // Print params
        for (size_t i = 0; i < n->params.size(); ++i) {
            printNode(n->params[i], next_pref, (i == n->params.size() - 1) && !n->block, out, dec);
        }
        // Print block
        if (n->block) printNode(n->block, next_pref, true, out, dec);
    }
    // ======================== PARAM ========================
    else if (auto n = dynamic_cast<ParamNode*>(node)) {
        out << "Param(";
        for (size_t i = 0; i < n->names.size(); ++i) {
            out << n->names[i];
            if (i + 1 < n->names.size()) out << ", ";
        }
        out << ")";
        if (dec) {
            std::string typeName = "unknown";
            if (auto tn = dynamic_cast<NamedTypeNode*>(n->type)) typeName = tn->name;
            out << " \xe2\x86\x92 type:" << typeName;
        }
        out << "\n";
    }
    // ======================== COMPOUND STMT ========================
    else if (auto n = dynamic_cast<CompoundStmtNode*>(node)) {
        out << "CompoundStmt";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        for (size_t i = 0; i < n->statements.size(); ++i) {
            printNode(n->statements[i], next_pref, i == n->statements.size() - 1, out, dec);
        }
    }
    // ======================== ASSIGN STMT ========================
    else if (auto n = dynamic_cast<AssignStmtNode*>(node)) {
        // Format: Assign('a' := 5) → type:void
        std::string targetName = n->target ? n->target->name : "?";
        std::string valueStr = "";
        // Build a summary of the value for display
        if (auto lit = dynamic_cast<LiteralNode*>(n->value)) {
            valueStr = lit->value;
        } else if (auto var = dynamic_cast<VarAccessNode*>(n->value)) {
            valueStr = var->name;
        } else if (auto bin = dynamic_cast<BinOpNode*>(n->value)) {
            // For BinOp, show the op symbol
            std::string opSym = bin->op;
            if (opSym == "plus")
                opSym = "+";
            else if (opSym == "minus")
                opSym = "-";
            else if (opSym == "times")
                opSym = "*";
            else if (opSym == "rdiv")
                opSym = "/";
            else if (opSym == "idiv")
                opSym = "div";
            else if (opSym == "imod")
                opSym = "mod";
            // Try to get left/right names
            std::string lStr = "...", rStr = "...";
            if (auto lv = dynamic_cast<VarAccessNode*>(bin->left))
                lStr = lv->name;
            else if (auto ll = dynamic_cast<LiteralNode*>(bin->left))
                lStr = ll->value;
            if (auto rv = dynamic_cast<VarAccessNode*>(bin->right))
                rStr = rv->name;
            else if (auto rl = dynamic_cast<LiteralNode*>(bin->right))
                rStr = rl->value;
            valueStr = lStr + opSym + rStr;
        }
        out << "Assign('" << targetName << "' := " << valueStr << ")";
        if (dec) {
            out << " \xe2\x86\x92 type:void";
        }
        out << "\n";
        // Print target
        out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 target ";
        if (n->target) {
            out << "'" << n->target->name << "'";
            if (dec)
                out << " \xe2\x86\x92 " << "tab_index:" << n->target->tab_index << ", type:" << n->target->eval_type;
        }
        out << "\n";
        // Print value
        out << next_pref << "\xe2\x94\x94\xe2\x94\x80 value ";
        if (auto lit = dynamic_cast<LiteralNode*>(n->value)) {
            out << lit->value;
            if (dec) out << " \xe2\x86\x92 type:" << lit->eval_type;
            out << "\n";
        } else if (auto var = dynamic_cast<VarAccessNode*>(n->value)) {
            out << "'" << var->name << "'";
            if (dec) out << " \xe2\x86\x92 tab_index:" << var->tab_index << ", type:" << var->eval_type;
            out << "\n";
        } else {
            out << "\n";
            if (n->value) {
                printNode(n->value, next_pref + "   ", true, out, dec);
            }
        }
    }
    // ======================== VAR ACCESS ========================
    else if (auto n = dynamic_cast<VarAccessNode*>(node)) {
        out << "'" << n->name << "'";
        if (dec) out << " \xe2\x86\x92 tab_index:" << n->tab_index << ", type:" << n->eval_type;
        out << "\n";
    }
    // ======================== LITERAL ========================
    else if (auto n = dynamic_cast<LiteralNode*>(node)) {
        out << n->value;
        if (dec) out << " \xe2\x86\x92 type:" << n->eval_type;
        out << "\n";
    }
    // ======================== BINOP ========================
    else if (auto n = dynamic_cast<BinOpNode*>(node)) {
        std::string opSym = n->op;
        if (opSym == "plus")
            opSym = "+";
        else if (opSym == "minus")
            opSym = "-";
        else if (opSym == "times")
            opSym = "*";
        else if (opSym == "rdiv")
            opSym = "/";
        else if (opSym == "idiv")
            opSym = "div";
        else if (opSym == "imod")
            opSym = "mod";
        else if (opSym == "eql")
            opSym = "=";
        else if (opSym == "neq")
            opSym = "<>";
        else if (opSym == "lss")
            opSym = "<";
        else if (opSym == "leq")
            opSym = "<=";
        else if (opSym == "gtr")
            opSym = ">";
        else if (opSym == "geq")
            opSym = ">=";
        else if (opSym == "andsy")
            opSym = "and";
        else if (opSym == "orsy")
            opSym = "or";
        out << "BinOp '" << opSym << "'";
        if (dec) out << " \xe2\x86\x92 type:" << n->eval_type;
        out << "\n";
        printNode(n->left, next_pref, false, out, dec);
        printNode(n->right, next_pref, true, out, dec);
    }
    // ======================== UNARY OP ========================
    else if (auto n = dynamic_cast<UnaryOpNode*>(node)) {
        std::string opSym = n->op;
        if (opSym == "notsy")
            opSym = "not";
        else if (opSym == "minus")
            opSym = "-";
        else if (opSym == "plus")
            opSym = "+";
        out << "UnaryOp '" << opSym << "'";
        if (dec) out << " \xe2\x86\x92 type:" << n->eval_type;
        out << "\n";
        printNode(n->right, next_pref, true, out, dec);
    }
    // ======================== FUNC CALL (expr) ========================
    else if (auto n = dynamic_cast<FuncCallNode*>(node)) {
        out << "FuncCall('" << n->name << "')";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        for (size_t i = 0; i < n->args.size(); ++i) {
            printNode(n->args[i], next_pref, i == n->args.size() - 1, out, dec);
        }
    }
    // ======================== PROC CALL STMT ========================
    else if (auto n = dynamic_cast<ProcCallStmtNode*>(node)) {
        out << n->name << "(...)";
        if (dec) {
            out << " \xe2\x86\x92 predefined";
            if (n->tab_index != -1) out << ", tab_index:" << n->tab_index;
        }
        out << "\n";
        for (size_t i = 0; i < n->args.size(); ++i) {
            printNode(n->args[i], next_pref, i == n->args.size() - 1, out, dec);
        }
    }
    // ======================== IF STMT ========================
    else if (auto n = dynamic_cast<IfStmtNode*>(node)) {
        out << "IfStmt";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->condition) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 condition\n";
            printNode(n->condition, next_pref + "\xe2\x94\x82  ", true, out, dec);
        }
        if (n->then_stmt) {
            bool hasElse = (n->else_stmt != nullptr);
            out << next_pref << (hasElse ? "\xe2\x94\x9c\xe2\x94\x80 " : "\xe2\x94\x94\xe2\x94\x80 ") << "then\n";
            printNode(n->then_stmt, next_pref + (hasElse ? "\xe2\x94\x82  " : "   "), true, out, dec);
        }
        if (n->else_stmt) {
            out << next_pref << "\xe2\x94\x94\xe2\x94\x80 else\n";
            printNode(n->else_stmt, next_pref + "   ", true, out, dec);
        }
    }
    // ======================== WHILE STMT ========================
    else if (auto n = dynamic_cast<WhileStmtNode*>(node)) {
        out << "WhileStmt";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->condition) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 condition\n";
            printNode(n->condition, next_pref + "\xe2\x94\x82  ", true, out, dec);
        }
        if (n->body) {
            out << next_pref << "\xe2\x94\x94\xe2\x94\x80 body\n";
            printNode(n->body, next_pref + "   ", true, out, dec);
        }
    }
    // ======================== FOR STMT ========================
    else if (auto n = dynamic_cast<ForStmtNode*>(node)) {
        out << "ForStmt('" << n->counter_name << "', " << n->direction << ")";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->init_val) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 init\n";
            printNode(n->init_val, next_pref + "\xe2\x94\x82  ", true, out, dec);
        }
        if (n->final_val) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 final\n";
            printNode(n->final_val, next_pref + "\xe2\x94\x82  ", true, out, dec);
        }
        if (n->body) {
            out << next_pref << "\xe2\x94\x94\xe2\x94\x80 body\n";
            printNode(n->body, next_pref + "   ", true, out, dec);
        }
    }
    // ======================== REPEAT STMT ========================
    else if (auto n = dynamic_cast<RepeatStmtNode*>(node)) {
        out << "RepeatStmt";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        for (size_t i = 0; i < n->body.size(); ++i) {
            printNode(n->body[i], next_pref, false, out, dec);
        }
        if (n->condition) {
            out << next_pref << "\xe2\x94\x94\xe2\x94\x80 until\n";
            printNode(n->condition, next_pref + "   ", true, out, dec);
        }
    }
    // ======================== CASE STMT ========================
    else if (auto n = dynamic_cast<CaseStmtNode*>(node)) {
        out << "CaseStmt";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->expression) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 expr\n";
            printNode(n->expression, next_pref + "\xe2\x94\x82  ", true, out, dec);
        }
        for (size_t i = 0; i < n->blocks.size(); ++i) {
            printNode(n->blocks[i], next_pref, i == n->blocks.size() - 1, out, dec);
        }
    }
    // ======================== CASE BLOCK ========================
    else if (auto n = dynamic_cast<CaseBlockNode*>(node)) {
        out << "CaseBlock";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        for (size_t i = 0; i < n->constants.size(); ++i) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 const: ";
            if (auto lit = dynamic_cast<LiteralNode*>(n->constants[i])) {
                out << lit->value;
            }
            out << "\n";
        }
        if (n->statement) {
            printNode(n->statement, next_pref, true, out, dec);
        }
    }
    // ======================== NAMED TYPE ========================
    else if (auto n = dynamic_cast<NamedTypeNode*>(node)) {
        out << "Type('" << n->name << "')";
        if (dec) out << formatAnnotation(n);
        out << "\n";
    }
    // ======================== ARRAY TYPE ========================
    else if (auto n = dynamic_cast<ArrayTypeNode*>(node)) {
        out << "ArrayType";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        if (n->index_type) {
            out << next_pref << "\xe2\x94\x9c\xe2\x94\x80 index: ";
            // inline print for range/named
            if (auto rng = dynamic_cast<RangeNode*>(n->index_type)) {
                std::string lo = "?", hi = "?";
                if (auto ll = dynamic_cast<LiteralNode*>(rng->low)) lo = ll->value;
                if (auto lh = dynamic_cast<LiteralNode*>(rng->high)) hi = lh->value;
                out << lo << ".." << hi << "\n";
            } else if (auto nt = dynamic_cast<NamedTypeNode*>(n->index_type)) {
                out << nt->name << "\n";
            } else {
                out << "\n";
                printNode(n->index_type, next_pref + "\xe2\x94\x82  ", true, out, dec);
            }
        }
        if (n->element_type) {
            out << next_pref << "\xe2\x94\x94\xe2\x94\x80 element: ";
            if (auto nt = dynamic_cast<NamedTypeNode*>(n->element_type)) {
                out << nt->name << "\n";
            } else {
                out << "\n";
                printNode(n->element_type, next_pref + "   ", true, out, dec);
            }
        }
    }
    // ======================== RANGE TYPE ========================
    else if (auto n = dynamic_cast<RangeNode*>(node)) {
        std::string lo = "?", hi = "?";
        if (auto ll = dynamic_cast<LiteralNode*>(n->low)) lo = ll->value;
        if (auto lh = dynamic_cast<LiteralNode*>(n->high)) hi = lh->value;
        out << "Range(" << lo << ".." << hi << ")";
        if (dec) out << formatAnnotation(n);
        out << "\n";
    }
    // ======================== RECORD TYPE ========================
    else if (auto n = dynamic_cast<RecordTypeNode*>(node)) {
        out << "RecordType";
        if (dec) out << formatAnnotation(n);
        out << "\n";
        for (size_t i = 0; i < n->fields.size(); ++i) {
            auto f = n->fields[i];
            out << next_pref << (i == n->fields.size() - 1 ? "\xe2\x94\x94\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80 ");
            out << "Field(";
            for (size_t j = 0; j < f->names.size(); ++j) {
                out << f->names[j];
                if (j + 1 < f->names.size()) out << ", ";
            }
            out << ")";
            if (dec && f->type) {
                if (auto nt = dynamic_cast<NamedTypeNode*>(f->type)) {
                    out << " : " << nt->name;
                }
            }
            out << "\n";
        }
    }
    // ======================== ENUM TYPE ========================
    else if (auto n = dynamic_cast<EnumTypeNode*>(node)) {
        out << "EnumType(";
        for (size_t i = 0; i < n->identifiers.size(); ++i) {
            out << n->identifiers[i];
            if (i + 1 < n->identifiers.size()) out << ", ";
        }
        out << ")";
        if (dec) out << formatAnnotation(n);
        out << "\n";
    }
    // ======================== EMPTY STMT ========================
    else if (dynamic_cast<EmptyStmtNode*>(node)) {
        out << "EmptyStmt\n";
    }
    // ======================== FALLBACK ========================
    else {
        out << "ASTNode";
        if (dec) out << formatAnnotation(node);
        out << "\n";
    }
}

void printAST(ASTNode* node, std::ostream& out, bool decorated) {
    if (!node) return;
    printNode(node, "", true, out, decorated);
}