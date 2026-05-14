#include "header/ast.hpp"
#include "header/semantic.hpp"

void NamedTypeNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void RangeNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void ArrayTypeNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void EnumTypeNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void RecordTypeNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }

void LiteralNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void VarAccessNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void BinOpNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void UnaryOpNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void FuncCallNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }

void AssignStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void CompoundStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void IfStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void CaseBlockNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void CaseStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void WhileStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void RepeatStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void ForStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void ProcCallStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void EmptyStmtNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }

void ConstDeclNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void TypeDeclNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void VarDeclNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void ParamNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void SubprogramDeclNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }

void BlockNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }
void ProgramNode::accept(SemanticVisitor& visitor) { visitor.visit(this); }