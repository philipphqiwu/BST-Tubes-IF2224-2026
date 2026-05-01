#include "header/parser.hpp"
#include <regex>
#include <stdexcept>

// token reader from lexer output
std::vector<Token> readTokensFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Tidak dapat membuka file token: " + path);
    std::vector<Token> toks;
    std::string line;
    int lineno = 0;
    std::regex valued(R"(^(\w+)\s+\((.+)\)$)");
    while (std::getline(f, line)) {
        ++lineno;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line == "comment") continue;
        std::smatch m;
        if (std::regex_match(line, m, valued)) toks.emplace_back(m[1].str(), m[2].str(), lineno);
        else toks.emplace_back(line, "", lineno);
    }
    return toks;
}

// print tree
static void printHelper(ParseNode* n, const std::string& pfx, bool isLast, std::ostream& out) {
    if (!n) return;
    out << pfx << (isLast ? "└── " : "├── ") << n->label << "\n";
    std::string child_pfx = pfx + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < n->children.size(); ++i) printHelper(n->children[i], child_pfx, i == n->children.size() - 1, out);
}

void printTree(ParseNode* node, std::ostream& out) {
    if (!node) return;
    out << node->label << "\n";
    for (size_t i = 0; i < node->children.size(); ++i)
        printHelper(node->children[i], "",
                    i == node->children.size() - 1, out);
}

// constructor 
Parser::Parser(std::vector<Token> toks)
    : tokens(std::move(toks)), pos(0) {
    tokens.emplace_back("EOF", "", -1);
}

static const Token EOF_TOKEN{"EOF", "", -1};

const Token& Parser::cur() const {
    if (pos < (int)tokens.size()) return tokens[pos];
    return EOF_TOKEN;
}
const Token& Parser::peek(int off) const {
    int i = pos + off;
    if (i < (int)tokens.size()) return tokens[i];
    return EOF_TOKEN;
}
bool Parser::atEnd() const { return cur().type == "EOF"; }
bool Parser::check(const std::string& t) const { return cur().type == t; }
bool Parser::checkAny(std::initializer_list<std::string> ts) const {
    for (auto& t : ts) if (cur().type == t) return true;
    return false;
}

void Parser::addError(const std::string& msg) { errors.push_back(msg); }
ParseNode* Parser::leafFromCur() const {return new ParseNode(cur().display());}

ParseNode* Parser::expect(const std::string& type) {
    if (cur().type == type) {
        auto* n = new ParseNode(cur().display());
        ++pos;
        return n;
    }
    std::string msg = "Syntax error: expected \"" + type + "\", got \"" + cur().display() + "\"";
    if (cur().line > 0) msg += " (near token line " + std::to_string(cur().line) + ")";
    addError(msg);
    return new ParseNode("[missing:" + type + "]");
}

void Parser::synchronize(std::initializer_list<std::string> follow) {
    while (!atEnd()) {
        for (auto& t : follow) if (cur().type == t) return;
        ++pos;
    }
}

// top-level parser
ParseNode* Parser::parse() { return parseProgram(); }

ParseNode* Parser::parseProgram() {
    auto* n = new ParseNode("<program>");
    n->addChild(parseProgramHeader());
    n->addChild(parseDeclarationPart());
    n->addChild(parseCompoundStatement());
    n->addChild(expect("period"));
    if (!atEnd()) {addError("Syntax error: unexpected tokens after program end (\"" + cur().display() + "\")");}
    return n;
}

ParseNode* Parser::parseProgramHeader() {
    auto* n = new ParseNode("<program-header>");
    n->addChild(expect("programsy"));
    n->addChild(expect("ident"));
    n->addChild(expect("semicolon"));
    return n;
}

ParseNode* Parser::parseDeclarationPart() {
    auto* n = new ParseNode("<declaration-part>");
    while (checkAny({"constsy","typesy","varsy","proceduresy","functionsy"})) {
        if (check("constsy")) n->addChild(parseConstDeclaration());
        else if (check("typesy")) n->addChild(parseTypeDeclaration());
        else if (check("varsy")) n->addChild(parseVarDeclaration());
        else n->addChild(parseSubprogramDeclaration());
    }
    return n;
}

// const
ParseNode* Parser::parseConstDeclaration() {
    auto* n = new ParseNode("<const-declaration>");
    n->addChild(expect("constsy"));
    do {
        n->addChild(expect("ident"));
        n->addChild(expect("eql"));
        n->addChild(parseConstant());
        n->addChild(expect("semicolon"));
    } while (check("ident"));
    return n;
}

ParseNode* Parser::parseConstant() {
    auto* n = new ParseNode("<constant>");
    if (checkAny({"charcon","string"})) {
        n->addChild(new ParseNode(cur().display())); ++pos;
    } else {
        if (checkAny({"plus","minus"})) {n->addChild(new ParseNode(cur().display())); ++pos;}
        if (checkAny({"ident","intcon","realcon"})) {n->addChild(new ParseNode(cur().display())); ++pos;}
        else {addError("Syntax error: expected constant, got \"" + cur().display() + "\"");}
    }
    return n;
}

// type
ParseNode* Parser::parseTypeDeclaration() {
    auto* n = new ParseNode("<type-declaration>");
    n->addChild(expect("typesy"));
    do {
        n->addChild(expect("ident"));
        n->addChild(expect("eql"));
        n->addChild(parseType());
        n->addChild(expect("semicolon"));
    } while (check("ident"));
    return n;
}

// var
ParseNode* Parser::parseVarDeclaration() {
    auto* n = new ParseNode("<var-declaration>");
    n->addChild(expect("varsy"));
    do {
        n->addChild(parseIdentifierList());
        n->addChild(expect("colon"));
        n->addChild(parseType());
        n->addChild(expect("semicolon"));
    } while (check("ident"));
    return n;
}

ParseNode* Parser::parseIdentifierList() {
    auto* n = new ParseNode("<identifier-list>");
    n->addChild(expect("ident"));
    while (check("comma")) {
        n->addChild(new ParseNode("comma")); ++pos;
        n->addChild(expect("ident"));
    }
    return n;
}

// type
ParseNode* Parser::parseType() {
    auto* n = new ParseNode("<type>");
    if (check("arraysy")) n->addChild(parseArrayType());
    else if (check("lparent")) n->addChild(parseEnumerated());
    else if (check("recordsy")) n->addChild(parseRecordType());
    else if (isRangeHere()) n->addChild(parseRange());
    else if (check("ident")) { n->addChild(new ParseNode(cur().display())); ++pos;}
    else {addError("Syntax error: expected type, got \"" + cur().display() + "\"");}
    return n;
}

ParseNode* Parser::parseArrayType() {
    auto* n = new ParseNode("<array-type>");
    n->addChild(expect("arraysy"));
    n->addChild(expect("lbrack"));
    if (isRangeHere()) n->addChild(parseRange());
    else n->addChild(expect("ident"));
    n->addChild(expect("rbrack"));
    n->addChild(expect("ofsy"));
    n->addChild(parseType());
    return n;
}

ParseNode* Parser::parseRange() {
    auto* n = new ParseNode("<range>");
    n->addChild(parseConstant());
    n->addChild(expect("period"));
    n->addChild(expect("period"));
    n->addChild(parseConstant());
    return n;
}

ParseNode* Parser::parseEnumerated() {
    auto* n = new ParseNode("<enumerated>");
    n->addChild(expect("lparent"));
    n->addChild(expect("ident"));
    while (check("comma")) {
        n->addChild(new ParseNode("comma")); ++pos;
        n->addChild(expect("ident"));
    }
    n->addChild(expect("rparent"));
    return n;
}

ParseNode* Parser::parseRecordType() {
    auto* n = new ParseNode("<record-type>");
    n->addChild(expect("recordsy"));
    n->addChild(parseFieldList());
    n->addChild(expect("endsy"));
    return n;
}

ParseNode* Parser::parseFieldList() {
    auto* n = new ParseNode("<field-list>");
    n->addChild(parseFieldPart());
    while (check("semicolon") && peek().type == "ident") {
        n->addChild(new ParseNode("semicolon")); ++pos;
        n->addChild(parseFieldPart());
    }
    return n;
}

ParseNode* Parser::parseFieldPart() {
    auto* n = new ParseNode("<field-part>");
    n->addChild(parseIdentifierList());
    n->addChild(expect("colon"));
    n->addChild(parseType());
    return n;
}

// subprogram
ParseNode* Parser::parseSubprogramDeclaration() {
    auto* n = new ParseNode("<subprogram-declaration>");
    if (check("proceduresy")) n->addChild(parseProcedureDeclaration());
    else n->addChild(parseFunctionDeclaration());
    return n;
}

ParseNode* Parser::parseProcedureDeclaration() {
    auto* n = new ParseNode("<procedure-declaration>");
    n->addChild(expect("proceduresy"));
    n->addChild(expect("ident"));
    if (check("lparent")) n->addChild(parseFormalParameterList());
    n->addChild(expect("semicolon"));
    n->addChild(parseBlock());
    n->addChild(expect("semicolon"));
    return n;
}

ParseNode* Parser::parseFunctionDeclaration() {
    auto* n = new ParseNode("<function-declaration>");
    n->addChild(expect("functionsy"));
    n->addChild(expect("ident"));
    if (check("lparent")) n->addChild(parseFormalParameterList());
    n->addChild(expect("colon"));
    n->addChild(expect("ident"));
    n->addChild(expect("semicolon"));
    n->addChild(parseBlock());
    n->addChild(expect("semicolon"));
    return n;
}

ParseNode* Parser::parseBlock() {
    auto* n = new ParseNode("<block>");
    n->addChild(parseDeclarationPart());
    n->addChild(parseCompoundStatement());
    return n;
}

ParseNode* Parser::parseFormalParameterList() {
    auto* n = new ParseNode("<formal-parameter-list>");
    n->addChild(expect("lparent"));
    n->addChild(parseParameterGroup());
    while (check("semicolon")) {
        n->addChild(new ParseNode("semicolon")); ++pos;
        n->addChild(parseParameterGroup());
    }
    n->addChild(expect("rparent"));
    return n;
}

ParseNode* Parser::parseParameterGroup() {
    auto* n = new ParseNode("<parameter-group>");
    n->addChild(parseIdentifierList());
    n->addChild(expect("colon"));
    if (check("arraysy")) n->addChild(parseArrayType());
    else n->addChild(expect("ident"));
    return n;
}

// compound statement
ParseNode* Parser::parseCompoundStatement() {
    auto* n = new ParseNode("<compound-statement>");
    n->addChild(expect("beginsy"));
    n->addChild(parseStatementList("endsy"));
    n->addChild(expect("endsy"));
    return n;
}

ParseNode* Parser::parseStatementList(const std::string& terminator) {
    auto* n = new ParseNode("<statement-list>");
    n->addChild(parseStatement());
    while (check("semicolon")) {
        if (peek().type == terminator) {
            n->addChild(new ParseNode("semicolon")); ++pos;
            break;
        }
        n->addChild(new ParseNode("semicolon")); ++pos;
        n->addChild(parseStatement());
    }
    return n;
}

// statement
ParseNode* Parser::parseStatement() {
    auto* n = new ParseNode("<statement>");
    if (check("ident")) {
        if (peek().type == "becomes") n->addChild(parseAssignmentStatement());
        else n->addChild(parseProcFuncCall());
    } else if (check("ifsy")) n->addChild(parseIfStatement());
    else if (check("casesy")) n->addChild(parseCaseStatement());
    else if (check("whilesy")) n->addChild(parseWhileStatement());
    else if (check("repeatsy")) n->addChild(parseRepeatStatement());
    else if (check("forsy")) n->addChild(parseForStatement());
    else if (check("beginsy")) n->addChild(parseCompoundStatement());
    return n;
}

ParseNode* Parser::parseAssignmentStatement() {
    auto* n = new ParseNode("<assignment-statement>");
    n->addChild(expect("ident"));
    n->addChild(expect("becomes"));
    n->addChild(parseExpression());
    return n;
}

ParseNode* Parser::parseIfStatement() {
    auto* n = new ParseNode("<if-statement>");
    n->addChild(expect("ifsy"));
    n->addChild(parseExpression());
    n->addChild(expect("thensy"));
    n->addChild(parseStatement());
    if (check("elsesy")) {
        n->addChild(new ParseNode("elsesy")); ++pos;
        n->addChild(parseStatement());
    }
    return n;
}

ParseNode* Parser::parseCaseStatement() {
    auto* n = new ParseNode("<case-statement>");
    n->addChild(expect("casesy"));
    n->addChild(parseExpression());
    n->addChild(expect("ofsy"));
    n->addChild(parseCaseBlock());
    n->addChild(expect("endsy"));
    return n;
}

ParseNode* Parser::parseCaseBlock() {
    auto* n = new ParseNode("<case-block>");
    n->addChild(parseConstant());
    while (check("comma")) {
        n->addChild(new ParseNode("comma")); ++pos;
        n->addChild(parseConstant());
    }
    n->addChild(expect("colon"));
    n->addChild(parseStatement());
    while (check("semicolon") && !peek().type.empty()) {
        const std::string& after = peek().type;
        if (after == "endsy") {
            n->addChild(new ParseNode("semicolon")); ++pos;
            break;
        }
        if (isConstantStart(1)) {
            n->addChild(new ParseNode("semicolon")); ++pos;
            n->addChild(parseCaseBlock());
        } else {
            n->addChild(new ParseNode("semicolon")); ++pos;
            break;
        }
    }
    return n;
}

ParseNode* Parser::parseWhileStatement() {
    auto* n = new ParseNode("<while-statement>");
    n->addChild(expect("whilesy"));
    n->addChild(parseExpression());
    n->addChild(expect("dosy"));
    n->addChild(parseStatement());
    return n;
}

ParseNode* Parser::parseRepeatStatement() {
    auto* n = new ParseNode("<repeat-statement>");
    n->addChild(expect("repeatsy"));
    n->addChild(parseStatementList("untilsy"));
    n->addChild(expect("untilsy"));
    n->addChild(parseExpression());
    return n;
}

ParseNode* Parser::parseForStatement() {
    auto* n = new ParseNode("<for-statement>");
    n->addChild(expect("forsy"));
    n->addChild(expect("ident"));
    n->addChild(expect("becomes"));
    n->addChild(parseExpression());
    if (check("tosy")) { n->addChild(new ParseNode("tosy"));     ++pos;}
    else if (check("downtosy")) { n->addChild(new ParseNode("downtosy")); ++pos;}
    else addError("Syntax error: expected \"tosy\" or \"downtosy\", got \"" + cur().display() + "\"");
    n->addChild(parseExpression());
    n->addChild(expect("dosy"));
    n->addChild(parseStatement());
    return n;
}

ParseNode* Parser::parseProcFuncCall() {
    auto* n = new ParseNode("<procedure/function-call>");
    n->addChild(expect("ident"));
    if (check("lparent")) {
        n->addChild(new ParseNode("lparent")); ++pos;
        if (!check("rparent")) n->addChild(parseParameterList());
        n->addChild(expect("rparent"));
    }
    return n;
}

ParseNode* Parser::parseParameterList() {
    auto* n = new ParseNode("<parameter-list>");
    n->addChild(parseExpression());
    while (check("comma")) {
        n->addChild(new ParseNode("comma")); ++pos;
        n->addChild(parseExpression());
    }
    return n;
}

// expressions
ParseNode* Parser::parseExpression() {
    auto* n = new ParseNode("<expression>");
    n->addChild(parseSimpleExpression());
    if (isRelOp()) {
        n->addChild(parseRelationalOperator());
        n->addChild(parseSimpleExpression());
    }
    return n;
}

ParseNode* Parser::parseSimpleExpression() {
    auto* n = new ParseNode("<simple-expression>");
    if (checkAny({"plus","minus"})) { n->addChild(new ParseNode(cur().display())); ++pos; }
    n->addChild(parseTerm());
    while (isAddOp()) {
        n->addChild(parseAdditiveOperator());
        n->addChild(parseTerm());
    }
    return n;
}

ParseNode* Parser::parseTerm() {
    auto* n = new ParseNode("<term>");
    n->addChild(parseFactor());
    while (isMulOp()) {
        n->addChild(parseMultiplicativeOperator());
        n->addChild(parseFactor());
    }
    return n;
}

ParseNode* Parser::parseFactor() {
    auto* n = new ParseNode("<factor>");
    if (check("ident")) {
        if (peek().type == "lparent") n->addChild(parseProcFuncCall());
        else { n->addChild(new ParseNode(cur().display())); ++pos; }
    } else if (checkAny({"intcon","realcon","charcon","string"})) {
        n->addChild(new ParseNode(cur().display())); ++pos;
    } else if (check("lparent")) {
        n->addChild(new ParseNode("lparent")); ++pos;
        n->addChild(parseExpression());
        n->addChild(expect("rparent"));
    } else if (check("notsy")) {
        n->addChild(new ParseNode("notsy")); ++pos;
        n->addChild(parseFactor());
    } else {
        addError("Syntax error: expected factor, got \"" + cur().display() + "\"");
        if (!atEnd()) ++pos;
    }
    return n;
}

ParseNode* Parser::parseRelationalOperator() {
    auto* n = new ParseNode("<relational-operator>");
    n->addChild(new ParseNode(cur().display())); ++pos;
    return n;
}
ParseNode* Parser::parseAdditiveOperator() {
    auto* n = new ParseNode("<additive-operator>");
    n->addChild(new ParseNode(cur().display())); ++pos;
    return n;
}
ParseNode* Parser::parseMultiplicativeOperator() {
    auto* n = new ParseNode("<multiplicative-operator>");
    n->addChild(new ParseNode(cur().display())); ++pos;
    return n;
}

// helper
bool Parser::isConstantStart(int off) const {
    const std::string& t = tokens[std::min(pos + off, (int)tokens.size()-1)].type;
    return t == "charcon" || t == "string" || t == "plus" || t == "minus" || t == "ident"   || t == "intcon" || t == "realcon";
}

bool Parser::isRangeHere() const {
    int i = pos;
    if (i < (int)tokens.size() && (tokens[i].type == "plus" || tokens[i].type == "minus")) ++i;
    if (i >= (int)tokens.size()) return false;
    const std::string& t = tokens[i].type;
    if (!(t=="ident"||t=="intcon"||t=="realcon"||t=="charcon"||t=="string")) return false;
    ++i;
    if (i >= (int)tokens.size() || tokens[i].type != "period") return false;
    ++i;
    if (i >= (int)tokens.size() || tokens[i].type != "period") return false;
    return true;
}

bool Parser::isStatementStart() const {
    return checkAny({"ident","ifsy","casesy","whilesy","repeatsy","forsy","beginsy"});
}
bool Parser::isRelOp() const {
    return checkAny({"eql","neq","gtr","geq","lss","leq"});
}
bool Parser::isAddOp() const {
    return checkAny({"plus","minus","orsy"});
}
bool Parser::isMulOp() const {
    return checkAny({"times","rdiv","idiv","imod","andsy"});
}

// run parser
void runParser(const std::vector<Token>& tokens, std::ostream& out) {
    Parser p(tokens);
    ParseNode* tree = p.parse();
    printTree(tree, out);
    if (p.hasErrors()) {
        out << "\n--- Syntax Errors ---\n";
        for (auto& e : p.getErrors()) out << e << "\n";
    }
    delete tree;
}