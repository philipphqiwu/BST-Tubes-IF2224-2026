#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include "parser/header/parser.hpp"
#include "ast.hpp"
#include <vector>
#include <string>

class ASTBuilder {
public:
    ProgramNode* build(ParseNode* parseTreeRoot);

private:
    std::string extractValue(const std::string& label);
    std::string extractType(const std::string& label);

    BlockNode* buildBlock(ParseNode* node);
    void buildDeclarationPart(ParseNode* node, std::vector<DeclNode*>& decls);
    TypeNode* buildType(ParseNode* node);
    
    StmtNode* buildStatement(ParseNode* node);
    CompoundStmtNode* buildCompoundStatement(ParseNode* node);
    
    ExprNode* buildExpression(ParseNode* node);
    ExprNode* buildSimpleExpression(ParseNode* node);
    ExprNode* buildTerm(ParseNode* node);
    ExprNode* buildFactor(ParseNode* node);
    VarAccessNode* buildVariable(ParseNode* node);
};

#endif