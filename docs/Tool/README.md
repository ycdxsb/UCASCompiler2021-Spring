# 实验相关工具

> 整体实验基于ANTLR4结合LLVM进行实现，因此本章简要介绍一下ANTLR4和LLVM



ANTLR4(前端)：包括词法语法解析，AST树生成。

LLVM(后端)：包括根据前端传入的AST生成LLVM中间代码，最后通过LLVM进行解释执行

