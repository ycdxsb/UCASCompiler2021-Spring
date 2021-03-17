lexer grammar SafeCLexer;

tokens {
    Comma,
    SemiColon,
    Assign,

    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    LeftParen,
    RightParen,

    If,
    Else,
    While,

    Equal,
    NonEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,

    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,

    Int,
    Void,
    Obc,
    Const,

    Identifier,
    IntConst
}

Comma: 
SemiColon: 
Assign: 

LeftBracket: 
RightBracket: 
LeftBrace: 
RightBrace:
LeftParen: 
RightParen:

If: 
Else:
While:

Equal:
NonEqual: 
Less: 
Greater: 
LessEqual:
GreaterEqual:

Plus:
Minus:
Multiply:
Divide:
Modulo:

Int: 'int';
Void: 'void';
Obc: 'obc';
Const: 'const';

Identifier: [_a-zA-Z][a-zA-Z0-9_]*;
IntConst:

BlockComment :
LineComment : 
WhiteSpace: [ \t\r\n]+ -> skip;



