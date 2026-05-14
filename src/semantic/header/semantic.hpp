#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "ast.hpp"
#include "symtab.hpp"
#include <vector>
#include <string>

class SemanticVisitor {
public:
    SymbolTable symtab;
    std::vector<std::string> errors;

    SemanticVisitor();

    bool hasErrors() const;
    void printErrors() const;
    void addError(const std::string& message);

    void visit(ProgramNode* node);
    void visit(BlockNode* node);
    
    void visit(NamedTypeNode* node);
    void visit(RangeNode* node);
    void visit(ArrayTypeNode* node);
    void visit(EnumTypeNode* node);
    void visit(RecordTypeNode* node);
    
    void visit(LiteralNode* node);
    void visit(VarAccessNode* node);
    void visit(BinOpNode* node);
    void visit(UnaryOpNode* node);
    void visit(FuncCallNode* node);
    
    void visit(AssignStmtNode* node);
    void visit(CompoundStmtNode* node);
    void visit(IfStmtNode* node);
    void visit(CaseBlockNode* node);
    void visit(CaseStmtNode* node);
    void visit(WhileStmtNode* node);
    void visit(RepeatStmtNode* node);
    void visit(ForStmtNode* node);
    void visit(ProcCallStmtNode* node);
    void visit(EmptyStmtNode* node);
    
    void visit(ConstDeclNode* node);
    void visit(TypeDeclNode* node);
    void visit(VarDeclNode* node);
    void visit(ParamNode* node);
    void visit(SubprogramDeclNode* node);

private:
    bool isTypeCompatible(const std::string& target_type, const std::string& value_type);
    std::string getResultType(const std::string& op, const std::string& left_type, const std::string& right_type);
};

#endif