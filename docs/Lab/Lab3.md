## 实验任务

1. 熟悉LLVM IR 表示和 中间代码生成的相关接口(IRBuilder)

2. 熟悉在IR生成中对while , if else等较复杂语句的生成

3. 对有obc属性的数组访问进行越界检查，插入检查代码，在数组访问越界时进行报错和错误处理

   

## SafeCGrammar

```
// SafeC Grammar
CompUnit    → [ CompUnit ] ( Decl | FuncDef )
Decl        → ConstDecl
            | VarDecl
Obc	        → 'obc'
BType       → 'int'
ConstDecl   → 'const' BType ConstDef { ',' ConstDef } ';'
ConstDef    → Ident '=' Exp
            | Ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
VarDecl     → BType VarDef { ',' VarDef } ';'
VarDef      → Ident
            | Array
            | Ident '=' Exp
            | Array '=' '{' Exp { ',' Exp } '}'
UnObcArray  → Ident '[' Exp ']'
ObcArray    → Obc Ident '[' Exp ']'
Array       → ObcArray | UnObcArray 
FuncDef     → void Ident '(' ')' Block
Block       → '{' { BlockItem } '}'
BlockItem   → Decl
            | Stmt
Stmt        → LVal '=' Exp ';'
            | Ident '(' ')' ';'
            | Block
            | 'if' '( Cond ')' Stmt [ 'else' Stmt ]
            | 'while' '(' Cond ')' Stmt
            | ';'
LVal        → Ident
            | Ident '[' Exp ']'
Cond        → Exp RelOp Exp
RelOp       → '==' | '!=' | '<' | '>' | '<=' | '>='
Exp         → Exp BinOp Exp
            | UnaryOp Exp
            | '(' Exp ')'
            | LVal
            | Number
Number      IntConst
BinOp       → '+' | '−' | '*' | '/' | '%'
UnaryOp     → '+' | '−'
```

## LLVM 中间代码生成

在实验二中，我们已经实现了`AstBuilder.cpp`，能够根据`ANTLR4`语法分析的结果，生成对应的抽象语法树。本次实验利用生成的抽象语法树，通过遍历生成的抽象语法树节点，调用`LLVM`的中间代码生成接口，生成对应的中间代码。

### main函数逻辑

```c
int main(int argc, const char **argv)
{
  bool debug = false;
  std::ifstream stream;
  std::string filename;
  if (argc != 2 && argc != 3)
  {
    cout << "usage : ./irbuilder filepath [-d]" << endl;
    cout << "help  : -d debug" << endl;
    return -1;
  }
  filename = argv[1];
  if (argc == 3 && strcmp(argv[2], "-d") == 0)
  {
    debug = true;
  }
  stream.open(filename);
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

  ast_builder.debug = debug;
  auto ast = ast_builder(tree);
  string module_name = filename;
  module_name = module_name.substr(module_name.find_last_of("/\\") + 1);
  cout << "Module Name " << module_name << endl;

  LLVMContext ctx;
  SafeCIRBuilder irbuilder(ctx);
  irbuilder.debug = debug;
  irbuilder.build(module_name, ast);
  auto module = irbuilder.get_module();
  auto runtime = irbuilder.get_runtime_info();

  module->print(outs(), nullptr);
  auto entry_func = module->getFunction("main");
  if (!entry_func)
  {
    cerr << "No 'main' function presented. Exiting." << endl;
    exit(-1);
  }
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  for (auto t : runtime->get_runtime_symbols())
    sys::DynamicLibrary::AddSymbol(get<0>(t), get<1>(t));

  string error_info;
  unique_ptr<ExecutionEngine> engine(EngineBuilder(move(module))
                                         .setEngineKind(EngineKind::JIT)
                                         .setErrorStr(&error_info)
                                         .create());
  if (!engine)
  {
    cerr << "EngineBuilder failed: " << error_info << endl;
    exit(-1);
  }
  engine->runFunction(entry_func, {});
  return 0;
}
```

`main`函数逻辑如下：

- 前部同实验二，打开输入文件，然后使用SafeCLexer和SafeCParser进行词法语法分析，然后使用`AstBuilder`生成抽象语法树
- 通过本次实验需要实现的`SafeCIRBuilder`根据抽象语法树生成中间代码
- 在中间代码中找到运行入口`main`函数，使用中间代码执行引擎执行生成的中间代码

### LLVM IR Builder

LLVM IR是

### SafecIRBuilder

`SafeCIRBuilder`是继承了`AstNode_Visitor`的类，维护了 `llvm::Module`、`llvm::IRBuilder`等构建 `LLVM IR` 所必须的类，并通过一些约定进行代码生成

#### 主要成员简介

```c++
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
```

- `context`、`builder`、`module`、`runtime`：会在各种` LLVM` 接口中要求传入，包括指针形式和引用形式。注意，请使用 `module.get()` 来获取 `module` 的指针，否则会引起 `unique_ptr` 的 `deconstruction`
- `value_result`：用来保存表达式结果，存储`Value`对象，将子节点的结果`Value`对象返回给上层调用
- `int_const_result`：用来保存表达式结果，对于常数的表达式结果，存储在`int_const_result`中，返回给上层调用。例如在数组声明时，对于`int A[1+1];`，通过获取表达式`1+1`的常数结果，使用`builder.CreateAlloca`新建一个长度为`2`的数组
- `current_function`： 保存了当前正在生成的函数对象指针。对于 `if` 和 `while` 语句的中间代码生成会有帮助，在为这些语句创建新的 `BasicBlock` 时需要传入这一指针
- `bb_count` ：用于对当前函数中的 `BasicBlock` 数量进行计数，用于对函数中的各个 `BasicBlock` 命名
- `lval_as_rval` ：用于表示当前上下文以何种方式使用正要处理的左值。如为真(即右值)，则 `visit(lval_syntax &)` 应对得到的左值地址取值；否则不进行取值
- `in_global` ：用于表示当前是否处于全局区域，是为了区分变量声明语句 `var_def_stmt_syntax` 在全局和局部的不同行为而设立的 `flag`
- `constexpr_expected`：用于标志将表达式结果存储在`value_result`还是在`int_const_result`中，避免存取出错
- `variables`：用于存储所有的变量
  - `enter_scope`：用于管理变量作用域，例如在进入函数体前，会使用`enter_scope`新建一个`std::unordered_map<std::string, std::tuple<llvm::Value *, bool, bool, bool>>`存储该作用域内的变量
  - `exit_scope`：用于管理变量作用域，与`enter_scope`相应
  - `declare_variable`：当遇到变量声明时，使用`declare_variable`声明变量，将信息存储到`variables`中，如果已声明改变量，则返回`false`，否则返回`true`
- `functions`：用于存储所有的函数

#### 静态语义检查（不要求）

在代码生成的过程中，可以顺便进行一些语义上的简单检查，当检查到问题时，应当进行合适的报错，例如

- 重复声明检查：函数、变量的重复声明
- 未定义使用检查：在使用变量前，应当先定义
- 算数运算检查：例如除数和模数不应该为0等等
- 左值检查：赋值运算的左值不能为常量等等。
- 数组越界检查：例如定义了`a[2]`，但是访问`a[3]`这类不需要做 静态数据流分析 的简单情形
- 声明了数组`a[2]`，却访问变量`a`；或者声明了变量`a`，却访问`a[2]`
- ...

#### 函数创建

调用 `Function::Create` 创建函数的IR对象时，需要指定函数类型、链接方式、函数名和模块，由于`SafeC`语言的函数均没有参数，并且返回类型为空，因此可以固定地按如下方法来创建：

```c
current_function = Function::Create(FunctionType::get(Type::getVoidTy(context), {}, false),
                                    GlobalValue::LinkageTypes::ExternalLinkage,
                                    name, module.get());
```

函数对象创建后，需要为其准备入口点对应的 `BasicBlock`。可以通过调用 `BasicBlock::Create`创建 BB，并使用 `builder.SetInsertPoint`方法设置好 `IRBuilder` 的插入点。

在完成对函数内语句的中间代码指令生成后，别忘了调用 `IRBuilder::CreateRetVoid`添加返回指令 `ret void`。

#### 表达式处理

对于表达式的处理，分为两种情况，根据`constexpr_expected`区分：

- `constexpr_expected`为`true`：此时应当在处理子表达式后, 通过 `int_const_result`取出结果，根据表达式的行为直接计算出常量结果，存回到 `int_const_result`内
- `constexpr_expected`为`false`：此时应当在处理子表达式后，通过 `value_result` 取出结果，根据表达式的行为调用相应的`builder` 的成员方法进行代码生成，将生成的匿名变量存回到 `value_result` 内，对于常量，可以构建一个`ConstantInt`对象：例如`ConstantInt::get(Type::getInt32Ty(context), node.number)`将常量转换为`ConstantInt`

#### 变量定义处理

对于变量定义的处理，分为两种情况，根据`in_global`区分

- `in_global`为`false`：说明当前声明的变量为局部变量
  - 对于局部变量，应当通过调用 `IRBuilder::CreateAlloca` 创建 `alloca` 指令。这一指令能够在栈上分配空间，需要思考如何使用 `alloca` 指令创建变量并获取指向它的指针值
  - 对于局部变量，无论变量为常量或可变量，其初始化表达式均**不要求**是常量表达式
- `in_global`为`true`：说明当前声明的变量为全局变量
  - 对于全局变量，需要使用类似于 `new GlobalVariable` 的方式在当前 `module` 中创建一个全局变量定义，`GlobalVariable`所代表的值本身也是一个指向变量的指针，因此和 `alloca` 指令的结果是一致的，可以统一在`lval_node`中处理
  - 对于全局变量，无论变量为常量或可变量，其初始化表达式均**要求**是常量表达式，只需要使用构造函数 `GlobalVariable::GlobalVariable`中的 `Initializer` 参数指定初始化的常量值即可；根据变量类型不同，它应为一个 `ConstantInt`  或`ConstantArray`

变量定义好后，需要通过 `declare_variable` 将其加入到当前上下文环境中

#### 左值与右值处理

对于左值的使用，分为两种情况：作为左值和作为右值使用，根据 `lval_as_rval` 区分

- `lval_as_rval`为`false`：作为左值使用，即作为被赋值的对象。此时应当取出其地址，存入 `value_result` 中。在过程中，需要检查变量的类型，对数组引用进行正确的索引（通过 `getelementptr` 指令）
- `lval_as_rval`为`true`：当作为右值使用时，除了上述步骤外，你还需要将值取出（通过 `load`指令），然后将取出的值所关联的 `Value` 指针存入 `value_result` 中

#### 控制流结构处理

在`SafeC`中，存在`if else`和`while`两类控制流语句，需要对这两类进行处理

在` LLVM IR` 中，单个 `BasicBlock` 中最后一条语句应为跳转，包括 `ret void`（`builder.CreateRetVoid`）、`br`（`builder.CreateBr`等）。对于控制流结构，需要分析它需要哪几个 `BasicBlock `来完成其功能，然后进行这些` BasicBlock` 的创建，分别设置 `builder` 的插入点为这些块（`builder.SetInsertPoint`）并进行相应的代码生成。别忘了一定要有一个` BB `块用作后续代码生成，即这一控制流结构结束后的语句的` IR` 插入点。

`BasicBlock `的创建可固定地使用`BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function)`

## SafeC运行时

SafeC语言在设计时并不支持输入和输出，且函数没有参数。为了方便测试程序执行结果，因此加入运行时，方便我们对中间代码进行测试，包含如下全局变量和函数。

```
int input_var;
int output_var;

void input(int *i)
{
    scanf("input:%d", i);
}

void output(int *i)
{
    printf("output:%d\n", *i);
}
```

当执行`input`函数时，会将输入结果存储到全局变量 `input_var`中。

当执行`output`函数时，会将全局变量`output_var`中的结果输出出来。

具体的运行时在`runtime.h/runtime.cpp`和`io.h/io.cpp`中实现

对于文件`simple.c`

```
void main(){
    output_var = 10;
    output();
}
```

通过`irbuilder`，获得中间代码如下

```c
Module Name simple.c
; ModuleID = 'simple.c'
source_filename = "simple.c"

@input_var = global i32 0
@output_var = global i32 0
@line = global i32 0
@pos = global i32 0

declare void @input_impl(i32*)

declare void @output_impl(i32*)

declare void @obc_check_error_impl(i32*, i32*)

define void @input() {
entry:
  call void @input_impl(i32* @input_var)
  ret void
}

define void @output() {
entry:
  call void @output_impl(i32* @output_var)
  ret void
}

define void @obc_check_error() {
entry:
  call void @obc_check_error_impl(i32* @line, i32* @pos)
  ret void
}

define void @main() {
entry:
  store i32 10, i32* @output_var, align 4
  call void @output()
  ret void
}
```

最后执行中间代码，输出`output:10`。

## 动态数组越界检查

对于一些数组，我们是有obc属性的，当我们不确定具体访问的范围时，应当插入代码进行动态执行检查，例如`int idx = 3; int obc a[2]; a[idx]=1;`这类，当我们未进行静态数据流分析时，并不知道`a[idx]`会访问哪一块空间，因此需要在`IR`生成中加入动态检查代码

在`runtime.h`中，有`line、pos、str`三个全局变量，报错通过`io.h`中的`obc_check_error`输出信息

```c
// runtime.h
llvm::GlobalVariable *line;
llvm::GlobalVariable *pos;
llvm::GlobalVariable *str;

// io.h
void obc_check_error(int *,int *,char*);
```

只要将节点的行列信息和变量名分别存入`line、pos`和`str`中，并插入函数调用即可插入运行时越界检查的代码

```c
line = ConstantInt::get(Type::getInt32Ty(context), node_line);
builder.CreateStore(line, line_addr);
pos = ConstantInt::get(Type::getInt32Ty(context), node_pos);
builder.CreateStore(pos, pos_addr);        
str = ConstantDataArray::getString(module->getContext(),(string("obc Array") + string("[") + name + string("]")).c_str(), true);
builder.CreateStore(str, str_addr);
        
builder.CreateCall(functions["obc_check_error"], {});
```

**示例**

```c
$ cat -n simple.c 
     1  int n = 20;
     2  int obc fib[50] = {0};
     3  void fib(){
     4      fib[0] = 1;
     5      fib[1] = 1;
     6      int i = 2;
     7      while(i < n){
     8          fib[i] = fib[i-1] + fib[i-2];
     9          i = i + 1;
    10      }        
    11  }
    12  void output_fib(){
    13      int i = 0;
    14      while(i < n){
    15          output_var = fib[i];
    16          output();
    17          i = i + 1;
    18      }
    19  }
    20  void main(){
    21      fib();
    22      output_fib();
    23      fib[51] = 0;
    24  }
$ ./irbuilder simple.c 
...
output:1
output:1
output:2
output:3
output:5
output:8
output:13
output:21
output:34
output:55
output:89
output:144
output:233
output:377
output:610
output:987
output:1597
output:2584
output:4181
output:6765
obc Array[fib] [OutBound Check Error] at Line:23, Pos:4
```



## 代码编译和使用

### 编译

修改`CMakeLists.txt`中的`ANTLR_EXECUTABLE`路径，为自己环境中`jar`包所在路径

```
set(ANTLR_EXECUTABLE /home/ucascompile/ucascompile/antlr4/antlr-4.9.1-complete.jar)
```

1. 新建`build`目录`mkdir build`
2. 在`build`目录下使用`cmake .. && make`编译
3. 编译成功后会在`build`目录下生成名为`irbuilder`的可执行文件

### 使用

使用方法：`./irbuilder filepath`或`./astbuilder filepath -d`，当使用`-d`命令时，会输出`debug`信息，`debug`信息需要通过在代码中添加`log`函数输出，可以帮助调试



##   参考资料

- https://llvm.org/docs/LangRef.html
- https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/IR/IRBuilder.h
- [IR API(一)——使用LLVM提供的C接口和IRBuilder来生成LLVM IR(if 和 while 语句)](https://blog.csdn.net/qq_42570601/article/details/107771289)
- [IR API(二)——使用LLVM IR调用C的函数和全局变量](https://blog.csdn.net/qq_42570601/article/details/107958398)
- [IR API(三)——将C/C++中定义的结构体作为LLVM IR中函数的实参](https://blog.csdn.net/qq_42570601/article/details/107979539)
- [IR API(四)——操作IR的字符串、全局变量、全局常量及数组](https://blog.csdn.net/qq_42570601/article/details/108007986)
- [IR API(五)——使用LLVM提供的C接口和IRBuilder来生成LLVM IR常用方法总结](https://blog.csdn.net/qq_42570601/article/details/108059403)
- [IR API(六)——LLVM异常处理(Exception Handling in LLVM)](https://blog.csdn.net/qq_42570601/article/details/109602543)

