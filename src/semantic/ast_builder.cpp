#include "header/ast_builder.hpp"

std::string ASTBuilder::extractValue(const std::string& label) {
    size_t start = label.find('(');
    size_t end = label.rfind(')');
    if (start != std::string::npos && end != std::string::npos && end > start) {
        return label.substr(start + 1, end - start - 1);
    }
    return "";
}

std::string ASTBuilder::extractType(const std::string& label) {
    size_t start = label.find('(');
    if (start != std::string::npos) {
        return label.substr(0, start);
    }
    return label;
}

ProgramNode* ASTBuilder::build(ParseNode* root) {
    if (!root) return nullptr;
    std::string progName = "unknown";
    BlockNode* block = new BlockNode();

    for (ParseNode* child : root->children) {
        if (child->label == "<program-header>") {
            for (ParseNode* hc : child->children) {
                if (extractType(hc->label) == "ident") {
                    progName = extractValue(hc->label);
                }
            }
        } else if (child->label == "<declaration-part>") {
            buildDeclarationPart(child, block->declarations);
        } else if (child->label == "<compound-statement>") {
            block->compound_stmt = buildCompoundStatement(child);
        }
    }
    return new ProgramNode(progName, block);
}

void ASTBuilder::buildDeclarationPart(ParseNode* node, std::vector<DeclNode*>& decls) {
    for (ParseNode* child : node->children) {
        if (child->label == "<const-declaration>") {
            std::string c_name;
            for (ParseNode* cc : child->children) {
                std::string t = extractType(cc->label);
                if (t == "ident") {
                    c_name = extractValue(cc->label);
                } else if (cc->label == "<constant>") {
                    decls.push_back(new ConstDeclNode(c_name, buildExpression(cc))); 
                }
            }
        } else if (child->label == "<type-declaration>") {
            std::string t_name;
            for (ParseNode* tc : child->children) {
                if (extractType(tc->label) == "ident") {
                    t_name = extractValue(tc->label);
                } else if (tc->label == "<type>") {
                    decls.push_back(new TypeDeclNode(t_name, buildType(tc)));
                }
            }
        } else if (child->label == "<var-declaration>") {
            std::vector<std::string> idents;
            TypeNode* typeNode = nullptr;
            for (ParseNode* vc : child->children) {
                if (vc->label == "<identifier-list>") {
                    for (ParseNode* ic : vc->children) {
                        if (extractType(ic->label) == "ident") {
                            idents.push_back(extractValue(ic->label));
                        }
                    }
                } else if (vc->label == "<type>") {
                    typeNode = buildType(vc);
                    decls.push_back(new VarDeclNode(idents, typeNode));
                    idents.clear();
                }
            }
        } else if (child->label == "<subprogram-declaration>") {
            ParseNode* sub = child->children[0];
            bool is_func = (sub->label == "<function-declaration>");
            std::string name;
            std::string ret_type;
            std::vector<ParamNode*> params;
            BlockNode* blk = nullptr;

            for (ParseNode* sc : sub->children) {
                if (extractType(sc->label) == "ident") {
                    if (name.empty()) name = extractValue(sc->label);
                    else ret_type = extractValue(sc->label);
                } else if (sc->label == "<formal-parameter-list>") {
                    for (ParseNode* pc : sc->children) {
                        if (pc->label == "<parameter-group>") {
                            std::vector<std::string> pnames;
                            TypeNode* ptype = nullptr;
                            for (ParseNode* pgc : pc->children) {
                                if (pgc->label == "<identifier-list>") {
                                    for (ParseNode* ic : pgc->children) {
                                        if (extractType(ic->label) == "ident") {
                                            pnames.push_back(extractValue(ic->label));
                                        }
                                    }
                                } else if (pgc->label == "<array-type>") {
                                    ptype = buildType(pgc); 
                                } else if (extractType(pgc->label) == "ident") {
                                    ptype = new NamedTypeNode(extractValue(pgc->label));
                                }
                            }
                            if (ptype) params.push_back(new ParamNode(pnames, ptype));
                        }
                    }
                } else if (sc->label == "<block>") {
                    blk = buildBlock(sc);
                }
            }
            decls.push_back(new SubprogramDeclNode(is_func, name, params, ret_type, blk));
        }
    }
}

BlockNode* ASTBuilder::buildBlock(ParseNode* node) {
    BlockNode* b = new BlockNode();
    for (ParseNode* c : node->children) {
        if (c->label == "<declaration-part>") buildDeclarationPart(c, b->declarations);
        else if (c->label == "<compound-statement>") b->compound_stmt = buildCompoundStatement(c);
    }
    return b;
}

TypeNode* ASTBuilder::buildType(ParseNode* node) {
    if (node->children.empty()) return new NamedTypeNode("unknown");
    std::string clabel = node->children[0]->label;
    
    if (extractType(clabel) == "ident") {
        return new NamedTypeNode(extractValue(clabel));
    } else if (clabel == "<array-type>") {
        TypeNode* idx = nullptr;
        TypeNode* el = nullptr;
        for (ParseNode* ac : node->children[0]->children) {
            if (ac->label == "<range>") idx = buildType(ac);
            else if (extractType(ac->label) == "ident") idx = new NamedTypeNode(extractValue(ac->label));
            else if (ac->label == "<type>") el = buildType(ac);
        }
        return new ArrayTypeNode(idx, el);
    } else if (clabel == "<range>") {
        ExprNode* low = buildExpression(node->children[0]->children[0]);
        ExprNode* high = buildExpression(node->children[0]->children[3]); 
        return new RangeNode(low, high);
    } else if (clabel == "<enumerated>") {
        std::vector<std::string> idents;
        for (ParseNode* ec : node->children[0]->children) {
            if (extractType(ec->label) == "ident") idents.push_back(extractValue(ec->label));
        }
        return new EnumTypeNode(idents);
    } else if (clabel == "<record-type>") {
        RecordTypeNode* rec = new RecordTypeNode();
        for (ParseNode* rc : node->children[0]->children) {
            if (rc->label == "<field-list>") {
                for (ParseNode* fc : rc->children) {
                    if (fc->label == "<field-part>") {
                        FieldNode* fn = new FieldNode();
                        for (ParseNode* fpc : fc->children) {
                            if (fpc->label == "<identifier-list>") {
                                for (ParseNode* ic : fpc->children) {
                                    if (extractType(ic->label) == "ident") fn->names.push_back(extractValue(ic->label));
                                }
                            } else if (fpc->label == "<type>") {
                                fn->type = buildType(fpc);
                            }
                        }
                        rec->fields.push_back(fn);
                    }
                }
            }
        }
        return rec;
    }
    return new NamedTypeNode("unknown");
}

CompoundStmtNode* ASTBuilder::buildCompoundStatement(ParseNode* node) {
    CompoundStmtNode* comp = new CompoundStmtNode();
    for (ParseNode* child : node->children) {
        if (child->label == "<statement-list>") {
            for (ParseNode* stmtNode : child->children) {
                if (stmtNode->label == "<statement>") {
                    comp->statements.push_back(buildStatement(stmtNode));
                }
            }
        }
    }
    return comp;
}

StmtNode* ASTBuilder::buildStatement(ParseNode* node) {
    if (node->children.empty()) return new EmptyStmtNode();
    ParseNode* child = node->children[0];

    if (child->label == "<assignment-statement>") {
        VarAccessNode* target = nullptr;
        ExprNode* value = nullptr;
        for (ParseNode* ac : child->children) {
            if (ac->label == "<variable>") target = buildVariable(ac);
            else if (ac->label == "<expression>") value = buildExpression(ac);
        }
        return new AssignStmtNode(target, value);
    } else if (child->label == "<if-statement>") {
        ExprNode* cond = nullptr;
        StmtNode* thn = nullptr;
        StmtNode* els = nullptr;
        int step = 0;
        for (ParseNode* ic : child->children) {
            if (ic->label == "<expression>") cond = buildExpression(ic);
            else if (ic->label == "<statement>") {
                if (step == 0) { thn = buildStatement(ic); step++; }
                else els = buildStatement(ic);
            }
        }
        return new IfStmtNode(cond, thn, els);
    } else if (child->label == "<case-statement>") {
        ExprNode* expr = nullptr;
        CaseStmtNode* cs = nullptr;
        for (ParseNode* cc : child->children) {
            if (cc->label == "<expression>") {
                expr = buildExpression(cc);
                cs = new CaseStmtNode(expr);
            } else if (cc->label == "<case-block>") {
                CaseBlockNode* cb = new CaseBlockNode();
                for (ParseNode* cbc : cc->children) {
                    if (cbc->label == "<constant>") cb->constants.push_back(buildExpression(cbc));
                    else if (cbc->label == "<statement>") cb->statement = buildStatement(cbc);
                }
                cs->blocks.push_back(cb);
            }
        }
        return cs;
    } else if (child->label == "<while-statement>") {
        ExprNode* cond = nullptr;
        StmtNode* body = nullptr;
        for (ParseNode* wc : child->children) {
            if (wc->label == "<expression>") cond = buildExpression(wc);
            else if (wc->label == "<statement>") body = buildStatement(wc);
        }
        return new WhileStmtNode(cond, body);
    } else if (child->label == "<repeat-statement>") {
        RepeatStmtNode* rep = new RepeatStmtNode();
        for (ParseNode* rc : child->children) {
            if (rc->label == "<statement-list>") {
                for (ParseNode* sc : rc->children) {
                    if (sc->label == "<statement>") rep->body.push_back(buildStatement(sc));
                }
            } else if (rc->label == "<expression>") {
                rep->condition = buildExpression(rc);
            }
        }
        return rep;
    } else if (child->label == "<for-statement>") {
        std::string name;
        ExprNode* init = nullptr;
        ExprNode* fin = nullptr;
        std::string dir;
        StmtNode* body = nullptr;
        for (ParseNode* fc : child->children) {
            std::string typ = extractType(fc->label);
            if (typ == "ident") name = extractValue(fc->label);
            else if (typ == "tosy") dir = "tosy";
            else if (typ == "downtosy") dir = "downtosy";
            else if (fc->label == "<expression>") {
                if (!init) init = buildExpression(fc);
                else fin = buildExpression(fc);
            } else if (fc->label == "<statement>") body = buildStatement(fc);
        }
        return new ForStmtNode(name, init, dir, fin, body);
    } else if (child->label == "<procedure/function-call>") {
        std::string name;
        std::vector<ExprNode*> args;
        for (ParseNode* pc : child->children) {
            if (extractType(pc->label) == "ident") name = extractValue(pc->label);
            else if (pc->label == "<parameter-list>") {
                for (ParseNode* ac : pc->children) {
                    if (ac->label == "<expression>") args.push_back(buildExpression(ac));
                }
            }
        }
        return new ProcCallStmtNode(name, args);
    } else if (child->label == "<compound-statement>") {
        return buildCompoundStatement(child);
    }
    return new EmptyStmtNode();
}

ExprNode* ASTBuilder::buildExpression(ParseNode* node) {
    if (node->children.empty()) return nullptr;
    if (node->label == "<constant>") {
        if (node->children.size() == 1) {
             std::string t = extractType(node->children[0]->label);
             if (t == "ident") {
                 return new VarAccessNode(extractValue(node->children[0]->label));
             }
             return new LiteralNode(extractValue(node->children[0]->label), t);
        } else if (node->children.size() == 2) {
             std::string op = extractType(node->children[0]->label);
             std::string t = extractType(node->children[1]->label);
             ExprNode* r = nullptr;
             if (t == "ident") r = new VarAccessNode(extractValue(node->children[1]->label));
             else r = new LiteralNode(extractValue(node->children[1]->label), t);
             return new UnaryOpNode(op, r);
        }
    }

    ExprNode* left = buildSimpleExpression(node->children[0]);
    if (node->children.size() > 1) {
        std::string op;
        for(ParseNode* rc : node->children[1]->children) {
            op = extractType(rc->label); 
            break;
        }
        ExprNode* right = buildSimpleExpression(node->children[2]);
        return new BinOpNode(op, left, right);
    }
    return left;
}

ExprNode* ASTBuilder::buildSimpleExpression(ParseNode* node) {
    if (node->children.empty()) return nullptr;
    int idx = 0;
    std::string unary_op;
    std::string firstType = extractType(node->children[0]->label);
    if (firstType == "plus" || firstType == "minus") {
        unary_op = firstType;
        idx++;
    }

    ExprNode* left = buildTerm(node->children[idx++]);
    if (!unary_op.empty()) left = new UnaryOpNode(unary_op, left);

    while (idx < node->children.size()) {
        std::string op;
        for(ParseNode* oc : node->children[idx]->children) {
            op = extractType(oc->label);
            break;
        }
        idx++;
        ExprNode* right = buildTerm(node->children[idx++]);
        left = new BinOpNode(op, left, right);
    }
    return left;
}

ExprNode* ASTBuilder::buildTerm(ParseNode* node) {
    if (node->children.empty()) return nullptr;
    ExprNode* left = buildFactor(node->children[0]);
    int idx = 1;
    while (idx < node->children.size()) {
        std::string op;
        for(ParseNode* oc : node->children[idx]->children) {
            op = extractType(oc->label);
            break;
        }
        idx++;
        ExprNode* right = buildFactor(node->children[idx++]);
        left = new BinOpNode(op, left, right);
    }
    return left;
}

ExprNode* ASTBuilder::buildFactor(ParseNode* node) {
    if (node->children.empty()) return nullptr;
    ParseNode* child = node->children[0];

    std::string typ = extractType(child->label);
    if (child->label == "<variable>") {
        return buildVariable(child);
    } else if (typ == "intcon" || typ == "realcon" || typ == "charcon" || typ == "string") {
        return new LiteralNode(extractValue(child->label), typ);
    } else if (child->label == "<procedure/function-call>") {
        std::string name;
        std::vector<ExprNode*> args;
        for (ParseNode* pc : child->children) {
            if (extractType(pc->label) == "ident") name = extractValue(pc->label);
            else if (pc->label == "<parameter-list>") {
                for (ParseNode* ac : pc->children) {
                    if (ac->label == "<expression>") args.push_back(buildExpression(ac));
                }
            }
        }
        return new FuncCallNode(name, args);
    } else if (typ == "lparent") {
        return buildExpression(node->children[1]);
    } else if (typ == "notsy") {
        return new UnaryOpNode("notsy", buildFactor(node->children[1]));
    }
    return new LiteralNode("unknown", "unknown");
}

VarAccessNode* ASTBuilder::buildVariable(ParseNode* node) {
    std::string name;
    for (ParseNode* c : node->children) {
        if (extractType(c->label) == "ident") {
            name = extractValue(c->label);
            break;
        }
    }
    VarAccessNode* var = new VarAccessNode(name);
    
    for (ParseNode* c : node->children) {
        if (c->label == "<component-variable>") {
            VarComponent comp;
            for (ParseNode* cc : c->children) {
                if (extractType(cc->label) == "lbrack") {
                    comp.is_array_index = true;
                } else if (extractType(cc->label) == "period") {
                    comp.is_array_index = false;
                } else if (cc->label == "<index-list>") {
                    for (ParseNode* ic : cc->children) {
                        std::string t = extractType(ic->label);
                        if (t == "intcon" || t == "charcon" || t == "ident") {
                            comp.indices.push_back(new LiteralNode(extractValue(ic->label), t));
                        }
                    }
                } else if (extractType(cc->label) == "ident") {
                    comp.field_name = extractValue(cc->label);
                }
            }
            var->components.push_back(comp);
        }
    }
    return var;
}