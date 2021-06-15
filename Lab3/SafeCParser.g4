parser grammar SafeCParser;

options { tokenVocab = SafeCLexer; }

compUnit: (decl | funcDef) + EOF;

decl: constDecl | varDecl;

funcDef: Void Identifier LeftParen RightParen block;

constDecl: Const bType constDef (Comma constDef)* SemiColon;

bType: Int;

obcArray: Obc Identifier LeftBracket (exp)? RightBracket;

unobcArray: Identifier LeftBracket (exp)? RightBracket;

array: obcArray | unobcArray;

constDef: Identifier Assign exp | array Assign LeftBrace exp (Comma exp)* RightBrace;

varDecl: bType varDef (Comma varDef)* SemiColon;

varDef: Identifier | array  | Identifier Assign exp | array Assign LeftBrace exp (Comma exp)* RightBrace;

block: LeftBrace (blockItem)* RightBrace;

blockItem: decl | stmt;

lval: Identifier | Identifier LeftBracket exp RightBracket;

stmt: lval Assign exp SemiColon | block | Identifier LeftParen RightParen SemiColon |If LeftParen cond RightParen stmt (Else stmt)? | While LeftParen cond RightParen stmt | SemiColon ;

cond: exp (Equal | NonEqual | LessEqual | GreaterEqual | Less | Greater) exp;

number: IntConst;

exp: ( Plus | Minus ) exp | exp ( Multiply | Divide | Modulo ) exp | exp ( Plus | Minus ) exp | LeftParen exp RightParen | lval | number;

