## 实验内容‌

实验主要分为以下三个部分：

1. 根据EBNF文法实现对应的ANTLR4的语法
2. 通过ANTLR4的访问者模式实现抽象语法树的生成
3. 通过ANTLR4的访问者模式生成LLVM的中间代码‌

## 语言简介

Safe C语言是C语言的子集，但是增加了对数组的关键字扩展，以便对数组下标访问越界进行检查，主要具有以下特点：‌

- 在类型系统上，Safe C相对简单，去掉了复杂的指针，只保留了一维数组
- 和C语言一样，有全局变量和局部变量的概念，存在相应作用域
- 函数没有返回值和参数，因此对函数调用的传参通过全局变量进行传递

## EBNF描述

```
// SafeC Grammar
CompUnit    → [ CompUnit ] ( Decl | FuncDef )
Decl        → ConstDecl
            | VarDecl
Obc	    → 'obc'
BType       → 'int'
VarDecl     → BType VarDef { ',' VarDef } ';'
UnObcArray  → Ident '[' Exp ']'
ObcArray    → Obc Ident '[' Exp ']'
Array       → ObcArray | UnObcArray 
VarDef      → Ident
            | Array
            | Ident '=' Exp
            | Array '=' '{' Exp { ',' Exp } '}'
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

## 课外探索

为了尽可能减少同学们的学习负担，目前SafeC语言十分的mini，感兴趣的同学，可以考虑在其基础上加入更多扩展内容(下列排序根据复杂度从低到高排序)：

- 在类型系统上，SafeC只有int一种类型，这对一种语言来说是十分匮乏的，可以在目前的基础上加入对float、char、bool等类型的支持。
- 在结构上，目前只有if else和while语句，可以加入常见的for语句和do while语句
- 目前在函数调用时，并没有参数和返回值，可以考虑加入返回值类型以及函数参数
- 指针是C语言中十分关键的内容，目前SafeC语言缺乏多维指针以及指针相关的运算，可以考虑将指针加入其中。
- 在安全检查上，除了数组访问越界的问题，指针也存在着一些问题，例如未初始化访问、double free等等，因此可以为指针加入一个关键字，在后续实验中，生成LLVM IR时，可以加入相应的安全检查代码
