parser grammar SafeCParser;
options { tokenVocab = SafeCLexer; }

compUnit: (decl | funcDef) + EOF;

decl: constDecl | varDecl;

funcDef: Void Identifier LeftParen RightParen block;

constDecl: 

bType: Int;

obcArray: 

unobcArray: 

array: 

constDef: 

varDecl: 

varDef: 

block: 

blockItem: decl | stmt;

lval: 

stmt: 

cond: exp (Equal | NonEqual | LessEqual | GreaterEqual | Less | Greater) exp;

number: IntConst;

exp: 

