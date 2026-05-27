#ifndef ICGEN_HPP
#define ICGEN_HPP

#include "semantic/header/ast.hpp"
#include "semantic/header/symtab.hpp"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

struct ICInstr {
    std::string op;
    int level = 0;
    long long operand = 0;
    std::string str_operand;
    bool has_str = false;
    bool has_operand = true;
};

class IntermediateCodeGen {
public:
    std::vector<ICInstr> code;
    std::vector<std::string> errors;

    explicit IntermediateCodeGen(SymbolTable* st) : symtab(st) {}

    void generate(ProgramNode* program);
    void print(std::ostream& out) const;

    bool hasErrors() const { return !errors.empty(); }

private:
    SymbolTable* symtab;
    std::unordered_map<int, int> subprogStartLine;
    int currentLevel = 0;

    int emit(const std::string& op, int level, long long operand);
    int emitStr(const std::string& op, int level, const std::string& s);
    int emitNoOperand(const std::string& op);
    void patchOperand(int idx, long long operand);
    int nextLine() const { return static_cast<int>(code.size()); }

    void genSubprogram(SubprogramDeclNode* sub);
    void genDeclarations(const std::vector<DeclNode*>& decls);
    void genStmt(StmtNode* stmt);
    void genCompoundStmt(CompoundStmtNode* node);
    void genAssign(AssignStmtNode* node);
    void genIf(IfStmtNode* node);
    void genWhile(WhileStmtNode* node);
    void genFor(ForStmtNode* node);
    void genRepeat(RepeatStmtNode* node);
    void genCase(CaseStmtNode* node);
    void genProcCall(ProcCallStmtNode* node);
    void genWriteCall(ProcCallStmtNode* node, bool newline);

    void genExpr(ExprNode* expr);
    void genBinOp(BinOpNode* node);
    void genUnaryOp(UnaryOpNode* node);
    void genFuncCallExpr(FuncCallNode* node);
    void genVarLoad(VarAccessNode* node);
    void genVarStore(VarAccessNode* node);
    bool isSimpleVar(VarAccessNode* node) const;
    void pushVarAddress(VarAccessNode* node);

    void addError(const std::string& msg) { errors.push_back(msg); }
    int frameSize(int btab_idx) const;
};

#endif
