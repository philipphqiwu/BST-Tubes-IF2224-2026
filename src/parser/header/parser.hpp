#ifndef PARSER_HPP
#define PARSER_HPP
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <initializer_list>

struct Token {
    std::string type;
    std::string value;
    int line;

    Token(std::string t = "", std::string v = "", int l = 0) : type(std::move(t)), value(std::move(v)), line(l) {}

    std::string display() const {
        if (value.empty()) return type;
        return type + "(" + value + ")";
    }
};

struct ParseNode {
    std::string label;
    std::vector<ParseNode*> children;

    explicit ParseNode(std::string lbl) : label(std::move(lbl)) {}
    void addChild(ParseNode* c) { if (c) children.push_back(c); }
    ~ParseNode() { for (auto* c : children) delete c; }
};

class Parser {
    public:
        Parser(std::vector<Token> toks);
        ParseNode* parse();
        bool hasErrors() const {return !errors.empty();}
        const std::vector<std::string>& getErrors() const {return errors;}
    private:
        std::vector<Token> tokens;
        int pos;
        std::vector<std::string> errors;

        const Token& cur() const;
        const Token& peek(int off = 1) const;
        bool atEnd() const;
        bool check(const std::string& t) const;
        bool checkAny(std::initializer_list<std::string> ts) const;

        ParseNode* expect(const std::string& type);
        ParseNode* leafFromCur() const;

        void addError(const std::string& msg);

        void synchronize(std::initializer_list<std::string> follow);

        // syntax analyzer
        ParseNode* parseProgram();
        ParseNode* parseProgramHeader();
        ParseNode* parseDeclarationPart();

        ParseNode* parseConstDeclaration();
        ParseNode* parseConstant();

        ParseNode* parseTypeDeclaration();

        ParseNode* parseVarDeclaration();
        ParseNode* parseIdentifierList();

        ParseNode* parseType();
        ParseNode* parseArrayType();
        ParseNode* parseRange();
        ParseNode* parseEnumerated();
        ParseNode* parseRecordType();
        ParseNode* parseFieldList();
        ParseNode* parseFieldPart();

        ParseNode* parseSubprogramDeclaration();
        ParseNode* parseProcedureDeclaration();
        ParseNode* parseFunctionDeclaration();
        ParseNode* parseBlock();
        ParseNode* parseFormalParameterList();
        ParseNode* parseParameterGroup();

        ParseNode* parseCompoundStatement();
        ParseNode* parseStatementList(const std::string& terminator);
        ParseNode* parseStatement();
        ParseNode* parseAssignmentStatement();
        ParseNode* parseIfStatement();
        ParseNode* parseCaseStatement();
        ParseNode* parseCaseBlock();
        ParseNode* parseWhileStatement();
        ParseNode* parseRepeatStatement();
        ParseNode* parseForStatement();
        ParseNode* parseProcFuncCall();
        ParseNode* parseParameterList();

        ParseNode* parseExpression();
        ParseNode* parseSimpleExpression();
        ParseNode* parseTerm();
        ParseNode* parseFactor();

        ParseNode* parseRelationalOperator();
        ParseNode* parseAdditiveOperator();
        ParseNode* parseMultiplicativeOperator();

        // helper
        bool isConstantStart(int off = 0) const;
        bool isRangeHere() const;
        bool isStatementStart() const;
        bool isRelOp() const;
        bool isAddOp() const;
        bool isMulOp() const;
};

std::vector<Token> readTokensFromFile(const std::string& path);
void printTree(ParseNode* node, std::ostream& out);
void runParser(const std::vector<Token>& tokens, std::ostream& out);
#endif