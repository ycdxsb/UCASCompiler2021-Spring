// Generated from SafeCParser.g4 by ANTLR 4.9.1
#ifndef _ASTBUILDER_
#define _ASTBUILDER_

#include <memory.h>
#include "antlr4-runtime.h"
#include "SafeCParserBaseVisitor.h"
#include "AstNode.h"

using namespace std;

namespace antlrcpp
{

  /**
 * This class provides an empty implementation of SafeCParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
  class AstBuilder : public SafeCParserBaseVisitor
  {
  public:
    bool debug = false;
    void log(string info)
    {
      if (debug)
      {
        cerr << info << endl;
      }
    }
    AstBuilder() {}
    ~AstBuilder() {}
    virtual antlrcpp::Any visitCompUnit(SafeCParser::CompUnitContext *ctx) override;
    virtual antlrcpp::Any visitDecl(SafeCParser::DeclContext *ctx) override;
    virtual antlrcpp::Any visitFuncDef(SafeCParser::FuncDefContext *ctx) override;
    virtual antlrcpp::Any visitConstDecl(SafeCParser::ConstDeclContext *ctx) override;
    virtual antlrcpp::Any visitBType(SafeCParser::BTypeContext *ctx) override;
    virtual antlrcpp::Any visitObcArray(SafeCParser::ObcArrayContext *ctx) override;
    virtual antlrcpp::Any visitUnobcArray(SafeCParser::UnobcArrayContext *ctx) override;
    virtual antlrcpp::Any visitArray(SafeCParser::ArrayContext *ctx) override;
    virtual antlrcpp::Any visitConstDef(SafeCParser::ConstDefContext *ctx) override;
    virtual antlrcpp::Any visitVarDecl(SafeCParser::VarDeclContext *ctx) override;
    virtual antlrcpp::Any visitVarDef(SafeCParser::VarDefContext *ctx) override;
    virtual antlrcpp::Any visitBlock(SafeCParser::BlockContext *ctx) override;
    virtual antlrcpp::Any visitBlockItem(SafeCParser::BlockItemContext *ctx) override;
    virtual antlrcpp::Any visitLval(SafeCParser::LvalContext *ctx) override;
    virtual antlrcpp::Any visitStmt(SafeCParser::StmtContext *ctx) override;
    virtual antlrcpp::Any visitCond(SafeCParser::CondContext *ctx) override;
    virtual antlrcpp::Any visitNumber(SafeCParser::NumberContext *ctx) override;
    virtual antlrcpp::Any visitExp(SafeCParser::ExpContext *ctx) override;
    ptr<ast_node> operator()(antlr4::tree::ParseTree *ctx);
  };
  
} // namespace antlrcpp

#endif