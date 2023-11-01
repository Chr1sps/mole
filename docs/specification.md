# Mole programming language specification

## Grammar

Below is an Extended Backus-Nauf Form specification of Mole's grammar. It is
divided into four sections:

- lexer helper rules - helper rules for lexeme production
- lexer rules - rules that specify lexeme production
- intermediate rules - helper rules for transitioning from lexemes to AST
- AST rules - rules that specify AST nodes.

### Lexer helper rules

```ebnf
non_zero = "1" |
           "2" |
           "3" |
           "4" |
           "5" |
           "6" |
           "7" |
           "8" |
           "9";
digit = non_zero | "0";
special_char = "+" |
               "-" |
               "*" |
               "/" |
               "%" |
               "^";
white_space = ? any white space character ?;
alpha = ? any alphabetic utf-8 char ?;
alphanum = ? any alphanumeric utf-8 char ?;
identifier_char = alpha |
                  "_";
```

### Lexer rules

```ebnf
IDENTIFIER = identifier_char, {identifier_char};
INT = digit, {digit};
DOUBLE = INT, ".", INT;

KW_FN = "fn";
KW_EXTERN = "extern";
KW_LET = "let";
KW_RETURN = "return";
KW_MUT = "mut";
KW_CONST = "const";
KW_IF = "if";
KW_ELSE = "else";

TYPE_BOOL = "bool";

TYPE_U8 = "u8";
TYPE_U16 = "u16";
TYPE_U32 = "u32";
TYPE_U64 = "u64";

TYPE_I8 = "i8";
TYPE_I16 = "i16";
TYPE_I32 = "i32";
TYPE_I64 = "i64";

TYPE_F32 = "f32";
TYPE_F64 = "f64";

TYPE_CHAR = "char";

PLUS = "+";
MINUS = "-";
STAR = "*";
SLASH = "/";
PERCENT = "%";
EXP = "^^";

INCREMENT = "++";
DECREMENT = "--";

SHIFT_LEFT = "<<";
SHIFT_RIGHT = ">>";

EQUAL = "==";
NOT_EQUAL = "!=";
LESS = "<";
LESS_EQUAL = "<=";
GREATER = ">";
GREATER_EQUAL = ">=";

BIT_NEG = "~";
AMPERSAND = "&";
BIT_OR = "|";
BIT_XOR = "^";

NEG = "!";
AND = "&&";
OR = "||";

ASSIGN = "=";
ASSIGN_PLUS = "+=";
ASSIGN_MINUS = "-=";
ASSIGN_STAR = "*=";
ASSIGN_SLASH = "/=";
ASSIGN_PERCENT = "%=";
ASSIGN_EXP = "^^=";
ASSIGN_AMPERSAND = "&=";
ASSIGN_BIT_OR = "|=";
ASSIGN_BIT_XOR = "^=";
ASSIGN_BIT_NEG = "~=";
ASSIGN_SHIFT_LEFT = "<<=";
ASSIGN_SHIFT_RIGHT = ">>=";

L_BRACKET = "{";
R_BRACKET = "}";
L_PAREN = "(";
R_PAREN = ")";
L_SQ_BRACKET = "[";
R_SQ_BRACKET = "]";

PLACEHOLDER = "_";
ELLIPSIS = "...";
LAMBDA_ARROW = "=>";

COLON = ":";
COMMA = ",";
SEMICOLON = ";";

END = ? any character (or lack thereof) representing EOF ?;

```

### Intermediate rules

```ebnf
BUILT_IN_TYPE = TYPE_BOOL |
                TYPE_U8 |
                TYPE_U16 |
                TYPE_U32 |
                TYPE_U64 |
                TYPE_I8 |
                TYPE_I16 |
                TYPE_I32 |
                TYPE_I64 |
                TYPE_F32 |
                TYPE_F64 |
                TYPE_CHAR;

TYPE = NEVER_TYPE |
       SIMPLE_TYPE |
       FUNCTION_TYPE;
TYPES = [TYPE, {COMMA, TYPE}];

TYPE_SPECIFIER = COLON, TYPE;
INITIAL_VALUE = ASSIGN, EXPRESSION;

RETURN_TYPE = [LAMBDA_ARROW, TYPE];

TOP_LEVEL_STATEMENT = FUNCTION |
                      EXTERN |
                      VAR_DECL;

ARG = EXPRESSION;
ARGS = [ARG, {COMMA, ARG}];
LAMBDA_ARG = EXPRESSION |
             PLACEHOLDER;
ARG_WITH_ELLIPSIS = ARG, COMMA, ELLIPSIS;
LAMBDA_ARGS = [LAMBDA_ARG, {COMMA, LAMBDA_ARG}, [COMMA, ARG_WITH_ELLIPSIS]];

EXPRESSION = I32_EXPR |
             F64_EXPR |
             BINARY_EXPR |
             UNARY_EXPR |
             VARIABLE_EXPR |
             CALL_EXPR |
             LAMBDA_CALL_EXPR |
             PAREN_EXPR;
CALLABLE_EXPR = VARIABLE_EXPR |
                CALL_EXPR |
                LAMBDA_CALL_EXPR |
                PAREN_EXPR;
NON_FUNC_STMT = RETURN_STMT |
                ASSIGN_STMT |
                VAR_DECL_STMT |
                BLOCK;

PARAMETER = IDENTIFIER, COLON, TYPE;
PARAMS = [PARAMETER, {COMMA, PARAMETER}];

LHS = EXPRESSION;
PAREN_EXPR= L_PAREN, EXPRESSION, R_PAREN;

BINARY_OP = BUILT_IN_BINARY_OP;
UNARY_OP = BUILT_IN_UNARY_OP;
ASSIGN_OP = ASSIGN |
            ASSIGN_PLUS |
            ASSIGN_MINUS |
            ASSIGN_STAR |
            ASSIGN_SLASH |
            ASSIGN_PERCENT |
            ASSIGN_EXP |
            ASSIGN_EXP |
            ASSIGN_BIT_NEG |
            ASSIGN_AMPERSAND |
            ASSIGN_BIT_OR |
            ASSIGN_BIT_XOR |
            ASSIGN_SHIFT_LEFT |
            ASSIGN_SHIFT_RIGHT;

MUT_VAR_DECL = KW_LET, KW_MUT, IDENTIFIER, (TYPE_SPECIFIER | INITIAL_VALUE | (TYPE_SPECIFIER, INITIAL_VALUE)), SEMICOLON;
CONST_VAR_DECL = KW_LET, IDENTIFIER, (TYPE_SPECIFIER), INITIAL_VALUE, SEMICOLON;
```

### AST rules

```ebnf
NEVER_TYPE = NEG;
SIMPLE_TYPE = TYPE_U8 |
              TYPE_U16 |
              TYPE_U32 |
              TYPEU64 |
              TYPE_I8 |
              TYPE_I16 |
              TYPE_I32 |
              TYPE_I64 |
              TYPE_F32 |
              TYPE_F64 |
              TYPE_BOOL |
              TYPE_CHAR;
FUNCTION_TYPE = KW_FN, L_PAREN, TYPES, R_PAREN, RETURN_TYPE;

BUILT_IN_BINARY_OP = EXP |
                     STAR |
                     SLASH |
                     PERCENT |
                     PLUS |
                     MINUS |
                     SHIFT_LEFT |
                     SHIFT_RIGHT |
                     AMPERSAND |
                     BIT_XOR |
                     BIT_OR |
                     EQUAL |
                     NOT_EQUAL |
                     GREATER |
                     GREATER_EQUAL |
                     LESS |
                     LESS_EQUAL |
                     AND |
                     OR;
BUILT_IN_UNARY_OP = INCREMENT |
                    DECREMENT |
                    NEG |
                    BIT_NEG;

I32_EXPR = INT;
F64_EXPR = DOUBLE;
BINARY_EXPR = EXPRESSION, BINARY_OP, EXPRESSION;
UNARY_EXPR = UNARY_OP, EXPRESSION;
VARIABLE_EXPR = IDENTIFIER;
CALL_EXPR = EXPRESSION, L_PAREN, Parameters, R_PAREN;
LAMBDA_CALL_EXPR = EXPRESSION, L_PAREN, (LAMBDA_ARGS | ARG_WITH_ELLIPSIS), R_PAREN;

RETURN_STMT = KW_RETURN, [EXPRESSION], SEMICOLON;
ASSIGN_STMT = IDENTIFIER, ASSIGN, EXPRESSION, SEMICOLON;
EXTERN_STMT = KW_EXTERN, IDENTIFIER, L_PAREN, PARAMS, R_PAREN, RETURN_TYPE, SEMICOLON;
FUNC_DEF_STMT = KW_FN, IDENTIFIER, L_PAREN, PARAMS, R_PAREN, RETURN_TYPE, BLOCK;
VAR_DECL_STMT = MUT_VAR_DECL | CONST_VAR_DECL;

BLOCK = L_BRACKET, {NON_FUNC_STMT}, R_BRACKET;

PROGRAM = {TOP_LEVEL_STATEMENT};
```
