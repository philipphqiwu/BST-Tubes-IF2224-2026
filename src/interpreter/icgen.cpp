#include "header/icgen.hpp"

#include <algorithm>

namespace {

int opNumber(const std::string& op) {
    if (op == "neg")   return 1;
    if (op == "plus")  return 2;
    if (op == "minus") return 3;
    if (op == "times") return 4;
    if (op == "idiv" || op == "rdiv") return 5;
    if (op == "imod")  return 6;
    if (op == "eql")   return 7;
    if (op == "neq")   return 8;
    if (op == "lss")   return 9;
    if (op == "geq")   return 10;
    if (op == "gtr")   return 11;
    if (op == "leq")   return 12;
    if (op == "andsy") return 15;
    if (op == "orsy")  return 16;
    if (op == "notsy") return 17;
    return 0;
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

}

int IntermediateCodeGen::emit(const std::string& op, int level, long long operand) {
    ICInstr ins;
    ins.op = op;
    ins.level = level;
    ins.operand = operand;
    ins.has_operand = true;
    code.push_back(ins);
    return static_cast<int>(code.size()) - 1;
}

int IntermediateCodeGen::emitStr(const std::string& op, int level, const std::string& s) {
    ICInstr ins;
    ins.op = op;
    ins.level = level;
    ins.has_str = true;
    ins.str_operand = s;
    ins.has_operand = true;
    code.push_back(ins);
    return static_cast<int>(code.size()) - 1;
}

int IntermediateCodeGen::emitNoOperand(const std::string& op) {
    ICInstr ins;
    ins.op = op;
    ins.has_operand = false;
    code.push_back(ins);
    return static_cast<int>(code.size()) - 1;
}

void IntermediateCodeGen::patchOperand(int idx, long long operand) {
    if (idx >= 0 && idx < (int)code.size()) {
        code[idx].operand = operand;
    }
}

int IntermediateCodeGen::frameSize(int btab_idx) const {
    if (btab_idx < 0 || btab_idx >= (int)symtab->btab.size()) return 3;
    return symtab->btab[btab_idx].vsze;
}

void IntermediateCodeGen::generate(ProgramNode* program) {
    if (!program || !program->block) return;

    std::vector<SubprogramDeclNode*> subprograms;
    for (auto d : program->block->declarations) {
        if (auto sp = dynamic_cast<SubprogramDeclNode*>(d)) {
            subprograms.push_back(sp);
        }
    }

    int leadingJmpIdx = -1;
    if (!subprograms.empty()) {
        leadingJmpIdx = emit("JMP", 0, 0);
    }

    currentLevel = 1;
    for (auto sp : subprograms) {
        genSubprogram(sp);
    }

    currentLevel = 0;
    int mainStart = nextLine();
    if (leadingJmpIdx != -1) {
        patchOperand(leadingJmpIdx, mainStart);
    }

    int globalFrameSize = frameSize(0);
    emit("INT", 0, globalFrameSize);

    if (program->block->compound_stmt) {
        genCompoundStmt(program->block->compound_stmt);
    }

    emitNoOperand("RET");
}

void IntermediateCodeGen::genSubprogram(SubprogramDeclNode* sub) {
    if (!sub) return;
    if (sub->tab_index < 0) return;

    int startLine = nextLine();
    subprogStartLine[sub->tab_index] = startLine;

    int btab_idx = symtab->tab[sub->tab_index].ref;
    int frame = frameSize(btab_idx);

    emit("INT", 0, frame);

    int prevLevel = currentLevel;
    currentLevel = sub->lex_level + 1;

    if (sub->block && sub->block->compound_stmt) {
        genCompoundStmt(sub->block->compound_stmt);
    }

    currentLevel = prevLevel;
    emitNoOperand("RET");
}

void IntermediateCodeGen::genStmt(StmtNode* stmt) {
    if (!stmt) return;
    if (auto n = dynamic_cast<AssignStmtNode*>(stmt))        genAssign(n);
    else if (auto n = dynamic_cast<CompoundStmtNode*>(stmt)) genCompoundStmt(n);
    else if (auto n = dynamic_cast<IfStmtNode*>(stmt))       genIf(n);
    else if (auto n = dynamic_cast<WhileStmtNode*>(stmt))    genWhile(n);
    else if (auto n = dynamic_cast<ForStmtNode*>(stmt))      genFor(n);
    else if (auto n = dynamic_cast<RepeatStmtNode*>(stmt))   genRepeat(n);
    else if (auto n = dynamic_cast<CaseStmtNode*>(stmt))     genCase(n);
    else if (auto n = dynamic_cast<ProcCallStmtNode*>(stmt)) genProcCall(n);
    else if (dynamic_cast<EmptyStmtNode*>(stmt)) {
    }
}

void IntermediateCodeGen::genCompoundStmt(CompoundStmtNode* node) {
    if (!node) return;
    for (auto s : node->statements) genStmt(s);
}

void IntermediateCodeGen::genAssign(AssignStmtNode* node) {
    if (!node) return;

    if (node->target && node->target->components.empty()) {
        int idx = symtab->lookup(node->target->name);
        if (idx >= 0 && symtab->tab[idx].obj == ObjClass::FUNCTION) {
            genExpr(node->value);
            emit("STO", 0, 0);
            return;
        }
    }

    genExpr(node->value);
    genVarStore(node->target);
}

void IntermediateCodeGen::genIf(IfStmtNode* node) {
    if (!node) return;
    genExpr(node->condition);
    int jpcIdx = emit("JPC", 0, 0);

    genStmt(node->then_stmt);

    if (node->else_stmt) {
        int jmpIdx = emit("JMP", 0, 0);
        int elseStart = nextLine();
        patchOperand(jpcIdx, elseStart);
        genStmt(node->else_stmt);
        int endIf = nextLine();
        patchOperand(jmpIdx, endIf);
    } else {
        int endIf = nextLine();
        patchOperand(jpcIdx, endIf);
    }
}

void IntermediateCodeGen::genWhile(WhileStmtNode* node) {
    if (!node) return;
    int loopStart = nextLine();
    genExpr(node->condition);
    int jpcIdx = emit("JPC", 0, 0);
    genStmt(node->body);
    emit("JMP", 0, loopStart);
    int endLoop = nextLine();
    patchOperand(jpcIdx, endLoop);
}

void IntermediateCodeGen::genFor(ForStmtNode* node) {
    if (!node || node->tab_index < 0) return;

    int counter_lev = symtab->tab[node->tab_index].lev;
    int counter_adr = symtab->tab[node->tab_index].adr;
    int lvl = currentLevel - counter_lev;

    genExpr(node->init_val);
    emit("STO", lvl, counter_adr);

    int loopStart = nextLine();
    emit("LOD", lvl, counter_adr);
    genExpr(node->final_val);

    bool isDownto = (node->direction == "downto" || node->direction == "downtosy");
    int cmpOp = isDownto ? 10 : 12;
    emit("OPR", 0, cmpOp);
    int jpcIdx = emit("JPC", 0, 0);

    genStmt(node->body);

    emit("LOD", lvl, counter_adr);
    emit("LIT", 0, 1);
    emit("OPR", 0, isDownto ? 3 : 2);
    emit("STO", lvl, counter_adr);

    emit("JMP", 0, loopStart);
    int endLoop = nextLine();
    patchOperand(jpcIdx, endLoop);
}

void IntermediateCodeGen::genRepeat(RepeatStmtNode* node) {
    if (!node) return;
    int loopStart = nextLine();
    for (auto s : node->body) genStmt(s);
    genExpr(node->condition);
    emit("JPC", 0, loopStart);
}

void IntermediateCodeGen::genCase(CaseStmtNode* node) {
    if (!node) return;
    std::vector<int> endJumps;

    for (auto block : node->blocks) {
        if (!block) continue;
        std::vector<int> matchJumps;
        std::vector<int> failJumps;

        for (size_t ci = 0; ci < block->constants.size(); ++ci) {
            genExpr(node->expression);
            genExpr(block->constants[ci]);
            emit("OPR", 0, 7);
            if (ci + 1 < block->constants.size()) {
                int jpc = emit("JPC", 0, 0);
                failJumps.push_back(jpc);
                int jmpMatch = emit("JMP", 0, 0);
                matchJumps.push_back(jmpMatch);
                int next = nextLine();
                patchOperand(jpc, next);
            } else {
                int jpc = emit("JPC", 0, 0);
                failJumps.push_back(jpc);
            }
        }

        int bodyStart = nextLine();
        for (int j : matchJumps) patchOperand(j, bodyStart);
        genStmt(block->statement);
        int endJmp = emit("JMP", 0, 0);
        endJumps.push_back(endJmp);

        int nextCase = nextLine();
        for (int j : failJumps) patchOperand(j, nextCase);
    }

    int caseEnd = nextLine();
    for (int j : endJumps) patchOperand(j, caseEnd);
}

void IntermediateCodeGen::genProcCall(ProcCallStmtNode* node) {
    if (!node) return;
    std::string lname = toLower(node->name);
    if (lname == "writeln") { genWriteCall(node, true);  return; }
    if (lname == "write")   { genWriteCall(node, false); return; }
    if (lname == "readln") {
        return;
    }

    if (node->tab_index < 0) return;
    int sub_lev = symtab->tab[node->tab_index].lev;
    int lvl = currentLevel - sub_lev;

    for (auto arg : node->args) genExpr(arg);

    auto it = subprogStartLine.find(node->tab_index);
    int startLine = (it != subprogStartLine.end()) ? it->second : 0;
    emit("CAL", lvl, startLine);
}

void IntermediateCodeGen::genWriteCall(ProcCallStmtNode* node, bool newline) {
    if (!node || node->args.empty()) {
        if (newline) emit("OPR", 0, 14);
        return;
    }
    for (size_t i = 0; i < node->args.size(); ++i) {
        ExprNode* arg = node->args[i];
        bool isLast = (i + 1 == node->args.size());

        if (auto lit = dynamic_cast<LiteralNode*>(arg)) {
            if (lit->lit_type == "string") {
                emitStr("LIT", 0, lit->value);
            } else if (lit->lit_type == "charcon") {
                emitStr("LIT", 0, "'" + lit->value + "'");
            } else {
                genExpr(lit);
            }
        } else {
            genExpr(arg);
        }

        int op = (isLast && newline) ? 14 : 13;
        emit("OPR", 0, op);
    }
}

void IntermediateCodeGen::genExpr(ExprNode* expr) {
    if (!expr) return;
    if (auto n = dynamic_cast<LiteralNode*>(expr)) {
        if (n->lit_type == "intcon") {
            try { emit("LIT", 0, std::stoll(n->value)); } catch (...) { emit("LIT", 0, 0); }
        } else if (n->lit_type == "realcon") {
            try { emit("LIT", 0, (long long)std::stod(n->value)); } catch (...) { emit("LIT", 0, 0); }
        } else if (n->lit_type == "charcon") {
            char c = n->value.empty() ? 0 : n->value[0];
            emit("LIT", 0, (long long)(unsigned char)c);
        } else if (n->lit_type == "string") {
            emitStr("LIT", 0, n->value);
        } else {
            emit("LIT", 0, 0);
        }
    } else if (auto n = dynamic_cast<VarAccessNode*>(expr)) {
        genVarLoad(n);
    } else if (auto n = dynamic_cast<BinOpNode*>(expr)) {
        genBinOp(n);
    } else if (auto n = dynamic_cast<UnaryOpNode*>(expr)) {
        genUnaryOp(n);
    } else if (auto n = dynamic_cast<FuncCallNode*>(expr)) {
        genFuncCallExpr(n);
    }
}

void IntermediateCodeGen::genBinOp(BinOpNode* node) {
    if (!node) return;
    genExpr(node->left);
    genExpr(node->right);
    int op = opNumber(node->op);
    emit("OPR", 0, op);
}

void IntermediateCodeGen::genUnaryOp(UnaryOpNode* node) {
    if (!node) return;
    genExpr(node->right);
    if (node->op == "minus") {
        emit("OPR", 0, 1);
    } else if (node->op == "notsy") {
        emit("OPR", 0, 17);
    }
}

void IntermediateCodeGen::genFuncCallExpr(FuncCallNode* node) {
    if (!node || node->tab_index < 0) return;
    int sub_lev = symtab->tab[node->tab_index].lev;
    int lvl = currentLevel - sub_lev;
    for (auto arg : node->args) genExpr(arg);
    auto it = subprogStartLine.find(node->tab_index);
    int startLine = (it != subprogStartLine.end()) ? it->second : 0;
    emit("CAL", lvl, startLine);
}

bool IntermediateCodeGen::isSimpleVar(VarAccessNode* node) const {
    return node && node->components.empty();
}

void IntermediateCodeGen::genVarLoad(VarAccessNode* node) {
    if (!node) return;

    if (node->tab_index < 0) {
        emit("LIT", 0, 0);
        return;
    }
    const TabEntry& entry = symtab->tab[node->tab_index];

    if (entry.obj == ObjClass::CONSTANT) {
        emit("LIT", 0, entry.adr);
        return;
    }

    int lvl = currentLevel - entry.lev;

    if (isSimpleVar(node)) {
        emit("LOD", lvl, entry.adr);
        return;
    }

    pushVarAddress(node);
    emit("OPR", 0, 18);
}

void IntermediateCodeGen::genVarStore(VarAccessNode* node) {
    if (!node) return;
    if (node->tab_index < 0) {
        emit("STO", 0, 0);
        return;
    }
    const TabEntry& entry = symtab->tab[node->tab_index];
    int lvl = currentLevel - entry.lev;

    if (isSimpleVar(node)) {
        emit("STO", lvl, entry.adr);
        return;
    }

    pushVarAddress(node);
    emit("OPR", 0, 19);
}

void IntermediateCodeGen::pushVarAddress(VarAccessNode* node) {
    if (!node || node->tab_index < 0) {
        emit("LIT", 0, 0);
        return;
    }
    const TabEntry& entry = symtab->tab[node->tab_index];
    emit("LIT", 0, entry.adr);

    int current_ref = entry.ref;

    for (auto& comp : node->components) {
        if (comp.is_array_index) {
            int low = 0, high = 0, elsz = 1;
            bool has_bounds = false;
            if (current_ref >= 0 && current_ref < (int)symtab->atab.size()) {
                low = symtab->atab[current_ref].low;
                high = symtab->atab[current_ref].high;
                elsz = symtab->atab[current_ref].elsz;
                has_bounds = (low != high);
                current_ref = symtab->atab[current_ref].eref;
            }
            for (auto idx : comp.indices) {
                genExpr(idx);
                if (has_bounds) {
                    emit("LIT", 0, low);
                    emit("LIT", 0, high);
                    emit("OPR", 0, 22);
                }
                if (low != 0) {
                    emit("LIT", 0, low);
                    emit("OPR", 0, 3);
                }
                if (elsz != 1) {
                    emit("LIT", 0, elsz);
                    emit("OPR", 0, 4);
                }
                emit("OPR", 0, 2);
            }
        } else if (!comp.field_name.empty()) {
            int field_idx = symtab->lookup(comp.field_name);
            int offset = 0;
            if (field_idx >= 0) {
                offset = symtab->tab[field_idx].adr;
            }
            emit("LIT", 0, offset);
            emit("OPR", 0, 2);
        }
    }
}

void IntermediateCodeGen::print(std::ostream& out) const {
    for (size_t i = 0; i < code.size(); ++i) {
        const ICInstr& ins = code[i];
        out << i << " " << ins.op;
        if (ins.has_operand) {
            out << " " << ins.level << " ";
            if (ins.has_str) out << ins.str_operand;
            else             out << ins.operand;
        }
        out << "\n";
    }
}
