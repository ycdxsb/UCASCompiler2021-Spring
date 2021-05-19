## 实验任务

1. 熟悉 `ANTLR4` 访问者模式和访问者模式下的调试方法
2. 熟悉 `C++`类型转换与继承相关知识
3. 根据已有框架，实现对语法分析树的访问，生成`Json`表示的抽象语法树
4. 自行编写测试样例对代码进行测试，尤其是避免由于类型转换等导致的`Segmentation fault`问题

## 实验框架介绍

Lab2实验目录如下：

```
Lab2
├── cmake
│   ├── antlr4-generator.cmake.in
│   ├── Antlr4Package.md
│   ├── antlr4-runtime.cmake.in
│   ├── ExternalAntlr4Cpp.cmake
│   ├── FindANTLR.cmake
│   └── README.md
├── CMakeLists.txt
├── include
│   ├── AstBuilder.h
│   ├── AstNode.h
│   ├── AstNode_Visitor.h
│   └── AstSerializer.h
├── main.cpp
├── SafeCLexer.g4
├── SafeCParser.g4
├── src
│   └── AstBuilder.cpp
└── test_cases
    ├── decl
    ├── expr
    ├── ifelse
    ├── stmt
    └── while
```

- `cmake`目录：提供ANTLR4项目构建所需要的cmake文件
- `CMakeLists.txt`文件：编译项目所需要的CMakeList文件
- `main.cpp`：项目的入口文件
- `SafeCLexer.g4、SafeCParser.g4`：已实现完成的SafeC语法文件
- `AstNode.h`：实验提供的数据结构，需要阅读了解
- `AstBuilder.h/AstBuilder.cpp`：实验任务主要文件，需要同学们添加代码
- `AstSerializer.h`：将生成的Ast树序列化为json格式的文件，需要阅读了解
- `test_cases` ：测试文件所在目录

**注意：如果在实验一中未加入其它功能以及非终结符，以上文件理论上除`AstBuilder.cpp`外，都不需要做任何功能上的修改（debug信息除外）**

### Main介绍

```c
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

`main`函数流程如下：

1. 首先打开文件流，输入给`ANTLR4`生成的`SafeCLexer`进行词法分析，获取`token`输出到屏幕上
2. 将`token`输入到`SafeCParser`进行语法分析，获取分析结果中的根节点`compUnit`，输出到屏幕上
3. 最后，我们将语法树输入给我们实现的`AstBuilder`类中生成抽象语法树，并使用`AstSerializer`类将抽象语法树`dump`为`json`格式

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

1. 首先是定义了一些枚举类型，包括`BinOp`、`UnaryOp`、`RelOp`以及`BType`，虽然`SafeC`目前只有`INT`类型，但定义`BType`对于自己添加类型更加方便。
2. 然后定义所有`AST`节点的父节点`ast_node`。只包含节点的行，列信息

```c++
struct ast_node
{
    int line;
    int pos;
    virtual void accept(AstNode_visitor &visitor) = 0;
};
```

虽然在实现上可以让所有节点都只继承`ast_node`，但为了显示树的层次结构，我们并不这样实现

3. 定义编译的`comp_root`节点。`comp_root`节点由`compunit_node`组成，包括全局的变量定义以及函数定义
```c++
struct comp_root : virtual ast_node
{
    ptr_vector<compunit_node> comp_units;
    virtual void accept(AstNode_visitor &visitor) override;
};
```
4. 定义编译单元`compunit_node`、全局变量定义以及函数定义。`compunit_node`是虚继承于`ast_node`，为的是从继承上体现树的层次结构，`global_def_node`和`func_def_node`都继承于`compunit_node`，表示它们都是`compunit_node`的一种。

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

5. 定义表达式结构，分为`cond_node`，`binop_expr_node`，`unaryop_expr_node`，`lval_node`和`number_node`

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
    void log();
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

`ASTBuilder`建立在`SafeCParserBaseVisitor`的基础上，`SafeCParserBaseVisitor`可以通过`antlr4 -Dlanguage=Cpp -visitor -no-listener *.g4`命令生成。

**注意：**

**在实验一实现的g4文件中，如果加入了其他非终结符，那么生成的`SafeCParserBaseVisitor`接口会与上面不同，对增加的每一个非终结符，ANTLR4都会生成对应的访问者函数，因此需要参考已有代码，修改AstBuilder.h和AstBuilder.cpp，加入对应的访问者函数，并实现相应功能**

`AstBuilder`是一个继承自`SafeCParserBaseVisitor`类的访问者类，用于实现对ANTLR4解析后的语法分析树进行访问，其中的每个虚方法都是我们在实验中需要实现的内容。

在给出的代码中，已经实现了部分方法，可以帮助更快的了解相应的编程接口

> 在代码实现过程中，对`Context`对象的结构的了解非常重要，可以查看`SafeCParser.h`中的结构定义。
>
> 如果已经编译过项目，那么`SafeCParser.h`在`build/antlr4cpp_generated_src/SafeCParser/`下
>
> 否则可以通过`ANTLR4`生成代码查看，命令如下：`antlr4 -Dlanguage=Cpp -visitor -no-listener *.g4`

1. `ANTLR4`获取`token`符号的信息

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

我们在语法文件中，`number->intConst`，因此对于`ANTLR4`生成的`Number`上下文，可以通过`ctx->IntConst()`获取到对应的`intConst`对应的`token`，当获取成功时，我们可以通过`Context`对象的`->getSymbol()->getLine()`获取所在行号，以及通过`->getSymbol()->getCharPositionInLine();`获取所在列号，同时可以通过`->getSymbol()->getText();`获取对应的字符串，在上面的代码中，我们可以使用代码对字符串进行处理(16进制/10进制)获取int常量数字，填充到结构体`number_node`的成员中

2. 在对`exp`节点处理时，由于我们在语法文件对`exp`的推导为`exp-> (plus | minus) exp |exp binop exp ...`，也就是说`exp`的子节点中可能有多个`exp`节点，因此通过`->exp()`访问子节点时，ANTLR4会返回一个`vector`，在代码中可以通过下列方式判断`exp`推导的产生式是哪个

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



### 代码调试

在入口处使用`-d`参数输出调试信息，在`AstBuilder.cpp`中，通过调用`log`函数输出调试信息，这对访问者模式下调试`Segmentation Fault`十分有用，可以通过在访问节点前输出`log`，看是对哪个节点的处理出了问题。

**注意：Segmentation Fault的问题也可能出现在AstSerializer中（当未对结构体中每个成员正确赋值时），此时也需要对AstSerializer进行调试**

```
    void log(string info)
    {
      if (debug)
      {
        cerr << info << endl;
      }
    }
```

在代码实现时，可以选择自下而上实现，优先实现下层的代码，这个时候可以通过下面的方法进行测试：

**修改main函数中，parser解析出来的语法树入口点，在main中本身未compUnit，在传入AstBuilder后，会从compUnit进行访问，由于AstBuilder会根据传入的类型选择对应的访问者函数，因此我们可以修改入口点，直接访问我们正在实现的函数**

```
// main.cpp
auto tree = parser.compUnit();
auto ast = ast_builder(tree);
```



## 代码编译和使用

### 编译

修改`CMakeLists.txt`中的`ANTLR_EXECUTABLE`路径，为自己环境中`jar`包所在路径

```
set(ANTLR_EXECUTABLE /home/ucascompile/ucascompile/antlr4/antlr-4.9.1-complete.jar)
```

1. 新建`build`目录`mkdir build`
2. 在`build`目录下使用`cmake .. && make`编译
3. 编译成功后会在`build`目录下生成名为`astbuilder`的可执行文件

可能出现的问题和解决方案：

1. 在编译过程中，会从`github`下载依赖，因此需要保证网络正常，防止`git clone error`

2. 问题`fatal error: SafeCParserBaseVisitor.h: No such file or directory`

   解决方案如下：

   - 先将`CMakeLists.txt`中的最后一行`target_link_libraries(astbuilder AstBuilder)`使用#号注释掉，回到`build`目录下，使用`cmake .. && make`编译一次，此时会编译失败
   - 将#号删除，回到`build`目录下，使用`cmake .. && make`编译一次，此时会成功

   **注意：如果出现上面的错误，那么每次修改g4文件后，都应重新操作一遍**



### 使用

使用方法：`./astbuilder filepath`或`./astbuilder filepath -d`，当使用`-d`命令时，会输出相应的`debug`信息，`debug`信息需要通过在代码中添加`log`函数输出，可以帮助调试

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

1. 对于已给出的示例，实验提交时应当无`Segmentation Fault`问题

2. 除已经给出的测试集外，新建`self_test_cases`文件夹，构造测试样例`10`个，尽可能的对自己实现的代码进行覆盖性测试，无具体命名格式要求

3. 最终提交时，实验目录如下，**不需要提交build目录**：

```
Lab2/
├── cmake
│   ├── antlr4-generator.cmake.in
│   ├── Antlr4Package.md
│   ├── antlr4-runtime.cmake.in
│   ├── ExternalAntlr4Cpp.cmake
│   ├── FindANTLR.cmake
│   └── README.md
├── CMakeLists.txt
├── main.cpp
├── SafeCLexer.g4
├── SafeCParser.g4
├── src
│   ├── AstBuilder.h
│   ├── AstNode.h
│   └── AstSerializer.h
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



## 一些参考资料

- [访问者模式-菜鸟教程](https://www.runoob.com/design-pattern/visitor-pattern.html)

- [ANTLR学习笔记4：语法导入和访问者(Visitor)模式](https://blog.csdn.net/SHU15121856/article/details/106331151)

- [c++ 强制类型转换](https://zhuanlan.zhihu.com/p/101493574)

