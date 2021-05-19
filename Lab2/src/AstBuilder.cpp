#include "AstBuilder.h"

namespace antlrcpp
{
  antlrcpp::Any AstBuilder::visitCompUnit(SafeCParser::CompUnitContext *ctx)
  {
    auto result = new comp_root;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto compile_units = ctx->children;
    for (auto i = 0; i < compile_units.size(); i++)
    {
      if (auto decl = dynamic_cast<SafeCParser::DeclContext *>(compile_units[i]))
      {

      }
      else if (auto func_def = dynamic_cast<SafeCParser::FuncDefContext *>(compile_units[i]))
      {
        auto comp_unit = visit(func_def).as<func_def_node *>();
        result->comp_units.push_back(ptr<compunit_node>(comp_unit));
      }
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitDecl(SafeCParser::DeclContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitFuncDef(SafeCParser::FuncDefContext *ctx)
  {
    auto result = new func_def_node;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->name = ctx->Identifier()->getSymbol()->getText();
    result->body.reset(visit(ctx->block()).as<block_node *>());
    return result;
  }

  antlrcpp::Any AstBuilder::visitConstDecl(SafeCParser::ConstDeclContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitBType(SafeCParser::BTypeContext *ctx) {}

  antlrcpp::Any AstBuilder::visitObcArray(SafeCParser::ObcArrayContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitUnobcArray(SafeCParser::UnobcArrayContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitArray(SafeCParser::ArrayContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitConstDef(SafeCParser::ConstDefContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitVarDecl(SafeCParser::VarDeclContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitVarDef(SafeCParser::VarDefContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitBlock(SafeCParser::BlockContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitBlockItem(SafeCParser::BlockItemContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitLval(SafeCParser::LvalContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitStmt(SafeCParser::StmtContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitCond(SafeCParser::CondContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitNumber(SafeCParser::NumberContext *ctx)
  {
    
  }

  antlrcpp::Any AstBuilder::visitExp(SafeCParser::ExpContext *ctx)
  {
    auto exps = ctx->exp();
    if (exps.size() == 2)
    {
      // exp op exp
      auto result = new binop_expr_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      if (ctx->Plus())
      {
        result->op = BinOp::PLUS;
      }
      else if (ctx->Minus())
      {
        result->op = BinOp::MINUS;
      }
      else if (ctx->Multiply())
      {
        result->op = BinOp::MULTIPLY;
      }
      else if (ctx->Divide())
      {
        result->op = BinOp::DIVIDE;
      }
      else if (ctx->Modulo())
      {
        result->op = BinOp::MODULO;
      }
      result->lhs.reset(visit(ctx->exp()[0]).as<expr_node *>());
      result->rhs.reset(visit(ctx->exp()[1]).as<expr_node *>());
      return static_cast<expr_node *>(result);
    }
    else if (exps.size() == 1)
    {
      // unaryop exp
      
    }
    else if (ctx->lval())
    { // lval
      
    }
    else if (ctx->number())
    { // number
      
    }
    else
    { // LeftParen exp RightParen
      
    }
  }

  ptr<ast_node> AstBuilder::operator()(antlr4::tree::ParseTree *ctx)
  {
    auto result = visit(ctx);
    if (result.is<ast_node *>())
      return ptr<ast_node>(result.as<ast_node *>());
    if (result.is<comp_root *>())
      return ptr<ast_node>(result.as<comp_root *>());
    if (result.is<compunit_node *>())
      return ptr<ast_node>(result.as<compunit_node *>());
    if (result.is<func_def_node *>())
      return ptr<ast_node>(result.as<func_def_node *>());
    if (result.is<expr_node *>())
      return ptr<ast_node>(result.as<expr_node *>());
    if (result.is<cond_node *>())
      return ptr<ast_node>(result.as<cond_node *>());
    if (result.is<binop_expr_node *>())
      return ptr<ast_node>(result.as<binop_expr_node *>());
    if (result.is<unaryop_expr_node *>())
      return ptr<ast_node>(result.as<unaryop_expr_node *>());
    if (result.is<lval_node *>())
      return ptr<ast_node>(result.as<lval_node *>());
    if (result.is<number_node *>())
      return ptr<ast_node>(result.as<number_node *>());
    if (result.is<stmt_node *>())
      return ptr<ast_node>(result.as<stmt_node *>());
    if (result.is<var_def_stmt_node *>())
      return ptr<ast_node>(result.as<var_def_stmt_node *>());
    if (result.is<assign_stmt_node *>())
      return ptr<ast_node>(result.as<assign_stmt_node *>());
    if (result.is<func_call_stmt_node *>())
      return ptr<ast_node>(result.as<func_call_stmt_node *>());
    if (result.is<block_node *>())
      return ptr<ast_node>(result.as<block_node *>());
    if (result.is<if_stmt_node *>())
      return ptr<ast_node>(result.as<if_stmt_node *>());
    if (result.is<while_stmt_node *>())
      return ptr<ast_node>(result.as<while_stmt_node *>());
    if (result.is<empty_stmt_node *>())
      return ptr<ast_node>(result.as<empty_stmt_node *>());
    return ptr<ast_node>(result.as<ast_node*>());
  }
}