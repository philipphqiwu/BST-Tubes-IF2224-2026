#include "header/semantic.hpp"
#include <iostream>
#include <algorithm>

static bool parseLiteralValue(const ExprNode* node, long long& value) {
    const auto* lit = dynamic_cast<const LiteralNode*>(node);
    if (!lit) return false;

    try {
        if (lit->lit_type == "intcon") {
            value = std::stoll(lit->value);
            return true;
        }
        if (lit->lit_type == "charcon" && !lit->value.empty()) {
            value = static_cast<unsigned char>(lit->value[0]);
            return true;
        }
    } catch (...) {
    }

    return false;
}

static bool validateRangeType(SemanticVisitor* visitor, RangeNode* rangeNode, const std::string& context) {
    if (!rangeNode) return false;

    if (rangeNode->low) rangeNode->low->accept(*visitor);
    if (rangeNode->high) rangeNode->high->accept(*visitor);

    if ((rangeNode->low && rangeNode->low->eval_type == "real") ||
        (rangeNode->high && rangeNode->high->eval_type == "real")) {
        visitor->addError(context + ": subrange type cannot use Real type");
        rangeNode->eval_type = "unknown";
        return false;
    }

    long long low_val = 0;
    long long high_val = 0;
    if (parseLiteralValue(rangeNode->low, low_val) && parseLiteralValue(rangeNode->high, high_val)) {
        if (low_val > high_val) {
            visitor->addError(context + ": lower bound cannot be greater than upper bound");
            rangeNode->eval_type = "unknown";
            return false;
        }
    }

    rangeNode->eval_type = "range";
    return true;
}

SemanticVisitor::SemanticVisitor() {}

bool SemanticVisitor::hasErrors() const {
    return !errors.empty();
}

void SemanticVisitor::printErrors() const {
    for (const auto& e : errors) {
        std::cout << "Semantic Error: " << e << "\n";
    }
}

void SemanticVisitor::addError(const std::string& message) {
    errors.push_back(message);
}

static std::string getTypeNameStr(int type_id) {
    switch(type_id) {
        case 1: return "integer";
        case 2: return "real";
        case 3: return "boolean";
        case 4: return "char";
        case 5: return "string";
        default: return "unknown";
    }
}

static int getTypeIdVal(const std::string& name) {
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    if (lower_name == "integer") return 1;
    if (lower_name == "real") return 2;
    if (lower_name == "boolean") return 3;
    if (lower_name == "char") return 4;
    if (lower_name == "string") return 5;
    return 0;
}

bool SemanticVisitor::isTypeCompatible(const std::string& target_type, const std::string& value_type) {
    if (target_type == value_type) return true;
    // Assignment-compatible: Real := Integer is allowed
    if (target_type == "real" && value_type == "integer") return true;
    return false;
}

std::string SemanticVisitor::getResultType(const std::string& op, const std::string& left_type, const std::string& right_type) {
    // Arithmetic operators
    if (op == "plus" || op == "minus" || op == "times") {
        if (left_type == "real" || right_type == "real") return "real";
        if (left_type == "integer" && right_type == "integer") return "integer";
        // String concatenation
        if (op == "plus" && left_type == "string" && right_type == "string") return "string";
    } else if (op == "rdiv") {
        return "real";
    } else if (op == "idiv" || op == "imod") {
        if (left_type == "integer" && right_type == "integer") return "integer";
    }
    // Relational operators → always boolean
    else if (op == "eql" || op == "neq" || op == "gtr" || op == "geq" || op == "lss" || op == "leq") {
        return "boolean";
    }
    // Logical operators
    else if (op == "andsy" || op == "orsy") {
        if (left_type == "boolean" && right_type == "boolean") return "boolean";
    }
    return "unknown";
}

// ======================== PROGRAM ========================
void SemanticVisitor::visit(ProgramNode* node) {
    // Insert program name into tab
    int idx = symtab.insertTab(node->name, ObjClass::PROGRAM, 0, 0, 1, 0);
    node->tab_index = idx;
    node->lex_level = 0;

    if (!node->block) return;

    // Declarations belong to the global block (level 0)
    for (auto decl : node->block->declarations) {
        if (decl) decl->accept(*this);
    }

    // Main compound statement gets its own block (level 1)
    symtab.enterBlock();
    node->block->block_index = symtab.display[symtab.current_level];
    node->block->lex_level = symtab.current_level;

    if (node->block->compound_stmt) {
        node->block->compound_stmt->accept(*this);
    }

    symtab.leaveBlock();

    // Update btab[0].vsze with the total variable size in global block
    // vsze is already managed in VarDeclNode
}

// ======================== BLOCK ========================
void SemanticVisitor::visit(BlockNode* node) {
    node->block_index = symtab.display[symtab.current_level];
    node->lex_level = symtab.current_level;
    for (auto decl : node->declarations) {
        if (decl) decl->accept(*this);
    }
    if (node->compound_stmt) {
        node->compound_stmt->accept(*this);
    }
}

// ======================== TYPE NODES ========================
void SemanticVisitor::visit(NamedTypeNode* node) {
    node->eval_type = node->name;
}
void SemanticVisitor::visit(RangeNode* node) {
    validateRangeType(this, node, "Subrange");
}
void SemanticVisitor::visit(ArrayTypeNode* node) {
    if (node->index_type) node->index_type->accept(*this);
    if (node->element_type) node->element_type->accept(*this);
    node->eval_type = "array";
}
void SemanticVisitor::visit(EnumTypeNode* node) {
    node->eval_type = "enum";
}
void SemanticVisitor::visit(RecordTypeNode* node) {
    node->eval_type = "record";
}

// ======================== EXPRESSION NODES ========================
void SemanticVisitor::visit(LiteralNode* node) {
    if (node->lit_type == "intcon")
        node->eval_type = "integer";
    else if (node->lit_type == "realcon")
        node->eval_type = "real";
    else if (node->lit_type == "charcon")
        node->eval_type = "char";
    else if (node->lit_type == "string")
        node->eval_type = "string";
    else
        node->eval_type = "unknown";
}

void SemanticVisitor::visit(VarAccessNode* node) {
    int idx = symtab.lookup(node->name);
    if (idx == -1) {
        addError("Undeclared identifier: " + node->name);
        node->eval_type = "unknown";
    } else {
        node->tab_index = idx;
        node->lex_level = symtab.tab[idx].lev;

        // Determine the base type
        std::string base_type = getTypeNameStr(symtab.tab[idx].type);
        int current_ref = symtab.tab[idx].ref;

        // If type is 0 (structured type), determine from ref
        if (symtab.tab[idx].type == 0 && current_ref > 0) {
            // Could be array (ref → atab) or record (ref → btab)
            if (current_ref < (int)symtab.atab.size()) {
                base_type = "array";
            } else {
                base_type = "record";
            }
        }

        node->eval_type = base_type;
    }

    // Resolve type through component access (array indexing, field access)
    for (auto& comp : node->components) {
        if (comp.is_array_index) {
            // Array indexing: resolve to element type
            for (auto expr : comp.indices) {
                expr->accept(*this);
                if (expr->eval_type != "integer" && expr->eval_type != "char") {
                    addError("Array index must be simple type, got: " + expr->eval_type);
                }
            }
            // Resolve element type from atab if possible
            if (idx != -1 && symtab.tab[idx].ref >= 0 && symtab.tab[idx].ref < (int)symtab.atab.size()) {
                int etyp = symtab.atab[symtab.tab[idx].ref].etyp;
                node->eval_type = getTypeNameStr(etyp);
            }
        } else if (!comp.field_name.empty()) {
            // Record field access: resolve to field type
            int field_idx = symtab.lookup(comp.field_name);
            if (field_idx != -1) {
                node->eval_type = getTypeNameStr(symtab.tab[field_idx].type);
            }
        }
    }
}

void SemanticVisitor::visit(BinOpNode* node) {
    if (node->left) node->left->accept(*this);
    if (node->right) node->right->accept(*this);

    std::string l_type = node->left ? node->left->eval_type : "unknown";
    std::string r_type = node->right ? node->right->eval_type : "unknown";

    node->eval_type = getResultType(node->op, l_type, r_type);

    if (node->eval_type == "unknown") {
        addError("Invalid types for operator " + node->op + ": " + l_type + " and " + r_type);
    }
}

void SemanticVisitor::visit(UnaryOpNode* node) {
    if (node->right) node->right->accept(*this);
    std::string r_type = node->right ? node->right->eval_type : "unknown";

    if (node->op == "notsy") {
        if (r_type != "boolean") addError("Operator NOT requires boolean operand");
        node->eval_type = "boolean";
    } else {
        if (r_type != "integer" && r_type != "real") addError("Unary operator requires numeric operand");
        node->eval_type = r_type;
    }
}

void SemanticVisitor::visit(FuncCallNode* node) {
    int idx = symtab.lookup(node->name);
    if (idx == -1) {
        addError("Undeclared function: " + node->name);
        node->eval_type = "unknown";
    } else {
        if (symtab.tab[idx].obj != ObjClass::FUNCTION && symtab.tab[idx].obj != ObjClass::TYPE) {
            addError("Identifier " + node->name + " is not a function");
        }
        node->tab_index = idx;
        node->lex_level = symtab.tab[idx].lev;
        node->eval_type = getTypeNameStr(symtab.tab[idx].type);
    }

    for (auto arg : node->args) {
        if (arg) arg->accept(*this);
    }
}

// ======================== STATEMENT NODES ========================
void SemanticVisitor::visit(AssignStmtNode* node) {
    if (node->target) node->target->accept(*this);
    if (node->value) node->value->accept(*this);

    std::string t_type = node->target ? node->target->eval_type : "unknown";
    std::string v_type = node->value ? node->value->eval_type : "unknown";

    if (t_type != "unknown" && v_type != "unknown") {
        if (!isTypeCompatible(t_type, v_type)) {
            addError("Type mismatch in assignment: cannot assign " + v_type + " to " + t_type);
        }
    }
}

void SemanticVisitor::visit(CompoundStmtNode* node) {
    for (auto stmt : node->statements) {
        if (stmt) stmt->accept(*this);
    }
}

void SemanticVisitor::visit(IfStmtNode* node) {
    if (node->condition) {
        node->condition->accept(*this);
        if (node->condition->eval_type != "boolean") {
            addError("IF condition must evaluate to boolean, got " + node->condition->eval_type);
        }
    }
    if (node->then_stmt) node->then_stmt->accept(*this);
    if (node->else_stmt) node->else_stmt->accept(*this);
}

void SemanticVisitor::visit(CaseBlockNode* node) {
    for (auto c : node->constants) {
        if (c) c->accept(*this);
    }
    if (node->statement) node->statement->accept(*this);
}

void SemanticVisitor::visit(CaseStmtNode* node) {
    if (node->expression) node->expression->accept(*this);
    for (auto b : node->blocks) {
        if (b) b->accept(*this);
    }
}

void SemanticVisitor::visit(WhileStmtNode* node) {
    if (node->condition) {
        node->condition->accept(*this);
        if (node->condition->eval_type != "boolean") {
            addError("WHILE condition must evaluate to boolean");
        }
    }
    if (node->body) node->body->accept(*this);
}

void SemanticVisitor::visit(RepeatStmtNode* node) {
    for (auto stmt : node->body) {
        if (stmt) stmt->accept(*this);
    }
    if (node->condition) {
        node->condition->accept(*this);
        if (node->condition->eval_type != "boolean") {
            addError("UNTIL condition must evaluate to boolean");
        }
    }
}

void SemanticVisitor::visit(ForStmtNode* node) {
    int idx = symtab.lookup(node->counter_name);
    if (idx == -1) {
        addError("Undeclared FOR counter: " + node->counter_name);
    } else {
        node->tab_index = idx;
        node->lex_level = symtab.tab[idx].lev;
    }

    if (node->init_val) node->init_val->accept(*this);
    if (node->final_val) node->final_val->accept(*this);
    if (node->body) node->body->accept(*this);
}

void SemanticVisitor::visit(ProcCallStmtNode* node) {
    int idx = symtab.lookup(node->name);
    if (idx == -1) {
        std::string lower_name = node->name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (lower_name == "writeln" || lower_name == "readln") {
            idx = symtab.insertTabAtLevel(0, lower_name, ObjClass::PROCEDURE, 0, 0, 1, 0);
        } else {
            addError("Undeclared procedure: " + node->name);
        }
    } else {
        if (symtab.tab[idx].obj != ObjClass::PROCEDURE) {
            addError("Identifier " + node->name + " is not a procedure");
        }
    }
    if (idx != -1) {
        node->tab_index = idx;
        node->lex_level = symtab.tab[idx].lev;
    }
    for (auto arg : node->args) {
        if (arg) arg->accept(*this);
    }
}

void SemanticVisitor::visit(EmptyStmtNode* node) {}

// ======================== DECLARATION NODES ========================
void SemanticVisitor::visit(ConstDeclNode* node) {
    if (node->value) node->value->accept(*this);

    if (symtab.lookupLocal(node->name) != -1) {
        addError("Duplicate constant declaration: " + node->name);
        return;
    }

    std::string t_type = node->value ? node->value->eval_type : "unknown";
    int adr_val = 0;
    // Store constant value in adr if it's a literal integer
    if (auto lit = dynamic_cast<LiteralNode*>(node->value)) {
        if (lit->lit_type == "intcon") {
            try {
                adr_val = std::stoi(lit->value);
            } catch (...) {
            }
        }
    }
    int idx = symtab.insertTab(node->name, ObjClass::CONSTANT, getTypeIdVal(t_type), 0, 1, adr_val);
    node->tab_index = idx;
    node->lex_level = symtab.tab[idx].lev;
    node->eval_type = t_type;
}

void SemanticVisitor::visit(TypeDeclNode* node) {
    if (symtab.lookupLocal(node->name) != -1) {
        addError("Duplicate type declaration: " + node->name);
        return;
    }

    int ref = 0;
    int typeCode = 0;

    // Handle record type: insert btab entry
    if (auto recType = dynamic_cast<RecordTypeNode*>(node->type)) {
        int btab_idx = symtab.insertBTab();
        ref = btab_idx;
        // Register fields inside record
        for (auto field : recType->fields) {
            std::string ft_name = "unknown";
            if (auto fnt = dynamic_cast<NamedTypeNode*>(field->type)) ft_name = fnt->name;
            int ft_id = getTypeIdVal(ft_name);
            for (const auto& fname : field->names) {
                symtab.insertTab(fname, ObjClass::FIELD, ft_id, 0, 1, 0);
            }
        }
        symtab.btab[btab_idx].last = symtab.tab.size() - 1;
    }
    // Handle array type: insert atab entry
    else if (auto arrType = dynamic_cast<ArrayTypeNode*>(node->type)) {
        // Process the array type
        int xtyp = 1; // default index type = integer
        int etyp = 1; // default element type = integer
        int eref = 0;
        int low = 0, high = 0;

        if (auto rng = dynamic_cast<RangeNode*>(arrType->index_type)) {
            if (auto ll = dynamic_cast<LiteralNode*>(rng->low)) {
                try {
                    low = std::stoi(ll->value);
                } catch (...) {
                }
            }
            if (auto lh = dynamic_cast<LiteralNode*>(rng->high)) {
                try {
                    high = std::stoi(lh->value);
                } catch (...) {
                }
            }
        }
        if (auto nt = dynamic_cast<NamedTypeNode*>(arrType->element_type)) {
            etyp = getTypeIdVal(nt->name);
        }
        int elsz = 1;
        int size = (high - low + 1) * elsz;
        int atab_idx = symtab.insertATab(xtyp, etyp, eref, low, high, elsz, size);
        ref = atab_idx;
    }
    else if (auto rngType = dynamic_cast<RangeNode*>(node->type)) {
        if (!validateRangeType(this, rngType, "Subrange")) {
            typeCode = 0;
        } else {
            typeCode = 0;
        }
    }

    int idx = symtab.insertTab(node->name, ObjClass::TYPE, typeCode, ref, 1, 0);
    node->tab_index = idx;
    node->lex_level = symtab.tab[idx].lev;

    // Visit the type node for further processing
    if (node->type) node->type->accept(*this);
}

void SemanticVisitor::visit(VarDeclNode* node) {
    std::string t_name = "unknown";
    int ref = 0;

    if (auto ntn = dynamic_cast<NamedTypeNode*>(node->type)) {
        t_name = ntn->name;
    } else if (auto rngType = dynamic_cast<RangeNode*>(node->type)) {
        if (validateRangeType(this, rngType, "Subrange")) {
            t_name = "subrange";
        }
    } else if (auto arrType = dynamic_cast<ArrayTypeNode*>(node->type)) {
        t_name = "array";
        // Insert anonymous array into atab
        int xtyp = 1;
        int etyp = 1;
        int eref = 0;
        int low = 0, high = 0;

        if (auto rng = dynamic_cast<RangeNode*>(arrType->index_type)) {
            if (auto ll = dynamic_cast<LiteralNode*>(rng->low)) {
                try {
                    low = std::stoi(ll->value);
                } catch (...) {
                }
            }
            if (auto lh = dynamic_cast<LiteralNode*>(rng->high)) {
                try {
                    high = std::stoi(lh->value);
                } catch (...) {
                }
            }
        }
        if (auto nt = dynamic_cast<NamedTypeNode*>(arrType->element_type)) {
            etyp = getTypeIdVal(nt->name);
        }
        int elsz = 1;
        int size = (high - low + 1) * elsz;
        int atab_idx = symtab.insertATab(xtyp, etyp, eref, low, high, elsz, size);
        ref = atab_idx;
    } else if (dynamic_cast<RecordTypeNode*>(node->type)) {
        t_name = "record";
        int btab_idx = symtab.insertBTab();
        ref = btab_idx;
        auto recType = dynamic_cast<RecordTypeNode*>(node->type);
        for (auto field : recType->fields) {
            std::string ft_name = "unknown";
            if (auto fnt = dynamic_cast<NamedTypeNode*>(field->type)) ft_name = fnt->name;
            int ft_id = getTypeIdVal(ft_name);
            for (const auto& fname : field->names) {
                symtab.insertTab(fname, ObjClass::FIELD, ft_id, 0, 1, 0);
            }
        }
        symtab.btab[btab_idx].last = symtab.tab.size() - 1;
    }

    int type_id = getTypeIdVal(t_name);
    // Get current block to track vsze
    int btab_idx = symtab.display[symtab.current_level];

    for (size_t i = 0; i < node->names.size(); ++i) {
        const auto& name = node->names[i];
        if (symtab.lookupLocal(name) != -1) {
            addError("Duplicate variable declaration: " + name);
        } else {
            // adr = current variable offset in the block
            int adr = symtab.btab[btab_idx].vsze;
            int var_size = 1; // default size for simple types
            if (t_name == "array") {
                // Array variable size = array total size from atab
                if (ref >= 0 && ref < (int)symtab.atab.size()) {
                    var_size = symtab.atab[ref].size;
                }
            }
            int idx = symtab.insertTab(name, ObjClass::VARIABLE, type_id, ref, 1, adr);
            symtab.btab[btab_idx].vsze += var_size;

            // Store per-variable tab index
            node->tab_indices.push_back(idx);

            // Annotate the VarDeclNode
            if (i == 0) {
                node->tab_index = idx;
                node->lex_level = symtab.tab[idx].lev;
            }
        }
    }
    node->eval_type = t_name;
}

void SemanticVisitor::visit(ParamNode* node) {
    std::string t_name = "unknown";
    if (auto ntn = dynamic_cast<NamedTypeNode*>(node->type)) {
        t_name = ntn->name;
    }

    int type_id = getTypeIdVal(t_name);
    int btab_idx = symtab.display[symtab.current_level];

    for (const auto& name : node->names) {
        if (symtab.lookupLocal(name) != -1) {
            addError("Duplicate parameter name: " + name);
        } else {
            int adr = symtab.btab[btab_idx].vsze;
            symtab.insertTab(name, ObjClass::VARIABLE, type_id, 0, 0, adr);
            symtab.btab[btab_idx].vsze += 1;
            symtab.btab[btab_idx].psze += 1;
            symtab.btab[btab_idx].lpar = symtab.tab.size() - 1;
        }
    }
}

void SemanticVisitor::visit(SubprogramDeclNode* node) {
    if (symtab.lookupLocal(node->name) != -1) {
        addError("Duplicate subprogram declaration: " + node->name);
    } else {
        int ret_type = getTypeIdVal(node->return_type_name);
        ObjClass obj = node->is_function ? ObjClass::FUNCTION : ObjClass::PROCEDURE;
        int idx = symtab.insertTab(node->name, obj, ret_type, 0, 1, 0);
        node->tab_index = idx;
        node->lex_level = symtab.tab[idx].lev;

        // Set ref to the btab block that will be created
        symtab.tab[idx].ref = symtab.btab.size(); // will point to the next block
    }

    symtab.enterBlock();

    // Update the subprogram's ref to point to this block
    if (node->tab_index != -1) {
        symtab.tab[node->tab_index].ref = symtab.display[symtab.current_level];
    }

    for (auto p : node->params) {
        if (p) p->accept(*this);
    }

    if (node->block) node->block->accept(*this);

    symtab.leaveBlock();
}