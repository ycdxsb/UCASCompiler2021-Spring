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

Comma: ',';
SemiColon: ';';
Assign: '=';

LeftBracket: '[';
RightBracket: ']';
LeftBrace: '{';
RightBrace: '}';
LeftParen: '(';
RightParen: ')';

If: 'if';
Else: 'else';
While: 'while';

Equal: '==';
NonEqual: '!=';
Less: '<';
Greater: '>';
LessEqual: '<=';
GreaterEqual: '>=';

Plus: '+' ;
Minus: '-' ;
Multiply: '*' ;
Divide: '/' ;
Modulo: '%' ;

Int: 'int';
Void: 'void';
Obc: 'obc';
Const: 'const';

Identifier: [_a-zA-Z][a-zA-Z0-9_]*;
IntConst: (('0x'|'0X')[0-9a-fA-F]+)|([1-9][0-9]+)|[0-9];

BlockComment : '/*' .*? '*/' -> skip ;
LineComment : ('/' '\\' .*? '\r'? '\n' ('/' .*? '\r'? '\n')* | '//' (.*? '\\' '\r'? '\n')* .*? '\r'? '\n') -> skip;
WhiteSpace: [ \t\r\n]+ -> skip;



