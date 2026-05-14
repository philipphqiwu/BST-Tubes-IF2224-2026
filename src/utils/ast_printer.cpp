#include "header/ast_printer.hpp"

static void printDeco(ASTNode* node, std::ostream& out, bool decorated) {
    if (!decorated) return;
    out << " -> [";
    if (!node->eval_type.empty()) out << "type:" << node->eval_type;
    else out << "type:void";
    if (node->tab_index != -1) out << ", tab_idx:" << node->tab_index;
    if (node->lex_level != -1) out << ", lev:" << node->lex_level;
    out << "]";
}

static void printNode(ASTNode* node, std::string prefix, bool isLast, std::ostream& out, bool dec) {
    if (!node) return;
    out << prefix << (isLast ? "└── " : "├── ");

    std::string next_pref = prefix + (isLast ? "    " : "│   ");

    if (auto n = dynamic_cast<ProgramNode*>(node)) {
        out << "ProgramNode(" << n->name << ")"; printDeco(n, out, dec); out << "\n";
        printNode(n->block, next_pref, true, out, dec);
    } 
    else if (auto n = dynamic_cast<BlockNode*>(node)) {
        out << "Block\n";
        for (size_t i = 0; i < n->declarations.size(); ++i) {
            printNode(n->declarations[i], next_pref, (i == n->declarations.size() - 1) && !n->compound_stmt, out, dec);
        }
        if (n->compound_stmt) printNode(n->compound_stmt, next_pref, true, out, dec);
    }
    else if (auto n = dynamic_cast<VarDeclNode*>(node)) {
        out << "VarDecl(";
        for(size_t i=0; i<n->names.size(); ++i) out << n->names[i] << (i+1==n->names.size()?"":",");
        out << ")"; printDeco(n, out, dec); out << "\n";
        printNode(n->type, next_pref, true, out, dec);
    }
    else if (auto n = dynamic_cast<CompoundStmtNode*>(node)) {
        out << "CompoundStmt"; printDeco(n, out, dec); out << "\n";
        for (size_t i = 0; i < n->statements.size(); ++i) {
            printNode(n->statements[i], next_pref, i == n->statements.size() - 1, out, dec);
        }
    }
    else if (auto n = dynamic_cast<AssignStmtNode*>(node)) {
        out << "AssignStmt"; printDeco(n, out, dec); out << "\n";
        printNode(n->target, next_pref, false, out, dec);
        printNode(n->value, next_pref, true, out, dec);
    }
    else if (auto n = dynamic_cast<VarAccessNode*>(node)) {
        out << "Var(" << n->name << ")"; printDeco(n, out, dec); out << "\n";
    }
    else if (auto n = dynamic_cast<LiteralNode*>(node)) {
        out << "Literal(" << n->value << " : " << n->lit_type << ")"; printDeco(n, out, dec); out << "\n";
    }
    else if (auto n = dynamic_cast<BinOpNode*>(node)) {
        out << "BinOp(" << n->op << ")"; printDeco(n, out, dec); out << "\n";
        printNode(n->left, next_pref, false, out, dec);
        printNode(n->right, next_pref, true, out, dec);
    }
    else if (auto n = dynamic_cast<NamedTypeNode*>(node)) {
        out << "Type(" << n->name << ")"; printDeco(n, out, dec); out << "\n";
    }
    else if (auto n = dynamic_cast<ProcCallStmtNode*>(node)) { // Perbaikan: Handle print ProcCall (writeln)
        out << "ProcCall(" << n->name << ")"; printDeco(n, out, dec); out << "\n";
        for (size_t i = 0; i < n->args.size(); ++i) {
            printNode(n->args[i], next_pref, i == n->args.size() - 1, out, dec);
        }
    }
    else if (dynamic_cast<EmptyStmtNode*>(node)) {
        out << "EmptyStmt\n";
    }
    else {
        out << "ASTNode"; printDeco(node, out, dec); out << "\n";
    }
}

void printAST(ASTNode* node, std::ostream& out, bool decorated) {
    if (!node) return;
    printNode(node, "", true, out, decorated);
}