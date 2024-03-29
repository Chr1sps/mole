# Mole programming language specification

## Table of contents

- [Mole programming language specification](#mole-programming-language-specification)
  - [Table of contents](#table-of-contents)
  - [Type system](#type-system)
    - [Supported types](#supported-types)
    - [Type casting](#type-casting)
  - [Operators](#operators)
    - [Expression precedence](#expression-precedence)
    - [Compound assignments](#compound-assignments)
  - [String support](#string-support)
    - [String escape sequences](#string-escape-sequences)
  - [Variable declarations](#variable-declarations)
  - [Functions](#functions)
    - [Constant functions](#constant-functions)
    - [Externing functions](#externing-functions)
  - [Control flow](#control-flow)
    - [`if` statement](#if-statement)
    - [`while` statement](#while-statement)
    - [`match` statement](#match-statement)
  - [Expression statements](#expression-statements)
  - [Comments](#comments)
  - [Grammar](#grammar)
    - [Lexer rules](#lexer-rules)
    - [Parser rules](#parser-rules)
  - [Usage](#usage)
    - [Compiling to object files](#compiling-to-object-files)
    - [Compiling to LLVM bytecode](#compiling-to-llvm-bytecode)
    - [Compiling to LLVM IR](#compiling-to-llvm-ir)
    - [Outputting serialized JSON](#outputting-serialized-json)
    - [Full list of CLI options](#full-list-of-cli-options)
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
|  i32  |  X | ✓ | ✓ | ✓ | X  |
|  f64  |  X | ✓ | ✓ | ✓ | X  |
|  char |  X | ✓ | ✓ | X | ✓  |

Unlike languages such as C++ and JavaScript, Mole has no concept of truthy
values or implicit type conversions in order to emphasise on code clarity.

## Operators

All operators listed below are binary, unless indicated otherwise:

|Operator type|Operators|
|:-:|:-:|
|Arithmetical|`+`, `-`, `*`, `/`, `%`, `^^` unary `-`|
|Comparison|`==`,`!=`,`<`,`<=`,`>=`,`>`|
|Bitwise|`<<`, `>>`, `&`, `\|`, `^`, unary `~`|
|Logical|`&&`,`\|\|`, unary `!`               |
|Cast|`as` (takes type name as a second parameter)|
|Reference|Unary `&`, `&mut`|
|Dereference|Unary `*`|

Different types have different operator support:

|Operator\Type|`bool`|`u32`|`i32`|`f64`|`char`|`str`   |
|:-----------:|:----:|:---:|:---:|:---:|:----:|:------:|
|Arithmetical |X     |  ✓* |✓*   |✓*   |X     |X|
|Comparison   |only `==`,`!=`|✓|✓|✓|✓|X|
|Bitwise      |X|✓**|✓**|X|X|X|
|Logical      |✓|X|X|X|X|X|
|Cast         |✓|✓|✓|✓|✓|X|
|Reference    |✓|✓|✓|✓|✓|X**|

\* The exponentiation operator `^^` requires the exponent to be of type `u32`.

\*\* The shift left `<<` and arithmetic shift right `>>` operators require the
RHS to be of type `u32`.

\*\* Because `str` only exists in the referenced form `&str`, referencing
doesn't occur.

All non-referenced variables can be referenced through the reference operators
`&` and `&mut`. The difference between these operators is such that the first
one doesn't allow mutating the referenced value while the second one does.

All referenced types (except `str`) support dereferencing through the unary `*`
operator.

### Expression precedence

The expressions in Mole are evaluated in the following order:

|   Operator/expression     |Associativity|
|:-------------------------:|:-----------:|
|Parenthesis||
|Lambda calls, function calls, string indexing||
|Unary operators `!`,`~`,`-`,`&`,`&mut`|   |
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

### Compound assignments

The language allows use of the following compound assignments:

|Assignment type|Operators|
|:-:|:-:|
|Arithmetical|`+=`, `-=`, `*=`, `/=`, `%=`, `^^=`|
|Bitwise|`<<=`, `>>=`, `&=`, `\|=`, `^=` (`u32` and `i32`)|
|Logical/Comparison|`&=`, `\|=`, `^=` (`bool`)|

These behave exactly the same as assigning a binary expression of an according
type, with the only difference being the logical assignments, which mimic the
bitwise / comparison operators but work as if assigning a result of a logical /
comparison operator: `||` for `|=`, `&&` for `&=`, `==` for `^=`.

## String support

Mole support an equivalent of `wchar_t` based strings from C++.

### String escape sequences

Shown below is the list of escape sequences used in Mole:

|Sequence|Meaning    |
|----|---------------|
|`\\`|Backslash      |
|`\n`|Newline        |
|`\r`|Carriage return|
|`\'`|Single apostrophe|
|`\"`|Double apostrophe|
|`\0`|Null character|
|`\{NN..}`|Arbitrary UTF-8 character (`NN..` indicates arbitrary-length hexadecimal code)|

## Variable declarations

Declaring a variable makes it immutable by default:

```txt
let variable: u32 = 0;
// this doesn't compile
variable = 1;
```

To make it mutable, we use the `mut` keyword:

```txt
let mut variable: u32 = 0;
// this does compile
variable = 1;
```

Immutable variables require a value upon initialization, otherwise an error is
thrown.

Variable declarations support type inference:

```txt
let variable = 0; // the inferred type is u32
let variable = 0.3; // the inferred type is f64
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
extern isalnum(i32) => i32;
extern exit(i32);
extern getchar() => i32;
```

These functions are by default non-constant, as the language cannot ensure that
they obey the rules of accessing outside variables.

Note that when extending functions we don't need to specify parameter names,
just their types.

## Control flow

Mole has three constructs that define control flow in a program:

- the `if` statement
- the `while` loop
- the `match` statement.

### `if` statement

The `if` statement takes a boolean value in the parentheses and executes the
following statement(s) if the condition evaluates to `true`; the statement
can be followed with the `else` section that executes when the condition is
evaluated as `false`

```txt
// ...

if (condition) {

    do_something()

} else do_something_else()

// ...
```

### `while` statement

The `while` loop takes a boolean value in the parentheses and continues
evaluating the statement(s) associated with the loop as long as the condition
evaluates to `true`

```txt
// ...

while (i > 0) {
    do_this();
    do_that();
}

while (i > 3) do_squat();

// ...
```

When in the while loop, `continue` and `break` statement can be used to either
immediately skip to the next iteration of the loop or stop proccesing and exit
the loop. Note that both of these instructions only work for the innermost loop
currently in the scope:

```txt

while (true) {
    while (true) {
        if (a == 0) 
            break;
        
        a -= 1;
    }
    // this loop still executes after the inner break statement
}

```

### `match` statement

Mole has a simple pattern matching system that works with primitive,
non-referenced types:

```txt
let value = 5;
match value {
    0 => value = 1;                   // matches 0
    1 | 2 => value = 0;               // matches 1 or 2
    if (value > 4) => value = 2;      // matches any value bigger than 4
    else => value = 3;                // matches any value leftover
};
```

The example above shows all 4 possible scenarios of patterns:

- comparing against a single literal
- comparing against multiple literals joined by the `|` sign
- using the `if` guard, which matches if the expression inside evaluates to
  `true`
- matching against all values left with the dummy `else` pattern.

The `if` guard accepts any expression that resolves to a boolean value.

Not covering all the cases in the `match` statement will raise a compilation
warning:

```txt
let value = 3;
// code below throws a warning
match value {
    0 => value = 1;           
    1 | 2 => value = 0;       
};
```

The compiler doesn't check if the `if` guards cover all the cases, so the
code below will also generate a warning:

```txt
let value = 3;
// code below throws a warning
match value {
    if (true) => value = 1;           
};
```

Due to the nature of the pattern matching, the condition in a literal / guard
is evaluated each time the pattern is checked, which means that any function
calls that change state will be executed and may alter the evaluated pattern.

## Expression statements

Expressions can be evaluated in separate statements:

```txt
foo(3,2,1);
a+b;
!c;
```

This is useful for evoking side effects from expression evaluation.

## Comments

Mole support two types of comments:

- single line comments: `// this is a single line comment`
- multi-line comments:

  ```txt
  /*
  this is a multi-line comment
  */
  ```

These comments are lexed but ignored during parsing.

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
comment = single_line_comment | multi_line_comment;

single_line_comment = "//", {? any non-newline character ?}, ? newline character ?;
multi_line_comment = "/*", { non_asterisk | ("*", non_slash) }, "*/";

non_asterisk = ? any non-asterisk character ?;
non_slash= ? any non-slash character ?;

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
white_space = ? any white space character ?;
alpha = ? any alphabetic utf-8 char ?;
alphanum = ? any alphanumeric utf-8 char ?;
identifier_char = alpha |
                  "_";
hex_digit = digit |
            "a" |
            "b" |
            "c" |
            "d" |
            "e" |
            "A" |
            "B" |
            "C" |
            "D" |
            "E";

IDENTIFIER = identifier_char, {identifier_char};
UINT = digit, {digit};
DOUBLE = UINT, ".", UINT;

STRING = '"', {string_char | "'"}, '"';
CHAR = "'", (string_char | '"'), "'";
string_char = ? any character that isn't a backslash, an apostrophe or a double apostrophe? | escaped_char;
escaped_char = "\\" |
               "\n" |
               "\r" |
               "\'" |
               '\"' |
               "\0" |
               ("\{", hex_digit, {hex_digit}, "}");

KW_FN = "fn";
KW_EXTERN = "extern";
KW_LET = "let";
KW_RETURN = "return";
KW_MUT = "mut";
KW_CONST = "const";
KW_IF = "if";
KW_ELSE = "else";
KW_WHILE = "while";
KW_MATCH = "match";
KW_CONTINUE = "continue";
KW_BREAK = "break";
KW_AS = "as";
KW_TRUE = "true";
KW_FALSE = "false";

TYPE_BOOL = "bool";

TYPE_U32 = "u32";
TYPE_I32 = "i32";
TYPE_F64 = "f64";

TYPE_CHAR = "char";
TYPE_STR = "str";

PLUS = "+";
MINUS = "-";
STAR = "*";
SLASH = "/";
PERCENT = "%";
EXP = "^^";

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

LAMBDA_ARROW = "=>";

COLON = ":";
COMMA = ",";
SEMICOLON = ";";

```
<!-- 73 tokens -->
### Parser rules

```ebnf
PROGRAM = {TOP_LEVEL_STATEMENT};

TOP_LEVEL_STATEMENT = FUNC_DEF_STMT |
                      EXTERN_STMT |
                      VAR_DECL_STMT;

EXTERN_STMT = KW_EXTERN, IDENTIFIER, L_PAREN, [TYPES], R_PAREN, [RETURN_TYPE], SEMICOLON;
FUNC_DEF_STMT = KW_FN, [KW_CONST], IDENTIFIER, L_PAREN, [PARAMS], R_PAREN, [RETURN_TYPE], BLOCK;


PARAMS = PARAMETER, {COMMA, PARAMETER};
PARAMETER = IDENTIFIER, TYPE_SPECIFIER;
TYPE_SPECIFIER = COLON, TYPE;

RETURN_TYPE = LAMBDA_ARROW, TYPE;

TYPES = TYPE, {COMMA, TYPE};
TYPE = [REF_SPECIFIER], BUILT_IN_TYPE;
BUILT_IN_TYPE = TYPE_U32 | TYPE_I32 | TYPE_F64 | TYPE_BOOL | TYPE_CHAR | TYPE_STR;
REF_SPECIFIER = AMPERSAND, [KW_MUT];

BLOCK = L_BRACKET, {NON_FUNC_STMT}, R_BRACKET;

NON_FUNC_STMT = RETURN_STMT |
                ASSIGN_OR_EXPR_STMT |
                VAR_DECL_STMT |
                IF_STMT |
                WHILE_STMT |
                MATCH_STMT |
            CONTINUE_STMT |
               BREAK_STMT |
                BLOCK;


BREAK_STMT = KW_BREAK, SEMICOLON;
CONTINUE_STMT = KW_CONTINUE, SEMICOLON;

RETURN_STMT = KW_RETURN, [BINARY_EXPR], SEMICOLON;

ASSIGN_OP = ASSIGN |
            ASSIGN_PLUS |
            ASSIGN_MINUS |
            ASSIGN_STAR |
            ASSIGN_SLASH |
            ASSIGN_PERCENT |
            ASSIGN_EXP |
            ASSIGN_BIT_NEG |
            ASSIGN_AMPERSAND |
            ASSIGN_BIT_OR |
            ASSIGN_BIT_XOR |
            ASSIGN_SHIFT_LEFT |
            ASSIGN_SHIFT_RIGHT;

VAR_DECL_STMT = KW_LET, [KW_MUT], IDENTIFIER, [TYPE_SPECIFIER], [INITIAL_VALUE], SEMICOLON;

INITIAL_VALUE = ASSIGN, BINARY_EXPR;

ASSIGN_OR_EXPR_STMT = BINARY_EXPR, [ASSIGN_PART], SEMICOLON;
ASSIGN_PART = ASSIGN_OP, BINARY_EXPR;

PAREN_EXPR = L_PAREN, BINARY_EXPR, R_PAREN;
FACTOR = PAREN_EXPR |
         U32_EXPR |
         F64_EXPR |
         STRING_EXPR |
         CHAR_EXPR |
         BOOL_EXPR |
         VARIABLE_OR_CALL;


VARIABLE_OR_CALL = VARIABLE_EXPR, [CALL_PART];
CALL_PART = L_PAREN, [ARGS], R_PAREN;

INDEX_EXPR = FACTOR, [INDEX_PART];
INDEX_PART = L_SQ_BRACKET, BINARY_EXPR, R_SQ_BRACKET;

UNARY_EXPR = {UNARY_OP}, INDEX_EXPR;
CAST_EXPR = UNARY_EXPR , {KW_AS, TYPE};
BINARY_EXPR = CAST_EXPR, {BINARY_OP, CAST_EXPR};

VARIABLE_EXPR = IDENTIFIER;
ARGS = BINARY_EXPR, {COMMA, BINARY_EXPR};
U32_EXPR = UINT;
F64_EXPR = DOUBLE;
STRING_EXPR = STRING;
CHAR_EXPR = CHAR;
BOOL_EXPR = KW_TRUE | KW_FALSE;
UNARY_OP = NEG |
           BIT_NEG |
           MINUS |
           REF_SPECIFIER;
BINARY_OP = EXP |
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

IF_STMT = KW_IF, PAREN_EXPR, NON_FUNC_STMT, [ELSE_STMT];
ELSE_STMT = KW_ELSE, NON_FUNC_STMT;

WHILE_STMT = KW_WHILE, PAREN_EXPR, NON_FUNC_STMT;

MATCH_STMT = KW_MATCH, PAREN_EXPR, L_BRACKET, {MATCH_ARM}, R_BRACKET;
MATCH_ARM = LITERAL_ARM | GUARD_ARM | ELSE_ARM;

LITERAL_ARM = LITERAL_CONDITION, MATCH_ARM_STMT;
GUARD_ARM = GUARD_CONDITION, MATCH_ARM_STMT;
ELSE_ARM = KW_ELSE, MATCH_ARM_STMT;

MATCH_ARM_STMT = LAMBDA_ARROW, NON_FUNC_STMT;

LITERAL_CONDITION = UNARY_EXPR, {BIT_OR, UNARY_EXPR};
GUARD_CONDITION = KW_IF, PAREN_EXPR;
```
<!--  54 rules (for now)-->

## Usage

### Compiling to object files

By default, the compiler takes a single file path and returns a compiled object
file `out.o`. The output file can be specified using the `-o <filename>` flag.
The resulting object file can be compiled to an executable.

### Compiling to LLVM bytecode

Passing the `--bc-dump` flag makes the compiler return a file containing LLVM
bytecode (default filename: `out.bc`).

### Compiling to LLVM IR

Passing the `--ir-dump` flag makes the compiler return a file containing LLVM
IR (default filename: `out.ll`).

### Outputting serialized JSON

Passing the `--ast-dump` flag makes the compiler serialize the AST to JSON and
then either print it if no file name is provided or output it to a file
otherwise.

### Full list of CLI options

```txt
USAGE: molec [options] <input file>

OPTIONS:

Generic Options:

  --help        - Display available options (--help-hidden for more)
  --help-list   - Display list of available options (--help-list-hidden for more)
  --version     - Display the version of this program

Mole options:

  --ast-dump    - Dump the abstract syntax tree of the file as a JSON object.
  --bc-dump     - Dump the LLVM bytecode.
  --ir-dump     - Dump the LLVM IR.
  -o <filename> - Specify the output file.
  --optimize    - Optimize the created llvm ir.
```

## Compiler structure

The compiler's components can be split into two categories: the compiler
pipeline elements and other components code. The compiler pipeline components
are:

- `Reader` - provides a common interface for working with any type of input
(files, standard input, etc.) that allows other classes to extract characters
from the source as well as get data about their position in the code; it also
converts the Windows `\r\n` newline sequence to the Unix `\n` newline sequence
- `Lexer` - lazily fetches characters provided by the `Reader` and generates
a sequence of tokens based on the predefined grammar rules
- `Parser` - lazily fetches tokens from the `Lexer` and generates an abstract
syntax tree
- `SemanticChecker` - visits the created AST and checks if it is
semantically correct
- `CompiledProgram` - contains the compiled LLVM module and exposes methods for
outputting the LLVR IR, bytecode, object file or for optimisation; the class
utilizes RAII - the module is created upon object creation from the AST passed
to the class constructor.

Other notable components:

- `JsonSerializer` - serializes the AST to JSON, utilizes the `nlohmann::json`
library
- `LogMessage`, `Logger*` classes, `Reporter` - logging related classes,
the `Logger` base class defines a virtual `log` function that takes a
`LogMessage` object, which contains a message string and an enum indicating the
logging level, the `Reporter` class, meant for inheritance, defines an
interface for classes that facilitates registering multiple loggers and then
logging to them
- `Locale` - a RAII-style class that sets locale in a given scope.

### Error handling

Depending on where in the pipeline an error occured, the compiler may or may
not continue processing the code in order to gather more information and
report more issues. The behaviour is described as follows:

- when an error occurs in the `Reader`, `Lexer`, or the `Parser`, the compiler
will immediately stop processing the code as either there is an issue with the
source file that prevents reading from it or the source contains sequences that
cannot be converted into tokens, which prevents any further code analysis
- when an error occurs in the `SemanticChecker` the compiler will continue
analysing the code and outputting error and warning messages, but will **not**
pass the result to the next pipeline component in line
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
tokens that begin with the same terminal characters (for example: `>`, `>>` and
`>>=`)
- `Parser` - must be able to process both preset token sequences (unit tests)
as well as ones generated by a `Lexer` from source code (integration tests).
Due to the complexity of the AST it is preferred to validate the output using a
custom printing visitor class that can visit the tree and generate an
informative string about the code that can then be checked against a pattern.
As with the `Lexer`, the `Parser` needs to be able to differentiate edge cases
where there are productions beggining with the same sequence of tokens (for
example: variable expressions \[`foo`\] and function calls \[`foo()`\])
- `SemanticChecker` - due to the complexity of defining an AST structure, the
`SemanticChecker` utilizes the previous pipeline components (that have been
tested beforehand) to generate an AST based on source code. Due to the
`SemanticChecker` not generating any intermediate artifacts to be passed onward
when working on an AST, the main subject of tests is checking if the component
correctly asserts the code's semantic validity by checking generated errors in
the `ErrorHandler`
- `CompiledProgram` - this class was only verified using manual acceptance
tests as a part of the entire compiler - the entire pipeline was

Additionally, all the classes must correctly report errors to a `Logger`
(integration tests).

To test the compiler fully, an additional set of integration/acceptance tests
may be performed on the final compiler using example source code files.
