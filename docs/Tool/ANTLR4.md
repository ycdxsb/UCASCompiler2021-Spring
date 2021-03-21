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



## 参考资料

- [ANTLR4](https://www.antlr.org/)
- [Antlr简明教程](https://wizardforcel.gitbooks.io/antlr4-short-course/content/)



