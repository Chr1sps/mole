# Mole programming language specification

## Table of contents

- [Mole programming language specification](#mole-programming-language-specification)
  - [Table of contents](#table-of-contents)
  - [Type system](#type-system)
    - [Supported types](#supported-types)
    - [Type casting](#type-casting)
  - [Operators](#operators)
    - [Expression precedence](#expression-precedence)
  - [String support](#string-support)
    - [String escape sequences](#string-escape-sequences)
  - [Control flow](#control-flow)
  - [Functions](#functions)
    - [Constant functions](#constant-functions)
    - [Externing functions](#externing-functions)
    - [In-place lambdas](#in-place-lambdas)
  - [Pattern matching](#pattern-matching)
  - [Comments](#comments)
  - [Grammar](#grammar)
    - [Lexer rules](#lexer-rules)
    - [Parser rules](#parser-rules)
  - [Compiler structure](#compiler-structure)
    - [Error handling](#error-handling)
  - [Testing](#testing)

## Type system

Mole is a strongly, statically typed language with type inference.

### Supported types

The language supports following types:

- `i32` - equivalent of `int` in other languages
- `u32` - unsigned equivalent of `i32`
- `f64` - equivalent of `double` in other languages
- `bool` - boolean type with values `true` and `false`
- `char` - equivalent of `wchar_t` in C++
- `str` - equivalent of an array of `wchar_t` in C++.

It also supports both non-mutable and mutable references (`&` and
`&mut`, respectfully) for aliasing purposes.

Due to fact that the `str` type has string literals that are heap-allocated the
type can *only* exist as a non-mutable reference `&str`.

### Type casting

Type casting can *only* work between primitive, non-referenced types (`&str`
cannot be cast into anything else).

The `as` operator is used to cast values between types.

The following table shows which types can be casted between:

|From\To|bool|u32|i32|f64|char|
|:-----:|:--:|:-:|:-:|:-:|:--:|
|  bool |  ✓ | ✓ | ✓ | ✓ | X  |
|  u32  |  X | ✓ | ✓ | ✓ | ✓  |
|  i32  |  X | ✓ | ✓ | ✓ | ✓  |
|  f64  |  X | ✓ | ✓ | ✓ | X  |
|  char |  X | ✓ | ✓ | X | ✓  |

Unlike languages such as C++ and JavaScript, Mole has no concept of truthy
values or implicit type conversions in order to emphasise on code clarity.

## Operators

All operators listed below are binary, unless indicated otherwise:

|Operator type|Operators|
|:-:|:-:|
|Arithmetical|`+`, `-`, `*`, `/`, `%`, unary `-`, unary `++`, unary `--`|
|Comparison|`==`,`!=`,`<`,`<=`,`>=`,`>`|
|Bitwise|`<<`, `>>`, `&`, `\|`, `^`, unary `~`|
|Logical|`&&`,`\|\|`, unary `!`               |
|Cast|`as` (takes type name as a second parameter)|
|Reference|Unary `&`|

Different types have different operator support:

|Operator\Type|`bool`|`u32`|`i32`|`f64`|`char`|`str`   |
|:-----------:|:----:|:---:|:---:|:---:|:----:|:------:|
|Arithmetical |X     |  ✓  |✓    |✓    |X     |only `+`|
|Comparison   |only `==`,`!=`|✓|✓|✓|✓|only `==`,`!=`|
|Bitwise      |X|✓|✓|X|X|X|
|Logical      |✓|X|X|X|X|X|
|Cast         |✓|✓|✓|✓|✓|✓*|
|Reference    |✓|✓|✓|✓|✓|✓|

\* `str` type has no casts supported, but the casting capability is still
supported

### Expression precedence

The expressions in Mole are evaluated in the following order:

|   Operator/expression     |Associativity|
|:-------------------------:|:-----------:|
|Lambda calls, function calls, string indexing||
|Unary operators `++`,`--`,`!`,`~`,`-`|   |
|Casting operator `as`      |left-to-right|
|`^^`                       |right-to-left|
|`*`,`/`,`%`                |left-to-right|
|`+`,`-`                    |left-to-right|
|`<<`,`>>`                  |left-to-right|
|`&`                        |left-to-right|
|`^`                        |left-to-right|
|`\|`                       |left-to-right|
|`==`,`!=`,`<`,`<=`,`>=`,`>`|left-to-right|
|`&&`                       |left-to-right|
|`\|\|`                     |left-to-right|

## String support

Mole support an equivalent of `wchar_t` based strings from C++, alongside a
basic set of operators:

- `+` for string concatenation (works both with other strings and with `char`s)
- `[N]` for indexing (where `N` indicates an `u32` value)
- `==` and `!=` for comparison.

### String escape sequences

Shown below is the list of escape sequences used in Mole:

|Sequence|Meaning    |
|----|---------------|
|`\\`|Backslash      |
|`\n`|Newline        |
|`\t`|Carriage return|
|`\'`|Single apostrophe|
|`\"`|Double apostrophe|
|`\0`|Null character|
|`\{NN..}`|Arbitrary UTF-8 character (`NN..` indicates 2-,4-,6- or 8-digit hexadecimal code)|

## Control flow

Mole has two constructs that define control flow in a program:

- the `if` statement takes a boolean value in the parentheses and executes the
following statement(s) if the condition evaluates to `true`; the statement can
be followed with the `else` section that executes when the condition is
evaluated as `false`

```txt
// ...

if (condition) {

    // do something if condition is true

} else { 

    // do something if condition is false

}

// ...
```

- the `while` loop takes a boolean value in the parentheses and continues
  evaluating the statement(s) associated with the loop as long as the condition
  evaluates to `true`

  ```txt
  // ...

  let mut a = 0;
  let mut i = 5;

  while (i > 0) {
      a += i;
      --i;
  }


  // ...
  ```

When in the while loop, `continue` and `break` statement can be used to either
immediately skip to the next iteration of the loop or stop proccesing and exit
the loop. Note that both of these instructions only work for the innermost loop
currently in the scope:

```txt

while (true) {
    while (true) {
        if (a == 0) {
            break;
        }
        --a;
    }
    // this loop still executes after the inner break statement
}

```

## Functions

Arguments in functions can be passed by value or by reference (either
constant or non-mutable):

```txt
fn foo(a: i32, b: i32) => i32 {
    return a + b;
}

fn boo(a: &mut i32, b: &i32) {
    a += b;
}
```

Functions can then be called as follows:

```txt
foo(32,32);
let mut a = 0;
let b = 3;
boo(a, b);
```

### Constant functions

*Constant functions* are a feature that allow a programmer to signify that a
function doesn't change the outside state.

When a function is marked `const`, the compiler will check if there are any
reads/writes to a non-constant variable outside of the function scope (but
**not** taking into account passed parameters, as these are handled through
parameter type declarations) and throw an error when such a situation occurs.

```txt
// this compiles
fn const foo(a: i32, b: i32) {
    return a + b;
}

// this compiles - only the `a` parameter changes, not the outside state
fn const foo(a: &mut i32, b: i32) {
    a += b;
}

let mut some_value = 0;

// this compiles
fn boo() {
    some_value = 1;
}

// this does NOT compile - `some_value` is changed
// despite the function being constant
fn const boo() {
    some_value = 2;
}
```

### Externing functions

The language provides a syntax for importing external functions from other
languages, such as C or C++:

```txt
extern isalnum(ch: i32) => i32;
extern exit(exit_code: i32);
extern getchar() => i32;
```

These functions are by default non-constant, as the language cannot ensure that
they obey the rules of accessing outside variables.

### In-place lambdas

*In-place lambdas* are inpired by function currying from functional
programming. It's a syntactic sugar of partially applying function arguments.

```txt
fn foo(a: i32, b: i32, c: i32, d: i32) => i32 {
    return a+b+c+d;
}

fn main() {
    let a: fn(i32,i32,i32) => i32 = foo(1, ...);
    let b: fn(i32) => i32 = foo(1, _, 3, 4);

    // useful for delaying function execution, for example in a button handler
    let c: fn() => i32 = foo(1, 2, 3, 4, ...); 

    //both styles can be mixed...
    let d: fn(i32,i32,i32) => i32 = foo(_, 2, ...); 

    //...provided the `_` token isn't next to the `...` token
    // this doesn't work
    let e = foo(1, _, ...);
}
```

## Pattern matching

Mole has a simple pattern matching system that works with primitive,
non-referenced types:

```txt
let value = 5;
match value {
    0 => {value = 1;}           // matches 0
    1 | 2 => {value = 0;}       // matches 1 or 2
    _ => {value = 3;}           // matches any value leftover
};
```

The example above shows all 3 possible scenarios of patterns:

- comparing against a single literal
- comparing against multiple literals joined by the `|` sign
- matching against all values left with the dummy `_` pattern.

Pattern matching only works against compile-time literals and expressions that
are based on them.

Not covering all the cases in the `match` statement will raise a warning from
the compiler. If a value matched against doesn't match any of the cases in that
case the behaviour is undefined:

```txt
let value = 3;
// code below is undefined behaviour
match value {
    0 => {value = 1;}           
    1 | 2 => {value = 0;}       
    value < 0 => {value = 2;}   
};
```

## Comments

Mole support two types of comments:

- single line comments: `// this is a single line comment`
- multi-line comments:

  ```txt
  /*
  this is a multi-line comment
  */
  ```

These comments are ignored by the compiler during the lexing phase.

## Grammar

Below is an Extended Backus-Nauf Form specification of Mole's grammar. It is
divided into two sections:

- lexer rules - rules that specify lexeme productions
- parser rules - rules that specify AST nodes.

Because the lexer omits all the comments and doesn't generate any tokens
related to them, the parser rules assume that comments are treated as
whitespace.

### Lexer rules

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
escaped_char = "\n" |
               "\r" |

```

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
KW_WHILE = "while";
KW_CONTINUE = "continue";
KW_BREAK = "break";
KW_AS = "as";
KW_STR = "str";

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

### Parser rules

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

TOP_LEVEL_STATEMENT = FUNC_DEF_STMT |
                      EXTERN_STMT |
                      VAR_DECL_STMT;

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

## Compiler structure

The compiler consists of two parallel threads. The main thread is responsible
for outputting error information to the user, while the joined thread contains
the entire compiler pipeline. The pipeline sends generated warnings and errors
to the main thread through a synchonized queue.

Compiler pipeline components:

- `Reader` - provides a common interface for working with any type of input
(files, standard input, etc.) that allows other classes to extract characters
from the source as well as get data about their position in the code
- `Lexer` - lazily fetches characters provided by the `Reader` and generates
a sequence of tokens based on the predefined grammar rules
- `Parser` - lazily fetches tokens from the `Lexer` and generates an abstract
syntax tree
- `SemanticChecker` - visits the created AST and checks if it is
semantically correct
- `CodeGenerator` - visits the now syntactically and semantically correct AST
and generates LLVM IR.

Other components:

- `ErrorHandler` - runs in the main thread of the compiler and reports all the
warnings and errors raised by the pipeline.

### Error handling

Depending on where in the pipeline an error occured, the compiler may or may
not continue processing the code in order to gather more information and
report more issues. The behaviour is described as follows:

- when an error occurs in the `Reader` or the `Lexer`, the compiler will
immediately stop processing the code as either there is an issue with the
source file that prevents reading from it or the source contains sequences that
cannot be converted into tokens, which prevents any further code analysis
- when an error occurs in the `Parser` or the `SemanticChecker` the compiler
will continue analysing the code and outputting error messages, but will
**not** pass the result to the next pipeline component in line
- when an error occurs in the `CodeGenerator` the compiler will immediately
stop as this indicates issues with code generation that may cause the output to
be corrupted.

## Testing

Each of the compiler pipeline components shall be tested both using unit tests
and integration tests:

- `Reader` - must provide capabilities of character and position extraction
(unit tests)
- `Lexer` - must be able to process an input provided by a `Reader` into tokens
(integration tests). In particular, it must be able to correctly differentiate
tokens that begin with the same terminal characters (for example: `-`, `--` and
`-=`)
- `Parser` - must be able to process both preset token sequences (unit tests)
as well as ones generated by a `Lexer` from source code (integration tests).
Due to the complexity of the AST it is preferred to validate the output using a
custom printing visitor class that can visit the tree and generate an
informative string about the code that can then be checked against a pattern.
As with the `Lexer`, the `Parser` needs to be able to differentiate edge cases
where there are productions beggining with the same sequence of tokens (for
example: variable expressions \[`foo`\] and function calls \[`foo()`\])
- `SemanticChecker` - due to the complexity of defining an AST structure, the
`SemanticChecker` shall utilize the previous pipeline components (that have
presumably been tested beforehand) to generate an AST based on source code.
Due to the `SemanticChecker` not generating any intermediate artifacts to be
passed onward when working on an AST, the main subject of tests is checking if
the component correctly asserts the code's semantic validity by checking
generated errors in the `ErrorHandler`

Additionally, all the classes must correctly report errors to an `ErrorHandler`
(integration tests).

To test the compiler fully, an additional set of integration/acceptance tests
may be performed on the final compiler using example source code files.
