#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>

class SemanticVisitor;

class ASTNode {
public:
    int tab_index = -1;
    int lex_level = -1;
    std::string eval_type = "";

    virtual ~ASTNode() = default;
    virtual void accept(SemanticVisitor& visitor) = 0;
};

class TypeNode : public ASTNode {};
class ExprNode : public ASTNode {};
class StmtNode : public ASTNode {};
class DeclNode : public ASTNode {};

class NamedTypeNode : public TypeNode {
public:
    std::string name;
    NamedTypeNode(std::string name) : name(name) {}
    void accept(SemanticVisitor& visitor) override;
};

class RangeNode : public TypeNode {
public:
    ExprNode* low;
    ExprNode* high;
    RangeNode(ExprNode* l, ExprNode* h) : low(l), high(h) {}
    ~RangeNode() { delete low; delete high; }
    void accept(SemanticVisitor& visitor) override;
};

class ArrayTypeNode : public TypeNode {
public:
    TypeNode* index_type;
    TypeNode* element_type;
    ArrayTypeNode(TypeNode* idx, TypeNode* el) : index_type(idx), element_type(el) {}
    ~ArrayTypeNode() { delete index_type; delete element_type; }
    void accept(SemanticVisitor& visitor) override;
};

class EnumTypeNode : public TypeNode {
public:
    std::vector<std::string> identifiers;
    EnumTypeNode(std::vector<std::string> idents) : identifiers(idents) {}
    void accept(SemanticVisitor& visitor) override;
};

struct FieldNode {
    std::vector<std::string> names;
    TypeNode* type;
    ~FieldNode() { delete type; }
};

class RecordTypeNode : public TypeNode {
public:
    std::vector<FieldNode*> fields;
    ~RecordTypeNode() { for (auto f : fields) delete f; }
    void accept(SemanticVisitor& visitor) override;
};

class LiteralNode : public ExprNode {
public:
    std::string value;
    std::string lit_type;
    LiteralNode(std::string val, std::string type) : value(val), lit_type(type) {}
    void accept(SemanticVisitor& visitor) override;
};

struct VarComponent {
    bool is_array_index;
    std::vector<ExprNode*> indices;
    std::string field_name;
};

class VarAccessNode : public ExprNode {
public:
    std::string name;
    std::vector<VarComponent> components;
    VarAccessNode(std::string name) : name(name) {}
    ~VarAccessNode() {
        for (auto& comp : components) {
            for (auto idx : comp.indices) delete idx;
        }
    }
    void accept(SemanticVisitor& visitor) override;
};

class BinOpNode : public ExprNode {
public:
    std::string op;
    ExprNode* left;
    ExprNode* right;
    BinOpNode(std::string op, ExprNode* l, ExprNode* r) : op(op), left(l), right(r) {}
    ~BinOpNode() { delete left; delete right; }
    void accept(SemanticVisitor& visitor) override;
};

class UnaryOpNode : public ExprNode {
public:
    std::string op;
    ExprNode* right;
    UnaryOpNode(std::string op, ExprNode* r) : op(op), right(r) {}
    ~UnaryOpNode() { delete right; }
    void accept(SemanticVisitor& visitor) override;
};

class FuncCallNode : public ExprNode {
public:
    std::string name;
    std::vector<ExprNode*> args;
    FuncCallNode(std::string name, std::vector<ExprNode*> args) : name(name), args(args) {}
    ~FuncCallNode() { for (auto arg : args) delete arg; }
    void accept(SemanticVisitor& visitor) override;
};

class AssignStmtNode : public StmtNode {
public:
    VarAccessNode* target;
    ExprNode* value;
    AssignStmtNode(VarAccessNode* t, ExprNode* v) : target(t), value(v) {}
    ~AssignStmtNode() { delete target; delete value; }
    void accept(SemanticVisitor& visitor) override;
};

class CompoundStmtNode : public StmtNode {
public:
    std::vector<StmtNode*> statements;
    ~CompoundStmtNode() { for (auto stmt : statements) delete stmt; }
    void accept(SemanticVisitor& visitor) override;
};

class IfStmtNode : public StmtNode {
public:
    ExprNode* condition;
    StmtNode* then_stmt;
    StmtNode* else_stmt;
    IfStmtNode(ExprNode* cond, StmtNode* t, StmtNode* e = nullptr) 
        : condition(cond), then_stmt(t), else_stmt(e) {}
    ~IfStmtNode() { delete condition; delete then_stmt; delete else_stmt; }
    void accept(SemanticVisitor& visitor) override;
};

class CaseBlockNode : public ASTNode {
public:
    std::vector<ExprNode*> constants;
    StmtNode* statement;
    ~CaseBlockNode() {
        for (auto c : constants) delete c;
        delete statement;
    }
    void accept(SemanticVisitor& visitor) override;
};

class CaseStmtNode : public StmtNode {
public:
    ExprNode* expression;
    std::vector<CaseBlockNode*> blocks;
    CaseStmtNode(ExprNode* expr) : expression(expr) {}
    ~CaseStmtNode() {
        delete expression;
        for (auto b : blocks) delete b;
    }
    void accept(SemanticVisitor& visitor) override;
};

class WhileStmtNode : public StmtNode {
public:
    ExprNode* condition;
    StmtNode* body;
    WhileStmtNode(ExprNode* cond, StmtNode* b) : condition(cond), body(b) {}
    ~WhileStmtNode() { delete condition; delete body; }
    void accept(SemanticVisitor& visitor) override;
};

class RepeatStmtNode : public StmtNode {
public:
    std::vector<StmtNode*> body;
    ExprNode* condition;
    ~RepeatStmtNode() {
        for (auto s : body) delete s;
        delete condition;
    }
    void accept(SemanticVisitor& visitor) override;
};

class ForStmtNode : public StmtNode {
public:
    std::string counter_name;
    ExprNode* init_val;
    std::string direction;
    ExprNode* final_val;
    StmtNode* body;
    ForStmtNode(std::string name, ExprNode* i, std::string dir, ExprNode* f, StmtNode* b)
        : counter_name(name), init_val(i), direction(dir), final_val(f), body(b) {}
    ~ForStmtNode() { delete init_val; delete final_val; delete body; }
    void accept(SemanticVisitor& visitor) override;
};

class ProcCallStmtNode : public StmtNode {
public:
    std::string name;
    std::vector<ExprNode*> args;
    ProcCallStmtNode(std::string name, std::vector<ExprNode*> args) : name(name), args(args) {}
    ~ProcCallStmtNode() { for (auto arg : args) delete arg; }
    void accept(SemanticVisitor& visitor) override;
};

class EmptyStmtNode : public StmtNode {
public:
    void accept(SemanticVisitor& visitor) override;
};

class ConstDeclNode : public DeclNode {
public:
    std::string name;
    ExprNode* value;
    ConstDeclNode(std::string name, ExprNode* val) : name(name), value(val) {}
    ~ConstDeclNode() { delete value; }
    void accept(SemanticVisitor& visitor) override;
};

class TypeDeclNode : public DeclNode {
public:
    std::string name;
    TypeNode* type;
    TypeDeclNode(std::string name, TypeNode* t) : name(name), type(t) {}
    ~TypeDeclNode() { delete type; }
    void accept(SemanticVisitor& visitor) override;
};

class VarDeclNode : public DeclNode {
public:
    std::vector<std::string> names;
    TypeNode* type;
    VarDeclNode(std::vector<std::string> names, TypeNode* t) : names(names), type(t) {}
    ~VarDeclNode() { delete type; }
    void accept(SemanticVisitor& visitor) override;
};

class ParamNode : public ASTNode {
public:
    std::vector<std::string> names;
    TypeNode* type;
    ParamNode(std::vector<std::string> n, TypeNode* t) : names(n), type(t) {}
    ~ParamNode() { delete type; }
    void accept(SemanticVisitor& visitor) override;
};

class BlockNode;

class SubprogramDeclNode : public DeclNode {
public:
    bool is_function;
    std::string name;
    std::vector<ParamNode*> params;
    std::string return_type_name;
    BlockNode* block;

    SubprogramDeclNode(bool is_func, std::string n, std::vector<ParamNode*> p, std::string ret_type, BlockNode* b)
        : is_function(is_func), name(n), params(p), return_type_name(ret_type), block(b) {}
    ~SubprogramDeclNode();
    void accept(SemanticVisitor& visitor) override;
};

class BlockNode : public ASTNode {
public:
    std::vector<DeclNode*> declarations;
    CompoundStmtNode* compound_stmt;
    ~BlockNode() {
        for (auto decl : declarations) delete decl;
        delete compound_stmt;
    }
    void accept(SemanticVisitor& visitor) override;
};

inline SubprogramDeclNode::~SubprogramDeclNode() {
    for (auto p : params) delete p;
    delete block;
}

class ProgramNode : public ASTNode {
public:
    std::string name;
    BlockNode* block;
    ProgramNode(std::string name, BlockNode* b) : name(name), block(b) {}
    ~ProgramNode() { delete block; }
    void accept(SemanticVisitor& visitor) override;
};

#endif