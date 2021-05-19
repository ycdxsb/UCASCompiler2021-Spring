#ifndef _ASTSERIALIZER_
#define _ASTSERIALIZER_

#include "AstNode_Visitor.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

using namespace antlrcpp;
using namespace std;

std::string RelOp_map[] = {"EQUAL", "NON_EQUAL", "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL"};
std::string BinOp_map[] = {"PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MODULO"};
std::string UnaryOp_map[] = {"PLUS", "MINUS"};
std::string BType_map[] = {"INT"};

template <typename Writer>
class AstSerializer : public AstNode_Visitor
{
public:
    AstSerializer(Writer &w) : writer(w) {}

    bool debug = false;

    void log(string info)
    {
        if (debug)
        {
            cerr << info << endl;
        }
    }

    void serialize(ast_node &node)
    {
        node.accept(*this);
    }

    virtual void visit(comp_root &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("comp_root");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("global_defs");
        writer.StartArray();
        for (auto comp_unit : node.comp_units)
        {
            comp_unit->accept(*this);
        }
        writer.EndArray();
        writer.EndObject();
    }

    virtual void visit(compunit_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("compunit_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.EndObject();
    }

    virtual void visit(global_def_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("global_def_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.EndObject();
    }

    virtual void visit(func_def_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("func_def_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("name");
        writer.String(node.name.c_str());
        writer.Key("body");
        node.body->accept(*this);
        writer.EndObject();
    }

    virtual void visit(expr_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("expr_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.EndObject();
    }

    virtual void visit(cond_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("cond_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("op");
        writer.String(RelOp_map[int(node.op)].c_str());
        writer.Key("lhs");
        node.lhs->accept(*this);
        writer.Key("rhs");
        node.rhs->accept(*this);
        writer.EndObject();
    }

    virtual void visit(binop_expr_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("binop_expr_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("op");
        writer.String(BinOp_map[int(node.op)].c_str());
        writer.Key("lhs");
        node.lhs->accept(*this);
        writer.Key("rhs");
        node.rhs->accept(*this);
        writer.EndObject();
    }

    virtual void visit(unaryop_expr_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("unaryop_expr_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("op");
        writer.String(UnaryOp_map[int(node.op)].c_str());
        writer.Key("rhs");
        node.rhs->accept(*this);
        writer.EndObject();
    }

    virtual void visit(lval_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("lval_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("name");
        writer.String(node.name.c_str());
        if (node.array_index)
        {
            writer.Key("array_index");
            node.array_index->accept(*this);
        }
        writer.EndObject();
    }

    virtual void visit(number_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("number_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("BType");
        writer.String(BType_map[int(node.btype)].c_str());
        writer.Key("number");
        writer.Int(node.number);
        writer.EndObject();
    }

    virtual void visit(stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.EndObject();
    }

    virtual void visit(var_def_stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("var_def_stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("is_const");
        writer.Bool(node.is_const);
        writer.Key("is_obc");
        writer.Bool(node.is_obc);
        writer.Key("name");
        writer.String(node.name.c_str());
        writer.Key("BType");
        writer.String(BType_map[int(node.btype)].c_str());
        if (node.array_length)
        {
            writer.Key("array_lengh");
            node.array_length->accept(*this);
            writer.Key("array_initializers");
            writer.StartArray();
            for (auto init : node.initializers)
            {
                init->accept(*this);
            }
            writer.EndArray();
        }
        else if (node.initializers.size() > 0)
        {
            writer.Key("vardef_initializer");
            node.initializers[0]->accept(*this);
        }
        writer.EndObject();
    }

    virtual void visit(assign_stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("assign_stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("target");
        node.target->accept(*this);
        writer.Key("value");
        node.value->accept(*this);
        writer.EndObject();
    }

    virtual void visit(func_call_stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("func_call_stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("name");
        writer.String(node.name.c_str());
        writer.EndObject();
    }

    virtual void visit(block_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("block_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("body");
        writer.StartArray();
        for (auto stmt : node.body)
        {
            stmt->accept(*this);
        }
        writer.EndArray();
        writer.EndObject();
    }

    virtual void visit(if_stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("if_stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("cond");
        node.cond->accept(*this);
        writer.Key("If_body");
        node.if_body->accept(*this);
        if (node.else_body)
        {
            writer.Key("Else_body");
            node.else_body->accept(*this);
        }
        writer.EndObject();
    }

    virtual void visit(while_stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("while_stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.Key("cond");
        node.cond->accept(*this);
        writer.Key("body");
        node.body->accept(*this);
        writer.EndObject();
    }

    virtual void visit(empty_stmt_node &node) override
    {
        writer.StartObject();
        writer.Key("type");
        writer.String("empty_stmt_node");
        writer.Key("line");
        writer.Int(node.line);
        writer.Key("pos");
        writer.Int(node.pos);
        writer.EndObject();
    }

private:
    Writer &writer;
};

#endif