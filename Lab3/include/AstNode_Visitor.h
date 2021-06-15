#ifndef _ASTNODEVISITOR_
#define _ASTNODEVISITOR_
#include "AstNode.h"

class AstNode_Visitor
{
public:
    virtual void visit(comp_root &node) = 0;
    virtual void visit(compunit_node &node) = 0;
    virtual void visit(global_def_node &node) = 0;
    virtual void visit(func_def_node &node) = 0;
    virtual void visit(expr_node &node) = 0;
    virtual void visit(cond_node &node) = 0;
    virtual void visit(binop_expr_node &node) = 0;
    virtual void visit(unaryop_expr_node &node) = 0;
    virtual void visit(lval_node &node) = 0;
    virtual void visit(number_node &node) = 0;
    virtual void visit(stmt_node &node) = 0;
    virtual void visit(var_def_stmt_node &node) = 0;
    virtual void visit(assign_stmt_node &node) = 0;
    virtual void visit(func_call_stmt_node &node) = 0;
    virtual void visit(block_node &node) = 0;
    virtual void visit(if_stmt_node &node) = 0;
    virtual void visit(while_stmt_node &node) = 0;
    virtual void visit(empty_stmt_node &node) = 0;
};

void comp_root::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void compunit_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void global_def_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void func_def_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void expr_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void cond_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void binop_expr_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void unaryop_expr_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void lval_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void number_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void var_def_stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void assign_stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void func_call_stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void block_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void if_stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void while_stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }
void empty_stmt_node::accept(AstNode_Visitor &visitor) { visitor.visit(*this); }

#endif