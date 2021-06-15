#ifndef _ASTNODE_
#define _ASTNODE_

#include <vector>
#include <string>

template <typename T>
using ptr = std::shared_ptr<T>;

template <typename T>
using ptr_vector = std::vector<ptr<T>>;

enum class BinOp
{
    PLUS = 0,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO
};

enum class UnaryOp
{
    PLUS = 0,
    MINUS
};

enum class RelOp
{
    EQUAL = 0,
    NON_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL
};

enum class BType
{
    INT = 0,
};

struct ast_node;
struct comp_root;
struct compunit_node;
struct global_def_node;
struct func_def_node;
struct expr_node;
struct cond_node;
struct binop_expr_node;
struct unaryop_expr_node;
struct lval_node;
struct number_node;
struct stmt_node;
struct var_def_stmt_node;
struct assign_stmt_node;
struct func_call_stmt_node;
struct block_node;
struct if_stmt_node;
struct while_stmt_node;
struct empty_stmt_node;

struct AstNode_Visitor;

struct ast_node
{
    int line;
    int pos;
    virtual void accept(AstNode_Visitor &visitor) = 0;
};

struct comp_root : virtual ast_node
{
    ptr_vector<compunit_node> comp_units;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct compunit_node : virtual ast_node
{
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct global_def_node : compunit_node
{
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct func_def_node : compunit_node
{
    std::string name;
    ptr<block_node> body;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct expr_node : virtual ast_node
{
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct cond_node : expr_node
{
    RelOp op;
    ptr<expr_node> lhs, rhs;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct binop_expr_node : expr_node
{
    BinOp op;
    ptr<expr_node> lhs, rhs;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct unaryop_expr_node : expr_node
{
    UnaryOp op;
    ptr<expr_node> rhs;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct lval_node : expr_node
{
    std::string name;
    ptr<expr_node> array_index;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct number_node : expr_node
{
    BType btype;
    int number;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct stmt_node : virtual ast_node
{
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct var_def_stmt_node : stmt_node, global_def_node
{
    bool is_const;
    BType btype;
    std::string name;
    bool is_obc;
    ptr<expr_node> array_length;
    ptr_vector<expr_node> initializers;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct assign_stmt_node : stmt_node
{
    ptr<lval_node> target;
    ptr<expr_node> value;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct func_call_stmt_node : stmt_node
{
    std::string name;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct block_node : stmt_node
{
    ptr_vector<stmt_node> body;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct if_stmt_node : stmt_node
{
    ptr<cond_node> cond;
    ptr<stmt_node> if_body;
    ptr<stmt_node> else_body;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct while_stmt_node : stmt_node
{
    ptr<cond_node> cond;
    ptr<stmt_node> body;
    virtual void accept(AstNode_Visitor &visitor) override;
};

struct empty_stmt_node : stmt_node
{
    virtual void accept(AstNode_Visitor &visitor) override;
};

#endif