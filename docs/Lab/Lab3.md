## 实验任务

1. 熟悉`LLVM IR `表示和 中间代码生成的相关接口`(IRBuilder)`
2. 基于实验二`AstBuilder`结果，补充实现`SafeCIRBuilder`类中的visit函数
3. 熟悉在`IR`生成中对`while , if else`等较复杂语句的处理
4. 对有`obc`属性的数组访问进行越界检查，插入检查代码，在数组访问越界时进行错误处理

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

## 实验框架介绍

在实验二中，我们已经实现了`AstBuilder`类，能够根据`ANTLR4`语法分析的结果，生成对应的抽象语法树。本次实验利用生成的抽象语法树，通过遍历生成的抽象语法树节点，调用`LLVM`的中间代码生成接口，生成对应的中间代码。

### 项目结构

```shell
Lab3
├── cmake
│   ├── antlr4-generator.cmake.in
│   ├── Antlr4Package.md
│   ├── antlr4-runtime.cmake.in
│   ├── ExternalAntlr4Cpp.cmake
│   ├── FindANTLR.cmake
│   └── README.md
├── CMakeLists.txt
├── include
│   ├── AstBuilder.h
│   ├── AstNode.h
│   ├── AstNode_Visitor.h
│   ├── AstSerializer.h
│   └── SafeCIRBuilder.h
├── main.cpp
├── runtime
│   ├── io.cpp
│   ├── io.h
│   ├── runtime.cpp
│   └── runtime.h
├── SafeCLexer.g4
├── SafeCParser.g4
└── src
    └── AstBuilder.cpp
```

- `cmake`：编译依赖的`cmake`文件
- `CMakeLists.txt`：项目编译用的`config`文件
- `AstNode.h/AstNode_Visitor.h`：同`Lab2`
- `AstBuilder.h/AstBuilder.cpp`：`Lab2`实验内容代码
- `SafeCIRBuilder.h`：`SafeCIRBuilder`类实现代码，也是实验需要修改的代码
- `runtime/*`：运行时所需文件
- `*.g4`：`Lab1`中实现的语法文件

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

`LLVM IRBuilder`提供了接口，来帮助生成中间代码

对于下面求fib的源代码

```c
int fib(int n) {
  if (n == 0)
    return 0;
  else if (n == 1)
    return 1;
  else
    return fib(n - 1) + fib(n - 2);
}
int main() {
  int x = 0;
  for (int i = 1; i < 8; ++i) {
    x += fib(i);
  }
  return x;
}
```

#### clang生成IR

我们可以使用`clang`生成对应的`IR`表示：`clang -emit-llvm fib.c -S -o clang_fib.ll`如下

```c
; ModuleID = 'fib.c'
source_filename = "fib.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @fib(i32 %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  %4 = load i32, i32* %3, align 4
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %6, label %7

6:                                                ; preds = %1
  store i32 0, i32* %2, align 4
  br label %19

7:                                                ; preds = %1
  %8 = load i32, i32* %3, align 4
  %9 = icmp eq i32 %8, 1
  br i1 %9, label %10, label %11

10:                                               ; preds = %7
  store i32 1, i32* %2, align 4
  br label %19

11:                                               ; preds = %7
  %12 = load i32, i32* %3, align 4
  %13 = sub nsw i32 %12, 1
  %14 = call i32 @fib(i32 %13)
  %15 = load i32, i32* %3, align 4
  %16 = sub nsw i32 %15, 2
  %17 = call i32 @fib(i32 %16)
  %18 = add nsw i32 %14, %17
  store i32 %18, i32* %2, align 4
  br label %19

19:                                               ; preds = %11, %10, %6
  %20 = load i32, i32* %2, align 4
  ret i32 %20
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* %2, align 4
  store i32 1, i32* %3, align 4
  br label %4

4:                                                ; preds = %12, %0
  %5 = load i32, i32* %3, align 4
  %6 = icmp slt i32 %5, 8
  br i1 %6, label %7, label %15

7:                                                ; preds = %4
  %8 = load i32, i32* %3, align 4
  %9 = call i32 @fib(i32 %8)
  %10 = load i32, i32* %2, align 4
  %11 = add nsw i32 %10, %9
  store i32 %11, i32* %2, align 4
  br label %12

12:                                               ; preds = %7
  %13 = load i32, i32* %3, align 4
  %14 = add nsw i32 %13, 1
  store i32 %14, i32* %3, align 4
  br label %4

15:                                               ; preds = %4
  %16 = load i32, i32* %2, align 4
  ret i32 %16
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}

```

可以使用`lli`运行该IR，然后在shell中使用`echo $?`查看返回值

#### 手写IR

也可以像写汇编代码一样，写IR代码，如下

```asm
define i32 @fib(i32) {
    %2 = alloca i32, align 4 ;申请缓存区，得到指针i32* %2
    %3 = alloca i32, align 4 ;申请缓存区，得到指针i32* %3
    store i32 %0 , i32* %3, align 4 ;将形参 %0 的值放入内存
    %4 = load i32,i32* %3, align 4  ;读取形参到%4
    %5 = icmp eq i32 %4, 0  ; 与0比较是否相等
    br i1 %5, label %br1, label %br2 ;相等则转到标签br1，否则转到br2

    br1:
    ret i32 0   ; n等于0 ，返回0

    br2:
    %6 = load i32,i32* %3, align 4
    %7 = icmp eq i32 %6, 1  ;与1比较
    br i1 %7, label %br3, label %br4

    br3:
    ret i32 1   ; n等于1，返回1

    br4:
    %8 = load i32, i32* %3, align 4
    %9 = sub nsw i32 %8, 1
    %10 = call i32 @fib(i32 %9) ; 求fib(n-1)
    %11 = load i32, i32* %3, align 4
    %12 = sub nsw i32 %11, 2
    %13 = call i32 @fib(i32 %12) ; 求fib(n-2)
    %14 = add nsw i32 %10,%13
    ret i32 %14 ; 返回结果
}

define i32 @main() {
    %1 = alloca i32, align 4 ; 为局部变量 x 分配栈空间
    %2 = alloca i32, align 4 ; 为局部变量 i 分配栈空间
    store i32 0, i32* %1, align 4 ; x = 0
    store i32 1, i32* %2, align 4 ; i = 1
    br label %br1

    br1:
    %3 = load i32,i32* %2, align 4 ; 读取 i 的值
    %4 = icmp slt i32 %3, 8 ; 和8比较，看是否跳出循环
    br i1 %4,label %br2, label %br3 ; 判断成立，跳转到br2，否则跳转到br3

    br2:
    %5 = load i32, i32* %2, align 4 ; 读取 i 的值
    %6 = call i32 @fib(i32 %5)  ; 调用函数fib(i)，获得结果
    %7 = load i32 ,i32* %1, align 4 ; 读取 x 的值
    %8 = add nsw i32 %6, %7 ; x += fib(i)
    store i32 %8, i32* %1, align 4 ; 将结果存入 x 中
    %9 = add nsw i32 %5, 1 ; i = i + 1
    store i32 %9,i32* %2, align 4 ; 将结果存入 i 中
    br label %br1 ; 跳转到循环起始处继续执行

    br3:
    %10 = load i32,i32* %1, align 4 ; 读取 i 的值
    ret i32 %10 ; 返回 i 的值
}
```

#### IRBuilder生成IR

```c++
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <memory>

using namespace llvm;

int main()
{
    LLVMContext context;
    IRBuilder<> builder(context);
    // 新建一个 module，名为fib
    auto module = new Module("fib", context);
    // 将常数转换为 Value 类型
    auto zero = ConstantInt::get(Type::getInt32Ty(context), 0);
    auto one = ConstantInt::get(Type::getInt32Ty(context), 1);
    auto two = ConstantInt::get(Type::getInt32Ty(context), 2);
    auto eight = ConstantInt::get(Type::getInt32Ty(context), 8);

    // 新建函数fib，参数为int类型
    std::vector<Type *> arg;
    arg.push_back(builder.getInt32Ty());
    auto fib = Function::Create(FunctionType::get(Type::getInt32Ty(context), arg, false),
                                GlobalValue::LinkageTypes::ExternalLinkage,
                                "fib", module); //新建函数fib
    // 新建入口点基本块 entry
    auto entry = BasicBlock::Create(context, "entry", fib);
    builder.SetInsertPoint(entry);
    auto r2 = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "r2");
    auto r3 = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "r3");
    Value *r0 = fib->arg_begin();
    builder.CreateStore(r0, r3);
    auto r4 = builder.CreateLoad(Type::getInt32Ty(context), r3, "r4");
    auto r5 = builder.CreateICmpEQ(r4, zero, "r5");

    auto br1 = BasicBlock::Create(context, "br1", fib);
    auto br2 = BasicBlock::Create(context, "br2", fib);

    builder.CreateCondBr(r5, br1, br2);

    //br1
    builder.SetInsertPoint(br1);
    builder.CreateRet(zero);

    //br2
    builder.SetInsertPoint(br2);
    auto r6 = builder.CreateLoad(Type::getInt32Ty(context), r3, "r6");
    auto r7 = builder.CreateICmpEQ(r6, one, "r7");
    auto br3 = BasicBlock::Create(context, "br3", fib);
    auto br4 = BasicBlock::Create(context, "br4", fib);
    builder.CreateCondBr(r7, br3, br4);

    // br3
    builder.SetInsertPoint(br3);
    builder.CreateRet(one);

    // br4
    builder.SetInsertPoint(br4);
    auto r8 = builder.CreateLoad(Type::getInt32Ty(context), r3, "r8");
    auto r9 = builder.CreateNSWSub(r8, one, "r9");
    auto r10 = builder.CreateCall(fib, {r9}, "r10");
    auto r11 = builder.CreateLoad(Type::getInt32Ty(context), r3, "r11");
    auto r12 = builder.CreateNSWSub(r11, two, "r12");
    auto r13 = builder.CreateCall(fib, {r12}, "r13");
    auto r14 = builder.CreateNSWAdd(r10, r13, "r14");
    builder.CreateRet(r14);
    
    //新建函数 main
    auto main = Function::Create(FunctionType::get(Type::getInt32Ty(context), std::vector<Type *>(), false),
                             GlobalValue::LinkageTypes::ExternalLinkage,
                             "main", module); 
    auto mentry = BasicBlock::Create(context, "entry", main);
    builder.SetInsertPoint(mentry);
    auto m1 = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "r1");
    auto m2 = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "r2");
    builder.CreateStore(zero, m1);
    builder.CreateStore(one, m2);
    auto mbr1 = BasicBlock::Create(context, "br1", main);
    builder.CreateBr(mbr1);

    // br1
    builder.SetInsertPoint(mbr1);
    auto m3 = builder.CreateLoad(Type::getInt32Ty(context), m2, "r3");
    auto m4 = builder.CreateICmpSLT(m3, eight, "r4");
    auto mbr2 = BasicBlock::Create(context, "br2", main);
    auto mbr3 = BasicBlock::Create(context, "br3", main);
    builder.CreateCondBr(m4, mbr2, mbr3);
    
    // br2
    builder.SetInsertPoint(mbr2);
    auto m5 = builder.CreateLoad(Type::getInt32Ty(context), m2, "r5");
    auto m6 = builder.CreateCall(fib,{m5},"r6");
    auto m7 = builder.CreateLoad(Type::getInt32Ty(context),m1,"r7"); 
    auto m8 = builder.CreateNSWAdd(m6,m7,"r8");
    builder.CreateStore(m8,m1);
    auto m9 = builder.CreateNSWAdd(m5,one,"r9");
    builder.CreateStore(m9,m2);
    builder.CreateBr(mbr1);

    // br3
    builder.SetInsertPoint(mbr3);
    auto m10 = builder.CreateLoad(Type::getInt32Ty(context),m1,"r10");
    builder.CreateRet(m10);

    module->print(outs(), nullptr);
    return 0;
}
```

使用命令编译该代码，并运行

```
c++ gen_fib.cpp -o gen_fib  `llvm-config --cxxflags --ldflags --libs --system-libs`
```

可以获得生成的IR如下：

```asm
; ModuleID = 'fib'
source_filename = "fib"

define i32 @fib(i32 %0) {
entry:
  %r2 = alloca i32, align 4
  %r3 = alloca i32, align 4
  store i32 %0, i32* %r3, align 4
  %r4 = load i32, i32* %r3, align 4
  %r5 = icmp eq i32 %r4, 0
  br i1 %r5, label %br1, label %br2

br1:                                              ; preds = %entry
  ret i32 0

br2:                                              ; preds = %entry
  %r6 = load i32, i32* %r3, align 4
  %r7 = icmp eq i32 %r6, 1
  br i1 %r7, label %br3, label %br4

br3:                                              ; preds = %br2
  ret i32 1

br4:                                              ; preds = %br2
  %r8 = load i32, i32* %r3, align 4
  %r9 = sub nsw i32 %r8, 1
  %r10 = call i32 @fib(i32 %r9)
  %r11 = load i32, i32* %r3, align 4
  %r12 = sub nsw i32 %r11, 2
  %r13 = call i32 @fib(i32 %r12)
  %r14 = add nsw i32 %r10, %r13
  ret i32 %r14
}

define i32 @main() {
entry:
  %r1 = alloca i32, align 4
  %r2 = alloca i32, align 4
  store i32 0, i32* %r1, align 4
  store i32 1, i32* %r2, align 4
  br label %br1

br1:                                              ; preds = %br2, %entry
  %r3 = load i32, i32* %r2, align 4
  %r4 = icmp slt i32 %r3, 8
  br i1 %r4, label %br2, label %br3

br2:                                              ; preds = %br1
  %r5 = load i32, i32* %r2, align 4
  %r6 = call i32 @fib(i32 %r5)
  %r7 = load i32, i32* %r1, align 4
  %r8 = add nsw i32 %r6, %r7
  store i32 %r8, i32* %r1, align 4
  %r9 = add nsw i32 %r5, 1
  store i32 %r9, i32* %r2, align 4
  br label %br1

br3:                                              ; preds = %br1
  %m10 = load i32, i32* %r1, align 4
  ret i32 %m10
}
```



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



## 实验提交要求

1. 新建`test_cases`文件夹，构造**带有输出**的测试样例`10`个，尽可能的对自己实现的代码进行覆盖性测试，无具体命名格式要求
2. 最终提交时，实验目录如下，**不需要提交build目录**：

```
Lab3
├── cmake
│   ├── antlr4-generator.cmake.in
│   ├── Antlr4Package.md
│   ├── antlr4-runtime.cmake.in
│   ├── ExternalAntlr4Cpp.cmake
│   ├── FindANTLR.cmake
│   └── README.md
├── CMakeLists.txt
├── include
│   ├── AstBuilder.h
│   ├── AstNode.h
│   ├── AstNode_Visitor.h
│   ├── AstSerializer.h
│   └── SafeCIRBuilder.h
├── main.cpp
├── runtime
│   ├── io.cpp
│   ├── io.h
│   ├── runtime.cpp
│   └── runtime.h
├── SafeCLexer.g4
├── SafeCParser.g4
├── src
│   └── AstBuilder.cpp
└── test_cases
```



##   参考资料

- https://llvm.org/docs/LangRef.html
- https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/IR/IRBuilder.h
- [IR API(一)——使用LLVM提供的C接口和IRBuilder来生成LLVM IR(if 和 while 语句)](https://blog.csdn.net/qq_42570601/article/details/107771289)
- [IR API(二)——使用LLVM IR调用C的函数和全局变量](https://blog.csdn.net/qq_42570601/article/details/107958398)
- [IR API(三)——将C/C++中定义的结构体作为LLVM IR中函数的实参](https://blog.csdn.net/qq_42570601/article/details/107979539)
- [IR API(四)——操作IR的字符串、全局变量、全局常量及数组](https://blog.csdn.net/qq_42570601/article/details/108007986)
- [IR API(五)——使用LLVM提供的C接口和IRBuilder来生成LLVM IR常用方法总结](https://blog.csdn.net/qq_42570601/article/details/108059403)
- [IR API(六)——LLVM异常处理(Exception Handling in LLVM)](https://blog.csdn.net/qq_42570601/article/details/109602543)

