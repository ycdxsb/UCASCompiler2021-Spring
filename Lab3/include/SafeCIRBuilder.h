#ifndef _SAFEC_IR_BUILDER_
#define _SAFEC_IR_BUILDER_

#include <deque>
#include <unordered_map>
#include <string>
#include <tuple>
#include <stdexcept>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "AstNode.h"
#include "AstNode_Visitor.h"
#include "runtime.h"
using namespace std;
using namespace llvm;

class SafeCIRBuilder : public AstNode_Visitor
{
    llvm::LLVMContext &context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<runtime_info> runtime;

    llvm::Value *value_result;
    int int_const_result;
    llvm::Function *current_function;
    int bb_count;

    bool lval_as_rval;
    bool in_global;
    bool constexpr_expected;
    bool error_flag;

    void visit(comp_root &node)
    {
        for (auto comp_unit : node.comp_units)
        {
            if (auto global_var_decl = std::dynamic_pointer_cast<var_def_stmt_node>(comp_unit))
            {
            }
            else if (auto global_func_def = std::dynamic_pointer_cast<func_def_node>(comp_unit))
            {
                in_global = false;
                global_func_def->accept(*this);
            }
        }
    };

    void visit(compunit_node &node){};

    void visit(global_def_node &node){};

    void visit(func_def_node &node)
    {
        current_function = Function::Create(FunctionType::get(Type::getVoidTy(context), {}, false),
                                            GlobalValue::LinkageTypes::ExternalLinkage,
                                            node.name, module.get());
        functions[node.name] = current_function;
        auto entry = BasicBlock::Create(context, "entry", current_function);
        builder.SetInsertPoint(entry);
        bb_count = 0;
        auto body = node.body;
        body->accept(*this);
        builder.CreateRetVoid();
    };

    void visit(block_node &node)
    {
        enter_scope();
        for (auto stmt : node.body)
        {
        }
        exit_scope();
    };

    void visit(stmt_node &node){

    };

    void visit(func_call_stmt_node &node)
    {
        if (functions.count(node.name) == 0)
        {
            cerr << node.line << ":" << node.pos << " Function: " << node.name << " is not decalared" << endl;
            exit(-1);
        }
        builder.CreateCall(functions[node.name], {});
    };

    void visit(empty_stmt_node &node)
    {
        return;
    };

    void visit(expr_node &node){

    };

    void visit(cond_node &node){

    };

    void visit(number_node &node)
    {
        if (constexpr_expected)
        {
            int_const_result = node.number;
        }
        else
        {
            value_result = ConstantInt::get(Type::getInt32Ty(context), node.number);
        }
    };

    void visit(binop_expr_node &node){};

    void visit(unaryop_expr_node &node)
    {
        if (auto visit_node = std::dynamic_pointer_cast<binop_expr_node>(node.rhs))
        {
            visit_node->accept(*this);
        }
        else if (auto visit_node = std::dynamic_pointer_cast<unaryop_expr_node>(node.rhs))
        {
            visit_node->accept(*this);
        }
        else if (auto visit_node = std::dynamic_pointer_cast<lval_node>(node.rhs))
        {
            visit_node->accept(*this);
        }
        else if (auto visit_node = std::dynamic_pointer_cast<number_node>(node.rhs))
        {
            visit_node->accept(*this);
        }
        if (!constexpr_expected && node.op == UnaryOp::MINUS)
        {
            // 非常数且为 - expr
            value_result = builder.CreateNSWNeg(value_result);
        }
        else if (constexpr_expected && node.op == UnaryOp::MINUS)
        {
            // 常数且为 - expr
            int_const_result = -int_const_result;
        }
    };

    void obc_check(Value *index, int array_length, int node_line, int node_pos, string name)
    {
    }

    void visit(lval_node &node){

    };

    void visit(var_def_stmt_node &node){

    };

    void visit(assign_stmt_node &node){

    };

    void visit(if_stmt_node &node){

    };
    void visit(while_stmt_node &node){

    };

public:
    SafeCIRBuilder(llvm::LLVMContext &ctx)
        : context(ctx), builder(ctx) {}

    bool debug = false;

    void log(string info)
    {
        if (debug)
        {
            cerr << info << endl;
        }
    }

    void build(std::string name, std::shared_ptr<ast_node> node)
    {
        // Initialize environment.
        module = std::make_unique<llvm::Module>(name, context);
        runtime = std::make_unique<runtime_info>(module.get());

        enter_scope();
        for (auto t : runtime->get_language_symbols())
        {
            llvm::GlobalValue *val;
            std::string name;
            bool is_function;
            bool is_const;
            bool is_array;
            bool is_obc;
            int array_length;
            std::tie(name, val, is_function, is_const, is_array, is_obc, array_length) = t;
            if (is_function)
                functions[name] = static_cast<llvm::Function *>(val);
            else
                declare_variable(name, val, is_const, is_array, is_obc, array_length);
        }

        lval_as_rval = true;
        in_global = true;
        constexpr_expected = false;
        error_flag = false;
        // Start building by starting iterate over the syntax tree.
        node->accept(*this);
        // Finish by clear IRBuilder's insertion point and moving away built module.
        builder.ClearInsertionPoint();
        exit_scope();

        if (error_flag)
        {
            module.release();
            runtime.release();
        }
    }

    std::unique_ptr<llvm::Module> get_module() { return std::move(module); }
    std::unique_ptr<runtime_info> get_runtime_info() { return std::move(runtime); }

private:
    void enter_scope() { variables.emplace_front(); }

    void exit_scope() { variables.pop_front(); }

    std::tuple<llvm::Value *, bool, bool, bool, int> lookup_variable(std::string name)
    {
        for (auto m : variables)
            if (m.count(name))
                return m[name];
        return std::make_tuple((llvm::Value *)nullptr, false, false, false, 0);
    }

    bool declare_variable(std::string name, llvm::Value *var_ptr, bool is_const, bool is_array, bool is_obc, int array_length)
    {
        if (variables.front().count(name))
            return false;
        variables.front()[name] = std::make_tuple(var_ptr, is_const, is_array, is_obc, array_length);
        return true;
    }

    std::deque<std::unordered_map<std::string, std::tuple<llvm::Value *, bool, bool, bool, int>>> variables;

    std::unordered_map<std::string, llvm::Function *> functions;
};
#endif