## ANTLR4简介

**ANTLR**（全名：**AN**other **T**ool for **L**anguage **R**ecognition）是基于`LL(*)`算法实现的语法解析器生成器，使用java语言编写，可用于进行词法分析和语法分析。ANTLR4的文法更加接近于EBNF文法描述，更加通俗易懂，通过编写相应的语法文件(.g4)，可自动生成适应于C++/Python/Java等语言的Lexer和Parser接口，进行编程‌

## EBNF文法

EBNF文法是对BNF文法的扩展，同时也解决了一些BNF文法的问题，感兴趣的同学可阅读下列资料进行了解

- [BNF范式](https://zh.wikipedia.org/wiki/巴科斯-瑙尔范式)
- [EBNF范式](https://zh.wikipedia.org/wiki/扩展巴科斯范式)

ANTLR4所使用的语法规则也十分接近EBNF语法，因此十分简洁易懂。

## ANTLR4工具链

为了了解ANTLR4的相关功能，和学一门新语言一样，我们还是从Hello World开始，下面的代码定义了一个名为Hello的语言，表示所有hello开头的由两个单词组成的句子。

```
grammar Hello;               // 定义文法的名字

s  : 'hello' ID ;            // 匹配关键字hello和标志符
ID : [a-z]+ ;                // 标志符由小写字母组成
WS : [ \t\r\n]+ -> skip ;    // 跳过空格、制表符、回车符和换行符
```

通过ANTLR4命令，我们可以利用ANTLR4自动生成一些接口，包括词法分析接口(HelloLexer.java)，语法分析文件(HelloParser.java)，监听者模式接口(HelloListener.java)

```
$ ANTLR4 Hello.g4
$ ls
HelloBaseListener.java  HelloLexer.interp  HelloListener.java
Hello.g4                HelloLexer.java    HelloParser.java
Hello.interp            HelloLexer.tokens  Hello.tokens
```

然后使用java对接口进行编译，即可得到一个简单的语法词法分析器Hello，通过分析器，可以对一个文件进行解析，常用参数如下：

- -tokens 打印出记号流。
- -tree 以LISP风格的文本形式打印出语法分析树。
- -gui 在对话框中可视化地显示语法分析树。



1. 查看文件的词法分析结果

   ```
   $ cat hello.c
   hello students
   
   $ grun Hello tokens -tokens hello.c
   [@0,0:4='hello',<'hello'>,1:0]
   [@1,6:13='students',<ID>,1:6]
   [@2,15:14='<EOF>',<EOF>,2:0]
   ```

2. 使用gui模式将文件的语法表示出来

   ```
   $ grun Hello r -gui hello.c
   ```

   

   ![image-20210321191602557](https://ycdxsb-1257345996.cos.ap-beijing.myqcloud.com/blog/2021-21-03-image-20210321191602557.png)

3. 使用命令行模式输出语法树

   ```
   $ grun Hello r -tree hello.c
   (r hello students)
   ```



## ANTLR4 分析原理

```
prog
    : assign    // 第一个选项（'|'是选项分隔符）
    | ifstat    // 第二个选项
    | whilestat
    ...
    
assign : ID '=' expr ;    // 匹配赋值语句像"a=5"
```

上面简单的定义了一个prog语言，由赋值语句，if语句，while语句等组成



我们以比较熟悉的赋值语句举例，它的语法如下

```
assign : ID '=' expr ;    // 匹配赋值语句像"a=5"
```

上面定义的意思是，Assign语句由ID,等于号,expr组成，等于号左侧是变量ID，右侧是表达式

ANTLR4会根据我们写的语法规则，生成对应的递归下降语法分析器，是递归下降方法的集合，每一条语法规则都有相对应的分析方法。语法分析器会自顶向下的从根节点开始进行递归分析。

赋值语句对应的分析方法如下：

```
// assign : ID '=' expr ;
void assign() {    // 根据规则assign生成的方法
    match(ID);     // 比较ID和当前输入符号然后消费
    match('=');
    expr();        // 通过调用expr()匹配表达式
}
```

当语法分析器分析到赋值语句时，会先进行对ID的匹配，然后匹配等于号，最后调用expr方法进行对表达式的匹配分析

而prog的分析方法则不同，如下，ANTLR4通过LL(*)算法，使用switch结构进行分析，这是因为当决定使用哪个分析方法时，必须要通过预读入的token决定走哪个分析分支：

```
void prog() {
    switch ( «current input token» ) {
        CASE ID : assign(); break;
        CASE IF : ifstat(); break;    // IF是关键字'if'的记号类型
        CASE WHILE : whilestat(); break;
        ...
        default : «raise no viable alternative exception»
    }
}
```

## ANTLR4二义性处理

一个句子可以有多种解释，称为二义性，是我们在定义语言的时候需要避免的，下面是一些二义性语法的例子

```
assign
    : ID '=' expr    // 匹配一个赋值语句，例如f()
    | ID '=' expr    // 前面选项的精确复制
    ;

expr
    : INT ;
```

虽然在我们人看来两个ID '=' expr是一样的，匹配哪个都可以，但是对于分析器来说，是十分不友好的

```
stat
    : expr          // 表达式语句
    | ID '(' ')'    // 函数调用语句
    ;

expr
    : ID '(' ')'
    | INT
    ;
```

在这里，我们对于f()的解释可以是`stat->expr->ID '(' ')'`，也可以是`stat->ID '(' ')'`，具有歧义

大部分语言都会被设计成非二义文法，如果发现存在二义时，语法分析器必须要进行选择来尽可能消除二义性，ANTLR4通过选择最先匹配到的语法规则来消除二义性

例如我们定义了下列两个规则

```
BEGIN: 'begin';
ID: [a-z]+;
```

当ANTLR4找到begin时，由于内部设定，会选择BEGIN规则进行匹配，而不是ID进行匹配。同时词法分析器也会为每个token进行尽可能长的匹配，因此遇到beginner时，会优先匹配ID，而不是BEGIN。



## 参考资料

- [ANTLR4](https://www.antlr.org/)
- [Antlr简明教程](https://wizardforcel.gitbooks.io/antlr4-short-course/content/)



