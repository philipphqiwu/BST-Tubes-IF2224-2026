#include "header/semantic.hpp"
#include <iostream>
#include <algorithm>

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
    if (target_type == "real" && value_type == "integer") return true;
    return false;
}

std::string SemanticVisitor::getResultType(const std::string& op, const std::string& left_type, const std::string& right_type) {
    if (op == "plus" || op == "minus" || op == "times") {
        if (left_type == "real" || right_type == "real") return "real";
        if (left_type == "integer" && right_type == "integer") return "integer";
    } else if (op == "rdiv") {
        return "real";
    } else if (op == "idiv" || op == "imod") {
        if (left_type == "integer" && right_type == "integer") return "integer";
    } else if (op == "eql" || op == "neq" || op == "gtr" || op == "geq" || op == "lss" || op == "leq") {
        return "boolean";
    } else if (op == "andsy" || op == "orsy") {
        if (left_type == "boolean" && right_type == "boolean") return "boolean";
    }
    return "unknown";
}

void SemanticVisitor::visit(ProgramNode* node) {
    symtab.insertTab(node->name, ObjClass::PROGRAM, 0, 0, 1, 0);
    if (node->block) node->block->accept(*this);
}

void SemanticVisitor::visit(BlockNode* node) {
    for (auto decl : node->declarations) {
        if (decl) decl->accept(*this);
    }
    if (node->compound_stmt) {
        node->compound_stmt->accept(*this);
    }
}

void SemanticVisitor::visit(NamedTypeNode* node) { node->eval_type = node->name; }
void SemanticVisitor::visit(RangeNode* node) { node->eval_type = "range"; }
void SemanticVisitor::visit(ArrayTypeNode* node) { node->eval_type = "array"; }
void SemanticVisitor::visit(EnumTypeNode* node) { node->eval_type = "enum"; }
void SemanticVisitor::visit(RecordTypeNode* node) { node->eval_type = "record"; }

void SemanticVisitor::visit(LiteralNode* node) {
    if (node->lit_type == "intcon") node->eval_type = "integer";
    else if (node->lit_type == "realcon") node->eval_type = "real";
    else if (node->lit_type == "charcon") node->eval_type = "char";
    else if (node->lit_type == "string") node->eval_type = "string";
    else node->eval_type = "unknown";
}

void SemanticVisitor::visit(VarAccessNode* node) {
    int idx = symtab.lookup(node->name);
    if (idx == -1) {
        addError("Undeclared identifier: " + node->name);
        node->eval_type = "unknown";
    } else {
        node->tab_index = idx;
        node->lex_level = symtab.tab[idx].lev;
        node->eval_type = getTypeNameStr(symtab.tab[idx].type);
    }

    for (auto& comp : node->components) {
        if (comp.is_array_index) {
            for (auto expr : comp.indices) {
                expr->accept(*this);
                if (expr->eval_type != "integer" && expr->eval_type != "char") {
                    addError("Array index must be simple type, got: " + expr->eval_type);
                }
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
    if (idx == -1) addError("Undeclared FOR counter: " + node->counter_name);

    if (node->init_val) node->init_val->accept(*this);
    if (node->final_val) node->final_val->accept(*this);
    if (node->body) node->body->accept(*this);
}

void SemanticVisitor::visit(ProcCallStmtNode* node) {
    int idx = symtab.lookup(node->name);
    if (idx == -1) {
        addError("Undeclared procedure: " + node->name);
    } else {
        if (symtab.tab[idx].obj != ObjClass::PROCEDURE) {
            addError("Identifier " + node->name + " is not a procedure");
        }
    }
    for (auto arg : node->args) {
        if (arg) arg->accept(*this);
    }
}

void SemanticVisitor::visit(EmptyStmtNode* node) { }

void SemanticVisitor::visit(ConstDeclNode* node) {
    if (node->value) node->value->accept(*this);
    
    if (symtab.lookupLocal(node->name) != -1) {
        addError("Duplicate constant declaration: " + node->name);
        return;
    }
    
    std::string t_type = node->value ? node->value->eval_type : "unknown";
    symtab.insertTab(node->name, ObjClass::CONSTANT, getTypeIdVal(t_type), 0, 1, 0);
}

void SemanticVisitor::visit(TypeDeclNode* node) {
    if (symtab.lookupLocal(node->name) != -1) {
        addError("Duplicate type declaration: " + node->name);
        return;
    }
    symtab.insertTab(node->name, ObjClass::TYPE, 0, 0, 1, 0);
}

void SemanticVisitor::visit(VarDeclNode* node) {
    std::string t_name = "unknown";
    if (auto ntn = dynamic_cast<NamedTypeNode*>(node->type)) {
        t_name = ntn->name;
    } else if (dynamic_cast<ArrayTypeNode*>(node->type)) {
        t_name = "array";
    }

    int type_id = getTypeIdVal(t_name);

    for (const auto& name : node->names) {
        if (symtab.lookupLocal(name) != -1) {
            addError("Duplicate variable declaration: " + name);
        } else {
            symtab.insertTab(name, ObjClass::VARIABLE, type_id, 0, 1, 0);
        }
    }
}

void SemanticVisitor::visit(ParamNode* node) {
    std::string t_name = "unknown";
    if (auto ntn = dynamic_cast<NamedTypeNode*>(node->type)) {
        t_name = ntn->name;
    }

    int type_id = getTypeIdVal(t_name);
    for (const auto& name : node->names) {
        if (symtab.lookupLocal(name) != -1) {
            addError("Duplicate parameter name: " + name);
        } else {
            symtab.insertTab(name, ObjClass::VARIABLE, type_id, 0, 0, 0);
        }
    }
}

void SemanticVisitor::visit(SubprogramDeclNode* node) {
    if (symtab.lookupLocal(node->name) != -1) {
        addError("Duplicate subprogram declaration: " + node->name);
    } else {
        int ret_type = getTypeIdVal(node->return_type_name);
        ObjClass obj = node->is_function ? ObjClass::FUNCTION : ObjClass::PROCEDURE;
        symtab.insertTab(node->name, obj, ret_type, 0, 1, 0);
    }

    symtab.enterBlock();

    for (auto p : node->params) {
        if (p) p->accept(*this);
    }

    if (node->block) node->block->accept(*this);

    symtab.leaveBlock();
}