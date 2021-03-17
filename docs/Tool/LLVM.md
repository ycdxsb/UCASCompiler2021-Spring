## LLVM 简介

> LLVM是Low Level Virtual Machine的简称，是一套编译工具链的集合‌

主要包含以下工具：

- **The LLVM Core libraries** provide a modern source- and target-independent optimizer, along with code generation support for many popular CPUs 
- **Clang**, an "LLVM native" C/C++/Objective-C compiler 
- **compiler-rt**, (consists of sanitizer runtimes: AddressSanitizer…) 
- **LLDB**, a native debugger building on libraries provided by LLVM and Clang 
- **LLD**, a new linker
- **libc++ and libc++ ABI**, a standard conformant and high-performance implementation of the C++ Standard Library
- ......



## LLVM编译链

LLVM工具链通过前后端隔离的方法，将不同语言转化为LLVM中间代码，最后在不同的平台上，将中间代码编译为可执行文件

![image-20210317101919169](https://ycdxsb-1257345996.cos.ap-beijing.myqcloud.com/blog/2021-17-03-image-20210317101919169.png)

LLVM's Implementation of Three-Phase DesignEnter a caption for this image (optional)

对于C语言，使用LLVM工具链从源代码到可执行文件的编译过程如下图所示

![image-20210317101934612](https://ycdxsb-1257345996.cos.ap-beijing.myqcloud.com/blog/2021-17-03-image-20210317101934612.png)

LLVM for C LanguageEnter a caption for this image (optional)‌

## LLVM 源代码结构

- `llvm/examples`：一些使用LLVM的简单示例
- `llvm/include`：使用LLVM需要包含的头文件位置
- `llvm/lib`：LLVM库文件所在位置，包括IR、字节码等库文件
- `llvm/project`：不是LLVM的主要文件，但是和LLVM一起发布，自己构建项目也在该目录下
- `llvm/test`：LLVM的测试代码
- `llvm/tools`：LLVM编译链所需要的文件，例如llvm-ar、llvm-as、llvm-link、lli等
- `llvm/utils`：依赖于LLVM的一些套件，比如codegen-diff等

## Clang 词法语法分析

Clang 是LLVM 编译链中对`C/C++/Obj-C`进行词法、语法解析的工具

### 词法分析

词法分析是编译的第一步，`Lexer`对构成源程序的字符流进行扫描，根据构词规则识别单词(也称单词符号或符号)

使用Clang命令`clang -cc1 -dump-tokens main.c`可以对C语言源文件进行处理，输出对应的token结果

```
/**
    main.c
**/
int fab(int n){
    if(n==1){
        return 1;
    }else if(n==2){
        return 1;
    }else{
        return fab(n-1) + fab(n-2);
    }
}


int main(){
    int result;
    result = fab(10);
    return 0;
}
```



```
int 'int'	 [StartOfLine]	Loc=<main.c:4:1>
identifier 'fab'	 [LeadingSpace]	Loc=<main.c:4:5>
l_paren '('		Loc=<main.c:4:8>
int 'int'		Loc=<main.c:4:9>
identifier 'n'	 [LeadingSpace]	Loc=<main.c:4:13>
r_paren ')'		Loc=<main.c:4:14>
l_brace '{'		Loc=<main.c:4:15>
if 'if'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:5:5>
l_paren '('		Loc=<main.c:5:7>
identifier 'n'		Loc=<main.c:5:8>
equalequal '=='		Loc=<main.c:5:9>
numeric_constant '1'		Loc=<main.c:5:11>
r_paren ')'		Loc=<main.c:5:12>
l_brace '{'		Loc=<main.c:5:13>
return 'return'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:6:9>
numeric_constant '1'	 [LeadingSpace]	Loc=<main.c:6:16>
semi ';'		Loc=<main.c:6:17>
r_brace '}'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:7:5>
else 'else'		Loc=<main.c:7:6>
if 'if'	 [LeadingSpace]	Loc=<main.c:7:11>
l_paren '('		Loc=<main.c:7:13>
identifier 'n'		Loc=<main.c:7:14>
equalequal '=='		Loc=<main.c:7:15>
numeric_constant '2'		Loc=<main.c:7:17>
r_paren ')'		Loc=<main.c:7:18>
l_brace '{'		Loc=<main.c:7:19>
return 'return'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:8:9>
numeric_constant '1'	 [LeadingSpace]	Loc=<main.c:8:16>
semi ';'		Loc=<main.c:8:17>
r_brace '}'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:9:5>
else 'else'		Loc=<main.c:9:6>
l_brace '{'		Loc=<main.c:9:10>
return 'return'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:10:9>
identifier 'fab'	 [LeadingSpace]	Loc=<main.c:10:16>
l_paren '('		Loc=<main.c:10:19>
identifier 'n'		Loc=<main.c:10:20>
minus '-'		Loc=<main.c:10:21>
numeric_constant '1'		Loc=<main.c:10:22>
r_paren ')'		Loc=<main.c:10:23>
plus '+'	 [LeadingSpace]	Loc=<main.c:10:25>
identifier 'fab'	 [LeadingSpace]	Loc=<main.c:10:27>
l_paren '('		Loc=<main.c:10:30>
identifier 'n'		Loc=<main.c:10:31>
minus '-'		Loc=<main.c:10:32>
numeric_constant '2'		Loc=<main.c:10:33>
r_paren ')'		Loc=<main.c:10:34>
semi ';'		Loc=<main.c:10:35>
r_brace '}'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:11:5>
r_brace '}'	 [StartOfLine]	Loc=<main.c:12:1>
int 'int'	 [StartOfLine]	Loc=<main.c:14:1>
identifier 'main'	 [LeadingSpace]	Loc=<main.c:14:5>
l_paren '('		Loc=<main.c:14:9>
r_paren ')'		Loc=<main.c:14:10>
l_brace '{'		Loc=<main.c:14:11>
int 'int'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:15:5>
identifier 'result'	 [LeadingSpace]	Loc=<main.c:15:9>
semi ';'		Loc=<main.c:15:15>
identifier 'result'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:16:5>
equal '='	 [LeadingSpace]	Loc=<main.c:16:12>
identifier 'fab'	 [LeadingSpace]	Loc=<main.c:16:14>
l_paren '('		Loc=<main.c:16:17>
numeric_constant '10'		Loc=<main.c:16:18>
r_paren ')'		Loc=<main.c:16:20>
semi ';'		Loc=<main.c:16:21>
return 'return'	 [StartOfLine] [LeadingSpace]	Loc=<main.c:17:5>
numeric_constant '0'	 [LeadingSpace]	Loc=<main.c:17:12>
semi ';'		Loc=<main.c:17:13>
r_brace '}'	 [StartOfLine]	Loc=<main.c:18:1>
eof ''		Loc=<main.c:18:2>‌
```

### 语法分析

语法分析是编译过程的逻辑阶段。`Parser`在词法分析的基础上将单词序列组合成各类语法短语，如“程序”，“语句”，“表达式”等等。语法分析程序判断源程序在结构上是否正确。源程序的结构由上下文无关文法描述。

使用Clang命令`clang -Xclang -ast-dump -fsyntax-only main.c`可以获得AST形式的语法分析结果

```
TranslationUnitDecl 0x55fb29acefb8 <<invalid sloc>> <invalid sloc>
|-TypedefDecl 0x55fb29acf878 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'
| `-BuiltinType 0x55fb29acf550 '__int128'
|-TypedefDecl 0x55fb29acf8e8 <<invalid sloc>> <invalid sloc> implicit __uint128_t 'unsigned __int128'
| `-BuiltinType 0x55fb29acf570 'unsigned __int128'
|-TypedefDecl 0x55fb29acfbf0 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'
| `-RecordType 0x55fb29acf9c0 'struct __NSConstantString_tag'
|   `-Record 0x55fb29acf940 '__NSConstantString_tag'
|-TypedefDecl 0x55fb29acfc98 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'
| `-PointerType 0x55fb29acfc50 'char *'
|   `-BuiltinType 0x55fb29acf050 'char'
|-TypedefDecl 0x55fb29b0e7d0 <<invalid sloc>> <invalid sloc> implicit __builtin_va_list 'struct __va_list_tag [1]'
| `-ConstantArrayType 0x55fb29acff30 'struct __va_list_tag [1]' 1
|   `-RecordType 0x55fb29acfd70 'struct __va_list_tag'
|     `-Record 0x55fb29acfcf0 '__va_list_tag'
|-FunctionDecl 0x55fb29b0e948 <main.c:4:1, line:12:1> line:4:5 used fab 'int (int)'
| |-ParmVarDecl 0x55fb29b0e870 <col:9, col:13> col:13 used n 'int'
| |-CompoundStmt 0x55fb29b0ee30 <col:15, line:12:1>
| | `-IfStmt 0x55fb29b0ee08 <line:5:5, line:11:5> has_else
| |   |-BinaryOperator 0x55fb29b0ea90 <line:5:8, col:11> 'int' '=='
| |   | |-ImplicitCastExpr 0x55fb29b0ea78 <col:8> 'int' <LValueToRValue>
| |   | | `-DeclRefExpr 0x55fb29b0ea38 <col:8> 'int' lvalue ParmVar 0x55fb29b0e870 'n' 'int'
| |   | `-IntegerLiteral 0x55fb29b0ea58 <col:11> 'int' 1
| |   |-CompoundStmt 0x55fb29b0eae0 <col:13, line:7:5>
| |   | `-ReturnStmt 0x55fb29b0ead0 <line:6:9, col:16>
| |   |   `-IntegerLiteral 0x55fb29b0eab0 <col:16> 'int' 1
| |   `-IfStmt 0x55fb29b0ede0 <line:7:11, line:11:5> has_else
| |     |-BinaryOperator 0x55fb29b0eb50 <line:7:14, col:17> 'int' '=='
| |     | |-ImplicitCastExpr 0x55fb29b0eb38 <col:14> 'int' <LValueToRValue>
| |     | | `-DeclRefExpr 0x55fb29b0eaf8 <col:14> 'int' lvalue ParmVar 0x55fb29b0e870 'n' 'int'
| |     | `-IntegerLiteral 0x55fb29b0eb18 <col:17> 'int' 2
| |     |-CompoundStmt 0x55fb29b0eba0 <col:19, line:9:5>
| |     | `-ReturnStmt 0x55fb29b0eb90 <line:8:9, col:16>
| |     |   `-IntegerLiteral 0x55fb29b0eb70 <col:16> 'int' 1
| |     `-CompoundStmt 0x55fb29b0edc8 <line:9:10, line:11:5>
| |       `-ReturnStmt 0x55fb29b0edb8 <line:10:9, col:34>
| |         `-BinaryOperator 0x55fb29b0ed98 <col:16, col:34> 'int' '+'
| |           |-CallExpr 0x55fb29b0ec98 <col:16, col:23> 'int'
| |           | |-ImplicitCastExpr 0x55fb29b0ec80 <col:16> 'int (*)(int)' <FunctionToPointerDecay>
| |           | | `-DeclRefExpr 0x55fb29b0ebb8 <col:16> 'int (int)' Function 0x55fb29b0e948 'fab' 'int (int)'
| |           | `-BinaryOperator 0x55fb29b0ec30 <col:20, col:22> 'int' '-'
| |           |   |-ImplicitCastExpr 0x55fb29b0ec18 <col:20> 'int' <LValueToRValue>
| |           |   | `-DeclRefExpr 0x55fb29b0ebd8 <col:20> 'int' lvalue ParmVar 0x55fb29b0e870 'n' 'int'
| |           |   `-IntegerLiteral 0x55fb29b0ebf8 <col:22> 'int' 1
| |           `-CallExpr 0x55fb29b0ed70 <col:27, col:34> 'int'
| |             |-ImplicitCastExpr 0x55fb29b0ed58 <col:27> 'int (*)(int)' <FunctionToPointerDecay>
| |             | `-DeclRefExpr 0x55fb29b0ecc0 <col:27> 'int (int)' Function 0x55fb29b0e948 'fab' 'int (int)'
| |             `-BinaryOperator 0x55fb29b0ed38 <col:31, col:33> 'int' '-'
| |               |-ImplicitCastExpr 0x55fb29b0ed20 <col:31> 'int' <LValueToRValue>
| |               | `-DeclRefExpr 0x55fb29b0ece0 <col:31> 'int' lvalue ParmVar 0x55fb29b0e870 'n' 'int'
| |               `-IntegerLiteral 0x55fb29b0ed00 <col:33> 'int' 2
| `-FullComment 0x55fb29b0f1a0 <line:2:1, col:10>
|   `-ParagraphComment 0x55fb29b0f170 <col:1, col:10>
|     `-TextComment 0x55fb29b0f140 <col:1, col:10> Text="    main.c"
`-FunctionDecl 0x55fb29b0eea8 <line:14:1, line:18:1> line:14:5 main 'int ()'
  `-CompoundStmt 0x55fb29b0f0d0 <col:11, line:18:1>
    |-DeclStmt 0x55fb29b0efc8 <line:15:5, col:15>
    | `-VarDecl 0x55fb29b0ef60 <col:5, col:9> col:9 used result 'int'
    |-BinaryOperator 0x55fb29b0f080 <line:16:5, col:20> 'int' '='
    | |-DeclRefExpr 0x55fb29b0efe0 <col:5> 'int' lvalue Var 0x55fb29b0ef60 'result' 'int'
    | `-CallExpr 0x55fb29b0f058 <col:14, col:20> 'int'
    |   |-ImplicitCastExpr 0x55fb29b0f040 <col:14> 'int (*)(int)' <FunctionToPointerDecay>
    |   | `-DeclRefExpr 0x55fb29b0f000 <col:14> 'int (int)' Function 0x55fb29b0e948 'fab' 'int (int)'
    |   `-IntegerLiteral 0x55fb29b0f020 <col:18> 'int' 10
    `-ReturnStmt 0x55fb29b0f0c0 <line:17:5, col:12>
      `-IntegerLiteral 0x55fb29b0f0a0 <col:12> 'int' 0‌
```

## 参考资料‌

- [LLVM每日谈](https://www.zhihu.com/column/llvm-clang)
- [LLVM Getting Started](https://llvm.org/docs/GettingStarted.html#getting-started)
- [Clang AST](https://clang.llvm.org/docs/IntroductionToTheClangAST.html)