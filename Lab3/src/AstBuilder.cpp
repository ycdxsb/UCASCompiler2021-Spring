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
        auto comp_units = (visit(decl).as<block_node *>())->body;
        for (auto j = 0; j < comp_units.size(); j++)
        {
          auto comp_unit = dynamic_pointer_cast<var_def_stmt_node>(comp_units[j]);
          result->comp_units.push_back(ptr<compunit_node>(comp_unit));
        }
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
    if (ctx->constDecl())
    {
      auto result = visit(ctx->constDecl());
      return result;
    }
    else
    {
      auto result = visit(ctx->varDecl());
      return result;
    }
    return ptr_vector<var_def_stmt_node>();
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
    auto result = new block_node;
    auto constdefs = ctx->constDef();
    for (auto i = 0; i < constdefs.size(); i++)
    {
      auto constdef = visit(constdefs[i]).as<var_def_stmt_node *>();
      if (ctx->bType()->Int())
      {
        constdef->btype = BType::INT;
      }
      result->body.push_back(ptr<stmt_node>(static_cast<stmt_node *>(constdef)));
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitBType(SafeCParser::BTypeContext *ctx) {}

  antlrcpp::Any AstBuilder::visitObcArray(SafeCParser::ObcArrayContext *ctx)
  {
    auto result = new var_def_stmt_node;
    result->name = ctx->Identifier()->getSymbol()->getText();
    result->is_obc = true;
    if (ctx->exp())
    {
      result->array_length.reset(visit(ctx->exp()).as<expr_node *>());
    }
    else
    {
      result->array_length = nullptr;
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitUnobcArray(SafeCParser::UnobcArrayContext *ctx)
  {
    auto result = new var_def_stmt_node;
    result->name = ctx->Identifier()->getSymbol()->getText();
    result->is_obc = false;
    if (ctx->exp())
    {
      result->array_length.reset(visit(ctx->exp()).as<expr_node *>());
    }
    else
    {
      result->array_length = nullptr;
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitArray(SafeCParser::ArrayContext *ctx)
  {
    if (ctx->obcArray())
    {
      auto result = visit(ctx->obcArray());
      return result;
    }
    else if (ctx->unobcArray())
    {
      auto result = visit(ctx->unobcArray());
      return result;
    }
  }

  antlrcpp::Any AstBuilder::visitConstDef(SafeCParser::ConstDefContext *ctx)
  {
    if (ctx->array())
    {
      auto result = visit(ctx->array()).as<var_def_stmt_node *>();
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->is_const = true;
      int i = 0;
      auto exps = ctx->exp();
      for (i = 0; i < exps.size(); i++)
      {
        result->initializers.push_back(ptr<expr_node>(visit(exps[i]).as<expr_node *>()));
      }
      if (result->initializers.size() > 0 && result->array_length == nullptr)
      {
        // int a[] = {1,2}
        auto lenght_node = new number_node;
        lenght_node->number = result->initializers.size();
        if (ctx->array()->unobcArray())
        {
          lenght_node->line = ctx->array()->unobcArray()->LeftBracket()->getSymbol()->getLine();
          lenght_node->pos = ctx->array()->unobcArray()->LeftBracket()->getSymbol()->getCharPositionInLine();
        }
        else if (ctx->array()->obcArray())
        {
          lenght_node->line = ctx->array()->obcArray()->LeftBracket()->getSymbol()->getLine();
          lenght_node->pos = ctx->array()->obcArray()->LeftBracket()->getSymbol()->getCharPositionInLine();
        }
        lenght_node->btype = BType::INT;
        result->array_length.reset(lenght_node);
      }
      return result;
    }
    else if (ctx->Identifier())
    {
      auto result = new var_def_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->name = ctx->Identifier()->getSymbol()->getText();
      result->is_const = true;
      auto exps = ctx->exp();
      result->is_obc = false;
      result->name = ctx->Identifier()->getSymbol()->getText();
      result->array_length = nullptr;
      result->initializers.push_back(ptr<expr_node>(visit(exps[0]).as<expr_node *>()));
      return result;
    }
  }

  antlrcpp::Any AstBuilder::visitVarDecl(SafeCParser::VarDeclContext *ctx)
  {
    auto result = new block_node;
    auto vardefs = ctx->varDef();
    for (auto i = 0; i < vardefs.size(); i++)
    {
      auto vardef = visit(vardefs[i]).as<var_def_stmt_node *>();
      if (ctx->bType()->Int())
      {
        vardef->btype = BType::INT;
      }
      result->body.push_back(ptr<stmt_node>(static_cast<stmt_node *>(vardef)));
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitVarDef(SafeCParser::VarDefContext *ctx)
  {
    // varDef: Identifier | array  | Identifier Assign exp | array Assign LeftBrace exp (Comma exp)* RightBrace;
    if (ctx->array())
    {
      auto result = visit(ctx->array()).as<var_def_stmt_node *>();
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->is_const = false;
      int i = 0;
      if (ctx->Assign())
      {
        auto exps = ctx->exp();
        for (i = 0; i < exps.size(); i++)
        {
          result->initializers.push_back(ptr<expr_node>(visit(exps[i]).as<expr_node *>()));
        }
      }
      if (result->initializers.size() > 0 && result->array_length == nullptr)
      {
        // int a[] = {1,2}
        auto lenght_node = new number_node;
        lenght_node->number = result->initializers.size();
        if (ctx->array()->unobcArray())
        {
          lenght_node->line = ctx->array()->unobcArray()->LeftBracket()->getSymbol()->getLine();
          lenght_node->pos = ctx->array()->unobcArray()->LeftBracket()->getSymbol()->getCharPositionInLine();
        }
        else if (ctx->array()->obcArray())
        {
          lenght_node->line = ctx->array()->obcArray()->LeftBracket()->getSymbol()->getLine();
          lenght_node->pos = ctx->array()->obcArray()->LeftBracket()->getSymbol()->getCharPositionInLine();
        }
        lenght_node->btype = BType::INT;
        result->array_length.reset(lenght_node);
      }
      return result;
    }
    else if (ctx->Identifier())
    {
      auto result = new var_def_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->name = ctx->Identifier()->getSymbol()->getText();
      result->is_const = false;
      auto exps = ctx->exp();
      result->is_obc = false;
      result->name = ctx->Identifier()->getSymbol()->getText();
      result->array_length = nullptr;
      if (ctx->Assign())
      {
        result->initializers.push_back(ptr<expr_node>(visit(exps[0]).as<expr_node *>()));
      }
      return result;
    }
  }

  antlrcpp::Any AstBuilder::visitBlock(SafeCParser::BlockContext *ctx)
  {
    auto result = new block_node;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto bodys = ctx->blockItem();
    for (auto i = 0; i < bodys.size(); i++)
    {
      if (bodys[i]->decl())
      {
        auto decl = visit(bodys[i]->decl()).as<block_node *>();
        for (auto j = 0; j < decl->body.size(); j++)
        {
          result->body.push_back(ptr<stmt_node>(decl->body[j]));
        }
      }
      else
      {
        auto stmt = visit(bodys[i]->stmt()).as<stmt_node *>();
        result->body.push_back(ptr<stmt_node>(stmt));
      }
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitBlockItem(SafeCParser::BlockItemContext *ctx)
  {
    if (ctx->decl())
    {
      auto result = visit(ctx->decl());
      return result;
    }
    else
    {
      auto result = visit(ctx->stmt());
      return result;
    }
  }

  antlrcpp::Any AstBuilder::visitLval(SafeCParser::LvalContext *ctx)
  {
    auto result = new lval_node;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->name = ctx->Identifier()->getSymbol()->getText();
    if (ctx->exp())
    {
      result->array_index.reset(visit(ctx->exp()).as<expr_node *>());
    }
    else
    {
      result->array_index = nullptr;
    }
    return result;
  }

  antlrcpp::Any AstBuilder::visitStmt(SafeCParser::StmtContext *ctx)
  {
    if (ctx->Assign())
    {
      // lval assign exp SemiColon
      auto result = new assign_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->target.reset(visit(ctx->lval()).as<lval_node *>());
      result->value.reset(visit(ctx->exp()).as<expr_node *>());
      return static_cast<stmt_node *>(result);
    }
    else if (ctx->block())
    {
      // block
      auto result = visit(ctx->block()).as<block_node *>();
      return static_cast<stmt_node *>(result);
    }
    else if (ctx->If())
    {
      // if else
      auto result = new if_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->cond.reset(visit(ctx->cond()).as<cond_node *>());
      auto stmts = ctx->stmt();
      result->if_body.reset(visit(stmts[0]).as<stmt_node *>());
      if (ctx->Else())
      {
        result->else_body.reset(visit(stmts[1]).as<stmt_node *>());
      }
      else
      {
        result->else_body = nullptr;
      }
      return static_cast<stmt_node *>(result);
    }
    else if (ctx->While())
    {
      // while
      auto result = new while_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->cond.reset(visit(ctx->cond()).as<cond_node *>());
      result->body.reset(visit(ctx->stmt()[0]).as<stmt_node *>());
      return static_cast<stmt_node *>(result);
    }
    else if (ctx->Identifier())
    {
      auto result = new func_call_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      result->name = ctx->Identifier()->getSymbol()->getText();
      return static_cast<stmt_node *>(result);
    }
    else if (ctx->SemiColon())
    {
      // empty
      auto result = new empty_stmt_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      return static_cast<stmt_node *>(result);
    }
  }

  antlrcpp::Any AstBuilder::visitCond(SafeCParser::CondContext *ctx)
  {
    auto result = new cond_node;
    auto exps = ctx->exp();
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    if (ctx->Equal())
    {
      result->op = RelOp::EQUAL;
    }
    else if (ctx->NonEqual())
    {
      result->op = RelOp::NON_EQUAL;
    }
    else if (ctx->Less())
    {
      result->op = RelOp::LESS;
    }
    else if (ctx->LessEqual())
    {
      result->op = RelOp::LESS_EQUAL;
    }
    else if (ctx->Greater())
    {
      result->op = RelOp::GREATER;
    }
    else if (ctx->GreaterEqual())
    {
      result->op = RelOp::GREATER_EQUAL;
    }
    result->lhs.reset(visit(exps[0]).as<expr_node *>());
    result->rhs.reset(visit(exps[1]).as<expr_node *>());
    return result;
  }

  antlrcpp::Any AstBuilder::visitNumber(SafeCParser::NumberContext *ctx)
  {
    auto result = new number_node;
    if (auto number = ctx->IntConst())
    {
      result->btype = BType::INT;
      result->line = number->getSymbol()->getLine();
      result->pos = number->getSymbol()->getCharPositionInLine();
      auto value = number->getSymbol()->getText();
      if (value.length() >= 3 && value[0] == '0' && (value[1] == 'x' or value[1] == 'X'))
      {
        result->number = std::stoi(value, nullptr, 16);
      }
      else
      {
        result->number = std::stoi(value, nullptr, 10);
      }
    }
    return result;
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
      auto result = new unaryop_expr_node;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      if (ctx->Plus())
      {
        result->op = UnaryOp::PLUS;
      }
      else if (ctx->Minus())
      {
        result->op = UnaryOp::MINUS;
      }
      result->rhs.reset(visit(ctx->exp()[0]).as<expr_node *>());
      return static_cast<expr_node *>(result);
    }
    else if (ctx->lval())
    { // lval
      auto result = visit(ctx->lval()).as<lval_node *>();
      return static_cast<expr_node *>(result);
    }
    else if (ctx->number())
    { // number
      auto result = visit(ctx->number()).as<number_node *>();
      return static_cast<expr_node *>(result);
    }
    else
    { // LeftParen exp RightParen
      auto result = visit(ctx->exp()[0]).as<expr_node *>();
      return static_cast<expr_node *>(result);
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