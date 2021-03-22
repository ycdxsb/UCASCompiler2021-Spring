## 实验任务

1. 熟悉 ANTLR4 访问者模式和访问者模式下的调试方法
2. 熟悉 C++类型转换与继承相关知识
3. 根据已有框架，实现对AST语法树的访问，生成Json表示的抽象语法树
4. 自行编写测试样例对代码进行测试，尤其是避免由于类型转换等导致的`Segmentation fault`问题

## 实验框架介绍

实验框架目录如下：

```
Lab2/
├── build
├── cmake
│   ├── antlr4-generator.cmake.in
│   ├── Antlr4Package.md
│   ├── antlr4-runtime.cmake.in
│   ├── ExternalAntlr4Cpp.cmake
│   ├── FindANTLR.cmake
│   └── README.md
├── CMakeLists.txt
├── main.cpp
├── SafeCLexer.g4
├── SafeCParser.g4
├── src
│   ├── AstBuilder.h
│   ├── AstNode.h
│   └── AstSerializer.h
└── test_cases
    ├── decl
    ├── expr
    ├── ifelse
    ├── stmt
    └── while
```

- build目录：build项目所在目录
- cmake目录：提供ANTLR4项目构建所需要的cmake文件
- CMakeLists.txt文件：编译项目所需要的CMakeList文件
- main.cpp：项目的入口文件
- SafeCLexer.g4、SafeCParser.g4：已实现完成的SafeC语法文件
- src/AstNode.h：实验提供的数据结构，需要阅读了解
- src/AstBuilder.h：实验任务主要文件，需要同学们添加代码
- src/AstSerializer.h：将生成的Ast树序列化为json格式的文件，需要阅读了解

- test_cases/ ：测试文件所在目录

注意，以上文件除`AstBuilder.h`外，都不需要做任何修改

### Main介绍

```
  stream.open(argv[1]);
  ANTLRInputStream input(stream);

  SafeCLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  tokens.fill();
  for (auto token : tokens.getTokens())
  {
    cout << token->toString() << endl;
  }

  SafeCParser parser(&tokens);
  auto tree = parser.compUnit();

  cout << tree->toStringTree(&parser) << endl;
  
  AstBuilder ast_builder;
  auto ast = ast_builder(tree);
  AstSerializer<decltype(writer)> astserializer(writer);
  astserializer.serialize(*ast);
  cout << s.GetString() << endl;
```

main函数流程如下：

1. 首先打开文件流，输入给ANTLR4生成的SafeCLexer进行词法分析，获取token输出到屏幕上
2. 将token输入到SafeCParser进行语法分析，获取分析结果中的根节点compUnit，输出到屏幕上
3. 最后，我们将语法树输入给我们实现的AstBuilder类中生成抽象语法树，并使用AstSerializer类将抽象语法树dump为json格式

### AstNode数据结构介绍

```c++
// AstNode.h
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
```

1. 首先是定义了一些枚举类型，包括BinOp、UnaryOp、RelOp以及BType，虽然SafeC目前只有INT类型，但定义BType对于自己添加类型更加方便。
2. 然后定义所有AST节点的base节点`ast_node`。只包含节点的行，列信息

```c++
struct ast_node
{
    int line;
    int pos;
    virtual void accept(AstNode_visitor &visitor) = 0;
};
```

虽然在实现上可以让所有节点都只继承ast_node，但为了显示树的层次结构，我们并不这样实现

3. 定义编译的comp_root节点。comp_root节点由compunit_node组成，包括全局的变量定义以及函数定义
```c++
struct comp_root : virtual ast_node
{
    ptr_vector<compunit_node> comp_units;
    virtual void accept(AstNode_visitor &visitor) override;
};
```
4. 定义编译单元compunit_node、全局变量定义以及函数定义。compunit_node是虚继承于ast_node，为的是从继承上体现树的层次结构，global_def_node和func_def_node都继承于compunit_node，表示它们都是compunit_node的一种。

```c++
struct compunit_node : virtual ast_node
{
    virtual void accept(AstNode_visitor &visitor) override;
};

struct global_def_node : compunit_node
{
    virtual void accept(AstNode_visitor &visitor) override;
};

struct func_def_node : compunit_node
{
    std::string name;
    ptr<block_node> body;
    virtual void accept(AstNode_visitor &visitor) override;
};
```

5. 定义表达式结构，分为cond_node，binop_expr_node，unaryop_expr_node，lval_node和number_node

```c++
struct expr_node : virtual ast_node
{
    virtual void accept(AstNode_visitor &visitor) override;
};

struct cond_node : expr_node
{
    RelOp op;
    ptr<expr_node> lhs, rhs;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct binop_expr_node : expr_node
{
    BinOp op;
    ptr<expr_node> lhs, rhs;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct unaryop_expr_node : expr_node
{
    UnaryOp op;
    ptr<expr_node> rhs;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct lval_node : expr_node
{
    std::string name;
    ptr<expr_node> array_index;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct number_node : expr_node
{
    BType btype;
    int number;
    virtual void accept(AstNode_visitor &visitor) override;
};
```

6. 定义语句，需要注意的是`var_def_stmt_node : stmt_node, global_def_node`，表明不管是全局的变量定义，还是内部的变量定义，我们都通过`var_def_stmt_node`这个结构进行表示

```c++
struct stmt_node : virtual ast_node
{
    virtual void accept(AstNode_visitor &visitor) override;
};

struct var_def_stmt_node : stmt_node, global_def_node
{
    bool is_const;
    BType btype;
    std::string name;
    bool is_obc;
    ptr<expr_node> array_length;
    ptr_vector<expr_node> initializers;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct assign_stmt_node : stmt_node
{
    ptr<lval_node> target;
    ptr<expr_node> value;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct func_call_stmt_node : stmt_node
{
    std::string name;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct block_node : stmt_node
{
    ptr_vector<stmt_node> body;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct if_stmt_node : stmt_node
{
    ptr<cond_node> cond;
    ptr<stmt_node> if_body;
    ptr<stmt_node> else_body;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct while_stmt_node : stmt_node
{
    ptr<cond_node> cond;
    ptr<stmt_node> body;
    virtual void accept(AstNode_visitor &visitor) override;
};

struct empty_stmt_node : stmt_node
{
    virtual void accept(AstNode_visitor &visitor) override;
};
```

### AstBuilder介绍

```c++
 /**
 * This class provides an empty implementation of SafeCParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
  class AstBuilder : public SafeCParserBaseVisitor
  {
  public:
    AstBuilder();
    ~AstBuilder();
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
```

AstBuilder是一个继承自SafeCParserBaseVisitor类的访问者类，用于实现对ANTLR4解析后的抽象语法树进行访问，其中的每个虚方法都是我们在实验中需要实现的内容。

在给出的代码中，已经实现了部分方法，可以更快的了解相应的编程接口

1. ANTLR4获取符号的信息

```
  antlrcpp::Any AstBuilder::visitNumber(SafeCParser::NumberContext *ctx)
  {
    auto result = new number_node;
    if (auto number = ctx->IntConst())
    {
      result->btype = BType::INT;
      result->line = number->getSymbol()->getLine();
      result->pos = number->getSymbol()->getCharPositionInLine();
      auto value = number->getSymbol()->getText();
      ...
    }
    return static_cast<expr_node *>(result);
  }
```

我们在语法文件中，number->intConst，因此对于ANTLR4生成的Number上下文，可以通过ctx->IntConst()获取到对应的intConst对应的token，当获取成功时，我们可以通过Context对象的`->getSymbol()->getLine()`获取所在行号，以及通过`->getSymbol()->getCharPositionInLine();`获取所在列号，同时可以通过`->getSymbol()->getText();`获取对应的字符串，在上面的代码中，我们可以使用代码对字符串进行处理(16进制/10进制)获取int常量数字，填充到结构体number_node的成员中

2. 在对exp节点处理时，由于我们在语法文件对exp的推导为`exp-> (plus | minus) exp |exp binop exp ...`，也就是说exp的子节点中可能有多个exp节点，因此通过`->exp()`访问子节点时，ANTLR4会返回一个数组

```
antlrcpp::Any AstBuilder::visitExp(SafeCParser::ExpContext *ctx)
{
    auto exps = ctx->exp();
    if (exps.size() == 2)
    { // exp op exp 
      ...
    }
    else if(exps.size()==2)
    { // unaryop exp
      ...
    }
    else if (ctx->lval())
    { // lval
      ...
    }
    else if (ctx->number())
    { // number
      ...
    }
    else
    { // LeftParen exp RightParen
      ...
    }
}
```



## 代码编译和使用

1. 编译代码，在build目录中使用`cmake .. && make`编译，成功后会生成名为astbuilder的可执行文件
2. 使用方法：`./astbuilder filepath`或`./astbuilder filepath -d`，当使用-d命令时，会输出相应的debug信息，可以帮助调试

示例：

```shell
$ cat simple.c
void f()
{
   int a = 1;
}
$ ./astbuilder simple.c
[@0,0:3='void',<25>,1:0]
[@1,5:5='f',<28>,1:5]
[@2,6:6='(',<8>,1:6]
[@3,7:7=')',<9>,1:7]
[@4,9:9='{',<6>,2:0]
[@5,14:16='int',<24>,3:3]
[@6,18:18='a',<28>,3:7]
[@7,20:20='=',<3>,3:9]
[@8,22:22='1',<29>,3:11]
[@9,23:23=';',<2>,3:12]
[@10,25:25='}',<7>,4:0]
[@11,27:26='<EOF>',<-1>,5:0]
(compUnit (funcDef void f ( ) (block { (blockItem (decl (varDecl (bType int) (varDef a = (exp (number 1))) ;))) })) <EOF>)
Ast:
{
    "type": "comp_root",
    "line": 1,
    "pos": 0,
    "global_defs": [
        {
            "type": "func_def_node",
            "line": 1,
            "pos": 0,
            "name": "f",
            "body": {
                "type": "block_node",
                "line": 2,
                "pos": 0,
                "body": [
                    {
                        "type": "var_def_stmt_node",
                        "line": 3,
                        "pos": 7,
                        "is_const": false,
                        "is_obc": false,
                        "name": "a",
                        "BType": "INT",
                        "vardef_initializer": {
                            "type": "number_node",
                            "line": 3,
                            "pos": 11,
                            "BType": "INT",
                            "number": 1
                        }
                    }
                ]
            }
        }
    ]
}
```



## 实验提交要求

除已经给出的测试集外，新建self_test_cases文件夹，构造测试样例10个，尽可能的对自己实现的代码进行覆盖性测试，无具体命名格式要求。

最终提交时，实验目录如下：

```
Lab2/
├── build
├── cmake
│   ├── antlr4-generator.cmake.in
│   ├── Antlr4Package.md
│   ├── antlr4-runtime.cmake.in
│   ├── ExternalAntlr4Cpp.cmake
│   ├── FindANTLR.cmake
│   └── README.md
├── CMakeLists.txt
├── main.cpp
├── SafeCLexer.g4
├── SafeCParser.g4
├── src
│   ├── AstBuilder.h
│   ├── AstNode.h
│   └── AstSerializer.h
└── test_cases
│   ├── decl
│   ├── expr
│   ├── ifelse
│   ├── stmt
│   └── while
└── self_test_cases
     ├── test1.c
     ├── test2.c
     ├── ...
```

