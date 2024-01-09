#include "lexer.hpp"
#include "locale.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "semantic_checker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

enum class Result
{
    NONE,
    WARNING,
    ERROR,
    WARNING_AND_ERROR
};

bool check_errors(const std::wstring &source)
{
    auto locale = Locale("C.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto program = parser.parse();
    auto checker = SemanticChecker();
    auto logger = DebugLogger();
    checker.add_logger(&logger);
    checker.verify(*program);
    return !logger.contains_errors();
}

bool check_errors_and_warnings(const std::wstring &source)
{
    auto locale = Locale("C.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto program = parser.parse();
    auto checker = SemanticChecker();
    auto logger = DebugLogger();
    checker.add_logger(&logger);
    checker.verify(*program);
    return !logger.contains_errors() && logger.contains_warnings();
}

#define CHECK_VALID(source) REQUIRE(check_errors(source))
#define CHECK_VALID_WITH_WARNINGS(source)                                     \
    REQUIRE(check_errors_and_warnings(source))
#define CHECK_INVALID(source) REQUIRE_FALSE(check_errors(source))
#define FN_WRAP(source) L"fn wrap(){" + std::wstring(source) + L"}"

TEST_CASE("Variable has no value or type assigned.")
{
    CHECK_INVALID(L"let var;");
    CHECK_INVALID(L"let mut var;");
}

TEST_CASE("Variable's declared type and assigned value's type don't match")
{
    SECTION("No type specifiers.")
    {
        CHECK_VALID(L"let var = 1;");
        CHECK_VALID(L"let var = 1.3;");
        CHECK_VALID(L"let var = true;");
        CHECK_VALID(L"let var = 'a';");
        CHECK_VALID(L"let var = \"a\";");
    }

    SECTION("Globals, normal types.")
    {
        CHECK_VALID(L"let var: u32 = 1;");
        CHECK_VALID(L"let var: f64 = 0.1;");
        CHECK_VALID(L"let var: bool = false;");
        CHECK_VALID(L"let var: char = 'a';");
        CHECK_INVALID(L"let var: u32 = 0.1;");
        CHECK_INVALID(L"let var: f64 = 1;");
        CHECK_INVALID(L"let var: bool = 'a';");
    }

    SECTION("References.")
    {
        CHECK_INVALID(L"let var: &u32 = 1;");
        CHECK_VALID(L"let var: f64 = 0.1;");
        CHECK_VALID(L"let var: bool = false;");
        CHECK_VALID(L"let var: char = 'a';");
        CHECK_INVALID(L"let var: u32 = 0.1;");
        CHECK_INVALID(L"let var: f64 = 1;");
    }

    SECTION("String.")
    {
        CHECK_VALID(L"let var: &str = \"a\";");
        CHECK_INVALID(L"let var: &mut str = \"a\";");
        CHECK_INVALID(L"let var: str = \"a\";");
    }
    SECTION("Inside a function")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 0.1;"));
        CHECK_INVALID(FN_WRAP(L"let var: u32 = 0.1;"));
        CHECK_INVALID(FN_WRAP(L"let var: f64 = 1;"));
    }
}

TEST_CASE("Unary expressions.")
{
    SECTION("On bool.")
    {
        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"-var;"));
        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"~var;"));
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"!var;"));
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"&var;"));
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"&mut var;"));
        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"*var;"));
    }
    SECTION("On u32.")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"-var;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"~var;"));
        CHECK_INVALID(FN_WRAP(L"let var: u32 = 1;"
                              L"!var;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"&var;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"&mut var;"));
        CHECK_INVALID(FN_WRAP(L"let var: u32 = 1;"
                              L"*var;"));
    }
    SECTION("On i32.")
    {
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"-var;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"~var;"));
        CHECK_INVALID(FN_WRAP(L"let var: i32 = -1;"
                              L"!var;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"&var;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"&mut var;"));
        CHECK_INVALID(FN_WRAP(L"let var: i32 = -1;"
                              L"*var;"));
    }
    SECTION("On f64.")
    {
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0;"
                            L"-var;"));
        CHECK_INVALID(FN_WRAP(L"let var: f64 = 1.0;"
                              L"~var;"));
        CHECK_INVALID(FN_WRAP(L"let var: f64 = 1.0;"
                              L"!var;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0;"
                            L"&var;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0;"
                            L"&mut var;"));
        CHECK_INVALID(FN_WRAP(L"let var: f64 = 1.0;"
                              L"*var;"));
    }
    SECTION("On char.")
    {
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"-var;"));
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"~var;"));
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"!var;"));
        CHECK_VALID(FN_WRAP(L"let var: char = 'a';"
                            L"&var;"));
        CHECK_VALID(FN_WRAP(L"let var: char = 'a';"
                            L"&mut var;"));
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"*var;"));
    }
    SECTION("On string.")
    {
        CHECK_INVALID(FN_WRAP(L"let var= \"a\";"
                              L"-var;"));
        CHECK_INVALID(FN_WRAP(L"let var= \"a\";"
                              L"~var;"));
        CHECK_INVALID(FN_WRAP(L"let var= \"a\";"
                              L"!var;"));
        CHECK_INVALID(FN_WRAP(L"let var= \"a\";"
                              L"&var;"));
        CHECK_INVALID(FN_WRAP(L"let var= \"a\";"
                              L"&mut var;"));
        CHECK_INVALID(FN_WRAP(L"let var= \"a\";"
                              L"*var;"));
    }
    SECTION("References.")
    {
        SECTION("Referencing non-lvalues.")
        {
            CHECK_INVALID(FN_WRAP(L"&false;"));
            CHECK_INVALID(FN_WRAP(L"&mut false;"));
            CHECK_INVALID(FN_WRAP(L"&3;"));
            CHECK_INVALID(FN_WRAP(L"&mut 3;"));
            CHECK_INVALID(FN_WRAP(L"&3.0;"));
            CHECK_INVALID(FN_WRAP(L"&mut 3.0;"));
            CHECK_INVALID(FN_WRAP(L"&(-3);"));
            CHECK_INVALID(FN_WRAP(L"&mut (-3);"));
            CHECK_INVALID(FN_WRAP(L"&'a';"));
            CHECK_INVALID(FN_WRAP(L"&mut 'a';"));
            CHECK_INVALID(FN_WRAP(L"&\"a\";"));
            CHECK_INVALID(FN_WRAP(L"&mut \"a\";"));
        }
        SECTION("Dereferencing non-lvalues.")
        {
            CHECK_INVALID(FN_WRAP(L"*false;"));
            CHECK_INVALID(FN_WRAP(L"*3;"));
            CHECK_INVALID(FN_WRAP(L"*3.0;"));
            CHECK_INVALID(FN_WRAP(L"*(-3);"));
            CHECK_INVALID(FN_WRAP(L"*'a';"));
            CHECK_INVALID(FN_WRAP(L"*\"a\";"));
        }
    }
}

TEST_CASE("Binary expressions.")
{
    SECTION("Addition (and concatenation).")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1+1;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)+(-1);"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0+1.0;"));
        CHECK_VALID(FN_WRAP(L"let var: &str = \"a\"+\"b\";"));
        CHECK_VALID(FN_WRAP(L"let var: &str = \"a\"+\'b\';"));
        CHECK_INVALID(FN_WRAP(L"true+false;"));
        CHECK_INVALID(FN_WRAP(L"1+(-1);"));
        CHECK_INVALID(FN_WRAP(L"1+1.0;"));
        CHECK_INVALID(FN_WRAP(L"1.0+(-1);"));
        CHECK_INVALID(FN_WRAP(L"\'a\'+\'b\';"));
        CHECK_INVALID(FN_WRAP(L"\'a\'+\"b\";"));
    }

    SECTION("Substraction.")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1-1;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)-(-1);"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0-1.0;"));
        CHECK_INVALID(FN_WRAP(L"\"a\"-\"b\";"));
        CHECK_INVALID(FN_WRAP(L"\"a\"-\'b\';"));
        CHECK_INVALID(FN_WRAP(L"true-false;"));
        CHECK_INVALID(FN_WRAP(L"1-(-1);"));
        CHECK_INVALID(FN_WRAP(L"1-1.0;"));
        CHECK_INVALID(FN_WRAP(L"1.0-(-1);"));
        CHECK_INVALID(FN_WRAP(L"\'a\'-\'b\';"));
        CHECK_INVALID(FN_WRAP(L"\'a\'-\"b\";"));
    }

    SECTION("Multiplication.")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1*1;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)*(-1);"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0*1.0;"));
        CHECK_INVALID(FN_WRAP(L"\"a\"*\"b\";"));
        CHECK_INVALID(FN_WRAP(L"\"a\"*\'b\';"));
        CHECK_INVALID(FN_WRAP(L"true*false;"));
        CHECK_INVALID(FN_WRAP(L"1*(-1);"));
        CHECK_INVALID(FN_WRAP(L"1*1.0;"));
        CHECK_INVALID(FN_WRAP(L"1.0*(-1);"));
        CHECK_INVALID(FN_WRAP(L"\'a\'*\'b\';"));
        CHECK_INVALID(FN_WRAP(L"\'a\'*\"b\";"));
    }

    SECTION("Division.")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1/1;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)/(-1);"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0/1.0;"));
        CHECK_INVALID(FN_WRAP(L"\"a\"/\"b\";"));
        CHECK_INVALID(FN_WRAP(L"\"a\"/\'b\';"));
        CHECK_INVALID(FN_WRAP(L"true/false;"));
        CHECK_INVALID(FN_WRAP(L"1/(-1);"));
        CHECK_INVALID(FN_WRAP(L"1/1.0;"));
        CHECK_INVALID(FN_WRAP(L"1.0/(-1);"));
        CHECK_INVALID(FN_WRAP(L"\'a\'/\'b\';"));
        CHECK_INVALID(FN_WRAP(L"\'a\'/\"b\";"));
    }

    SECTION("Modulus.")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1%1;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)%(-1);"));
        CHECK_INVALID(FN_WRAP(L"let var: i32 = 1%(-1);"));
        CHECK_INVALID(FN_WRAP(L"let var: i32 = (-1)%1;"));
        CHECK_INVALID(FN_WRAP(L"1.0%1.0;"));
        CHECK_INVALID(FN_WRAP(L"1.0%1;"));
        CHECK_INVALID(FN_WRAP(L"1%1.0;"));
        CHECK_INVALID(FN_WRAP(L"\"a\"%\"b\";"));
        CHECK_INVALID(FN_WRAP(L"\"a\"%\'b\';"));
        CHECK_INVALID(FN_WRAP(L"true%false;"));
        CHECK_INVALID(FN_WRAP(L"1%(-1);"));
        CHECK_INVALID(FN_WRAP(L"1%1.0;"));
        CHECK_INVALID(FN_WRAP(L"1.0%(-1);"));
        CHECK_INVALID(FN_WRAP(L"\'a\'%\'b\';"));
        CHECK_INVALID(FN_WRAP(L"\'a\'%\"b\";"));
    }
    SECTION("Exponentiation.")
    {
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1^^1;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)^^1;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0^^1;"));
        CHECK_INVALID(FN_WRAP(L"(-1)^^(-1);"));
        CHECK_INVALID(FN_WRAP(L"1.0^^1.0;"));
    }
    SECTION("Equations.")
    {
        SECTION("Booleans.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = true==true;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = true!=true;"));
            CHECK_INVALID(FN_WRAP(L"true>=true;"));
            CHECK_INVALID(FN_WRAP(L"true<=true;"));
            CHECK_INVALID(FN_WRAP(L"true>true;"));
            CHECK_INVALID(FN_WRAP(L"true<true;"));
        }
        SECTION("u32s.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = 1==1;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1!=1;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1>=1;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1<=1;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1>1;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1<1;"));
        }
        SECTION("i32s.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = (-1)==(-1);"));
            CHECK_VALID(FN_WRAP(L"let var: bool = (-1)!=(-1);"));
            CHECK_VALID(FN_WRAP(L"let var: bool = (-1)>=(-1);"));
            CHECK_VALID(FN_WRAP(L"let var: bool = (-1)<=(-1);"));
            CHECK_VALID(FN_WRAP(L"let var: bool = (-1)>(-1);"));
            CHECK_VALID(FN_WRAP(L"let var: bool = (-1)<(-1);"));
        }
        SECTION("f64s.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = 1.0==1.0;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1.0!=1.0;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1.0>=1.0;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1.0<=1.0;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1.0>1.0;"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 1.0<1.0;"));
        }
        SECTION("Chars.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = 'a'=='a';"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 'a'!='a';"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 'a'>='a';"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 'a'<='a';"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 'a'>'a';"));
            CHECK_VALID(FN_WRAP(L"let var: bool = 'a'<'a';"));
        }
        SECTION("Strings.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = \"a\"==\"a\";"));
            CHECK_VALID(FN_WRAP(L"let var: bool = \"a\"!=\"a\";"));
            CHECK_INVALID(FN_WRAP(L"\"a\">=\"a\";"));
            CHECK_INVALID(FN_WRAP(L"\"a\"<=\"a\";"));
            CHECK_INVALID(FN_WRAP(L"\"a\">\"a\";"));
            CHECK_INVALID(FN_WRAP(L"\"a\"<\"a\";"));
        }
    }
    SECTION("Bitwise.")
    {
        SECTION("Alternative.")
        {
            CHECK_VALID(FN_WRAP(L"let var: u32 = 1|0;"));
            CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)|(-1);"));
            CHECK_INVALID(FN_WRAP(L"true|false;"));
            CHECK_INVALID(FN_WRAP(L"1.0|0.0;"));
            CHECK_INVALID(FN_WRAP(L"'a'|'a';"));
            CHECK_INVALID(FN_WRAP(L"\"a\"|\"a\";"));
        }
        SECTION("Conjugation.")
        {
            CHECK_VALID(FN_WRAP(L"let var: u32 = 1&0;"));
            CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)&(-1);"));
            CHECK_INVALID(FN_WRAP(L"true&false;"));
            CHECK_INVALID(FN_WRAP(L"1.0&0.0;"));
            CHECK_INVALID(FN_WRAP(L"'a'&'a';"));
            CHECK_INVALID(FN_WRAP(L"\"a\"&\"a\";"));
        }
        SECTION("XOR.")
        {
            CHECK_VALID(FN_WRAP(L"let var: u32 = 1^0;"));
            CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)^(-1);"));
            CHECK_INVALID(FN_WRAP(L"true^false;"));
            CHECK_INVALID(FN_WRAP(L"1.0^0.0;"));
            CHECK_INVALID(FN_WRAP(L"'a'^'a';"));
            CHECK_INVALID(FN_WRAP(L"\"a\"^\"a\";"));
        }
        SECTION("Shifts.")
        {
            CHECK_VALID(FN_WRAP(L"let var: u32 = 1>>1;"));
            CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)>>1;"));
            CHECK_INVALID(FN_WRAP(L"1.0>>1;"));
            CHECK_INVALID(FN_WRAP(L"'a'>>1;"));
            CHECK_INVALID(FN_WRAP(L"\"a\">>1;"));
            CHECK_INVALID(FN_WRAP(L"1>>(-1);"));
            CHECK_INVALID(FN_WRAP(L"1>>1.0;"));

            CHECK_VALID(FN_WRAP(L"let var: u32 = 1<<1;"));
            CHECK_VALID(FN_WRAP(L"let var: i32 = (-1)<<1;"));
            CHECK_INVALID(FN_WRAP(L"1.0<<1;"));
            CHECK_INVALID(FN_WRAP(L"'a'<<1;"));
            CHECK_INVALID(FN_WRAP(L"\"a\"<<1;"));
            CHECK_INVALID(FN_WRAP(L"1<<(-1);"));
            CHECK_INVALID(FN_WRAP(L"1<<1.0;"));
        }
    }
    SECTION("Logical.")
    {
        SECTION("Alternative.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = true || false;"));
            CHECK_INVALID(FN_WRAP(L"1||0;"));
            CHECK_INVALID(FN_WRAP(L"1.0||0.0;"));
            CHECK_INVALID(FN_WRAP(L"(-1)||(-1);"));
            CHECK_INVALID(FN_WRAP(L"'a'||'a';"));
            CHECK_INVALID(FN_WRAP(L"\"a\"||\"a\";"));
        }
        SECTION("Conjugation.")
        {
            CHECK_VALID(FN_WRAP(L"let var: bool = true && false;"));
            CHECK_INVALID(FN_WRAP(L"1&&0;"));
            CHECK_INVALID(FN_WRAP(L"1.0&&0.0;"));
            CHECK_INVALID(FN_WRAP(L"(-1)&&(-1);"));
            CHECK_INVALID(FN_WRAP(L"'a'&&'a';"));
            CHECK_INVALID(FN_WRAP(L"\"a\"&&\"a\";"));
        }
    }
}

TEST_CASE("Function calls.")
{
    SECTION("Valid calls.")
    {
        CHECK_VALID(L"fn test(){}"
                    L"fn main(){"
                    L"  test();"
                    L"}");
        CHECK_VALID(L"fn test(){}"
                    L"fn main(){"
                    L"  let ptr = test;"
                    L"  ptr();"
                    L"}");
    }
    SECTION("Mismatched argument count.")
    {
        CHECK_INVALID(L"fn test(){}"
                      L"fn main(){"
                      L"  test(1);"
                      L"}");
        CHECK_INVALID(L"fn test(a: i32){}"
                      L"fn main(){"
                      L"  test();"
                      L"}");
    }
    SECTION("Mismatched argument type.")
    {
        CHECK_INVALID(L"fn test(a: i32){}"
                      L"fn main(){"
                      L"  test(1.0);"
                      L"}");
    }
}

TEST_CASE("Lambda calls.")
{
    SECTION("Valid calls.")
    {
        SECTION("Direct.")
        {
            CHECK_VALID(L"fn test(){}"
                        L"fn main(){"
                        L"  test@();"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  test@(_, _);"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  test@(1, _);"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  test@(_, 2);"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  test@(1, 2);"
                        L"}");
        }
        SECTION("Through a reference.")
        {
            CHECK_VALID(L"fn test(){}"
                        L"fn main(){"
                        L"  let ptr = test;"
                        L"  ptr@();"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  let ptr = test;"
                        L"  ptr@(_, _);"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  let ptr = test;"
                        L"  ptr@(1, _);"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  let ptr = test;"
                        L"  ptr@(_, 2);"
                        L"}");
            CHECK_VALID(L"fn test(a: u32, b: u32){}"
                        L"fn main(){"
                        L"  let ptr = test;"
                        L"  ptr@(1, 2);"
                        L"}");
        }
    }
    SECTION("Value is not callable.")
    {
        CHECK_INVALID(FN_WRAP(L"let test = 1;"
                              L"test@(1, 2);"));
    }
    SECTION("Mismatched argument count.")
    {
        CHECK_INVALID(L"fn test(){}"
                      L"fn main(){"
                      L"  test@(1);"
                      L"}");
        CHECK_INVALID(L"fn test(){}"
                      L"fn main(){"
                      L"  test@(1, _);"
                      L"}");
        CHECK_INVALID(L"fn test(a: u32, b: u32){}"
                      L"fn main(){"
                      L"  test@(1);"
                      L"}");
        CHECK_INVALID(L"fn test(a: u32, b: u32){}"
                      L"fn main(){"
                      L"  test@(1, 2, _);"
                      L"}");
        CHECK_INVALID(L"fn test(a: u32, b: u32){}"
                      L"fn main(){"
                      L"  test@(_, 2, 3);"
                      L"}");
    }
    SECTION("Mismatched argument type.")
    {
        CHECK_INVALID(L"fn test(a: u32, b: u32){}"
                      L"fn main(){"
                      L"  test@(1, 2.0);"
                      L"}");
        CHECK_INVALID(L"fn test(a: u32, b: u32){}"
                      L"fn main(){"
                      L"  test@(1.0, _);"
                      L"}");
    }
    SECTION("Const check.")
    {
        CHECK_VALID(L"fn const test(a: &u32, b: u32){}"
                    L"fn main(){"
                    L"  let var: u32 = 1;"
                    L"  let foo: fn(u32) = test@(&var, _);"
                    L"}");
        CHECK_INVALID(L"fn const test(a: &u32, b: u32){}"
                      L"fn main(){"
                      L"  let var: u32 = 1;"
                      L"  let foo: fn const(u32) = test@(&var, _);"
                      L"}");
        CHECK_VALID(L"fn const test(a: &mut u32, b: u32){}"
                    L"fn main(){"
                    L"  let mut var: u32 = 1;"
                    L"  let foo: fn(u32) = test@(&mut var, _);"
                    L"}");
        CHECK_INVALID(L"fn const test(a: &mut u32, b: u32){}"
                      L"fn main(){"
                      L"  let mut var: u32 = 1;"
                      L"  let foo: fn const(u32) = test@(&mut var, _);"
                      L"}");
    }
}

TEST_CASE("Indexing.")
{
    SECTION("Correct indexing.")
    {
        CHECK_VALID(FN_WRAP(L"let var: &str = \"some text\";"
                            L"let chr: char = var[2];"));
    }
    SECTION("Wrong indexed expr type.")
    {
        CHECK_INVALID(FN_WRAP(L"let var = true;"
                              L"var[2];"));
        CHECK_INVALID(FN_WRAP(L"let var = 2;"
                              L"var[2];"));
        CHECK_INVALID(FN_WRAP(L"let var = -2;"
                              L"var[2];"));
        CHECK_INVALID(FN_WRAP(L"let var = 2.0;"
                              L"var[2];"));
        CHECK_INVALID(FN_WRAP(L"let var = 'a';"
                              L"var[2];"));
    }
    SECTION("Wrong index type.")
    {
        CHECK_INVALID(FN_WRAP(L"let var: &str = \"some text\";"
                              L"var[-2];"));
        CHECK_INVALID(FN_WRAP(L"let var: &str = \"some text\";"
                              L"var[1.0];"));
        CHECK_INVALID(FN_WRAP(L"let var: &str = \"some text\";"
                              L"var[true];"));
        CHECK_INVALID(FN_WRAP(L"let var: &str = \"some text\";"
                              L"var['a'];"));
        CHECK_INVALID(FN_WRAP(L"let var: &str = \"some text\";"
                              L"var[\"a\"];"));
    }
}

TEST_CASE("Casting.")
{
    SECTION("From bool")
    {
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"var as bool;"));
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"var as u32;"));
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"var as i32;"));
        CHECK_VALID(FN_WRAP(L"let var: bool = true;"
                            L"var as f64;"));
        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"var as char;"));
    }
    SECTION("From u32")
    {
        CHECK_INVALID(FN_WRAP(L"let var: u32 = 1;"
                              L"var as bool;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"var as u32;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"var as i32;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"var as f64;"));
        CHECK_VALID(FN_WRAP(L"let var: u32 = 1;"
                            L"var as char;"));
    }
    SECTION("From i32")
    {
        CHECK_INVALID(FN_WRAP(L"let var: i32 = -1;"
                              L"var as bool;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"var as u32;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"var as i32;"));
        CHECK_VALID(FN_WRAP(L"let var: i32 = -1;"
                            L"var as f64;"));
        CHECK_INVALID(FN_WRAP(L"let var: i32 = -1;"
                              L"var as char;"));
    }
    SECTION("From f64")
    {
        CHECK_INVALID(FN_WRAP(L"let var: f64 = 1.0;"
                              L"var as bool;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0;"
                            L"var as u32;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0;"
                            L"var as i32;"));
        CHECK_VALID(FN_WRAP(L"let var: f64 = 1.0;"
                            L"var as f64;"));
        CHECK_INVALID(FN_WRAP(L"let var: f64 = 1.0;"
                              L"var as char;"));
    }
    SECTION("From char")
    {
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"var as bool;"));
        CHECK_VALID(FN_WRAP(L"let var: char = 'a';"
                            L"var as u32;"));
        CHECK_VALID(FN_WRAP(L"let var: char = 'a';"
                            L"var as i32;"));
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"var as f64;"));
        CHECK_VALID(FN_WRAP(L"let var: char = 'a';"
                            L"var as char;"));
    }
    SECTION("Casting from or to strings.")
    {
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"var as str;"));
        CHECK_INVALID(FN_WRAP(L"let var: char = 'a';"
                              L"var as &str;"));
        CHECK_INVALID(FN_WRAP(L"let var = \"a\";"
                              L"var as char;"));
        CHECK_INVALID(FN_WRAP(L"let var = \"a\";"
                              L"var as i32;"));
        CHECK_INVALID(FN_WRAP(L"let var = \"a\";"
                              L"var as str;"));
        CHECK_INVALID(FN_WRAP(L"let var = \"a\";"
                              L"var as &str;"));
        CHECK_INVALID(FN_WRAP(L"let var = \"a\";"
                              L"var as &mut str;"));
    }
    SECTION("Casting from or to references.")
    {

        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"var as &bool;"));
        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"let refer: &bool = &var;"
                              L"refer as i32;"));
        CHECK_INVALID(FN_WRAP(L"let var: bool = true;"
                              L"let refer = &mut var;"
                              L"refer as i32;"));
    }
}

TEST_CASE("Assignments.")
{
    SECTION("Normal assignments.")
    {
        CHECK_VALID(FN_WRAP(L"let mut var = 1; var = 2;"));
        CHECK_VALID(FN_WRAP(L"let mut var = 1;"
                            L"let refer: &u32 = &var;"));
        CHECK_VALID(FN_WRAP(L"let mut var = 1;"
                            L"let refer = &var;"));
        CHECK_VALID(FN_WRAP(L"let mut var = 1;"
                            L"let refer = &mut var;"));
        CHECK_INVALID(FN_WRAP(L"let mut var = 1;"
                              L" let refer: &mut u32 = &var;"));
        CHECK_INVALID(FN_WRAP(L"let mut var = 1;"
                              L"let refer: &u32 = &mut var;"));
        CHECK_INVALID(FN_WRAP(L"let mut var = \"some text\";"
                              L"let string = &var;"));
        CHECK_INVALID(FN_WRAP(L"let mut var = \"some text\";"
                              L"let string = &mut var;"));
    }
    SECTION("Simple assignments.")
    {
        SECTION("Plus.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var += true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var += 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var += (-1);"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1.0; var += 1.0;"));
            CHECK_VALID(FN_WRAP(L"let mut var = \"a\"; var += \"a\";"));
            CHECK_VALID(FN_WRAP(L"let mut var = \"a\"; var += 'a';"));
        }
        SECTION("Minus.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var -= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var -= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var -= (-1);"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1.0; var -= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var -= \"a\";"));
        }
        SECTION("Multiplication.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var *= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var *= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var *= (-1);"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1.0; var *= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var *= \"a\";"));
        }
        SECTION("Division.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var /= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var /= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var /= (-1);"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1.0; var /= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var /= \"a\";"));
        }
        SECTION("Modulo.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var %= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var %= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var %= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var %= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var %= \"a\";"));
        }
        SECTION("Exponentiation.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var ^^= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var ^^= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var ^^= 1;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = (-1); var ^^= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var ^^= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var ^^= \"a\";"));
        }
        SECTION("Bit and.")
        {
            CHECK_VALID(FN_WRAP(L"let mut var = true; var &= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var &= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var &= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var &= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var &= \"a\";"));
        }
        SECTION("Bit or.")
        {
            CHECK_VALID(FN_WRAP(L"let mut var = true; var |= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var |= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var |= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var |= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var |= \"a\";"));
        }
        SECTION("Bit xor.")
        {
            CHECK_VALID(FN_WRAP(L"let mut var = true; var ^= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var ^= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = (-1); var ^= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var ^= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var ^= \"a\";"));
        }
        SECTION("Shift left.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var <<= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var <<= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = -1; var <<= 1;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = (-1); var <<= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var <<= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var <<= \"a\";"));
        }
        SECTION("Shift right.")
        {
            CHECK_INVALID(FN_WRAP(L"let mut var = true; var >>= true;"));
            CHECK_VALID(FN_WRAP(L"let mut var = 1; var >>= 1;"));
            CHECK_VALID(FN_WRAP(L"let mut var = -1; var >>= 1;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = (-1); var >>= (-1);"));
            CHECK_INVALID(FN_WRAP(L"let mut var = 1.0; var >>= 1.0;"));
            CHECK_INVALID(FN_WRAP(L"let mut var = \"a\"; var >>= \"a\";"));
        }
    }
    SECTION("No const reassignments.")
    {
        CHECK_INVALID(FN_WRAP(L"let var = 1; var = 2;"));
        CHECK_INVALID(FN_WRAP(L"let var = 1; var += 2;"));
        CHECK_INVALID(FN_WRAP(L"let var = 1; var -= 2;"));
        CHECK_INVALID(FN_WRAP(L"let var = 1; var *= 2;"));
        CHECK_INVALID(FN_WRAP(L"let var = 1; var /= 2;"));
    }
}

TEST_CASE("Match statements.")
{
    SECTION("Checking for exhaustive patterns.")
    {
        CHECK_VALID(FN_WRAP(L"let var = 1;"
                            L"match(var) {else => {}}"));
        CHECK_VALID_WITH_WARNINGS(FN_WRAP(L"let var = 1;"
                                          L"match(var) {1 => {}}"));
        CHECK_VALID(FN_WRAP(L"let var = 1;"
                            L"match(var) {"
                            L"1 => {}"
                            L"else => {}"
                            L"}"));
        CHECK_VALID_WITH_WARNINGS(FN_WRAP(L"let var = 1;"
                                          L"match(var) {"
                                          L"if (true) => {}"
                                          L"}"));
        CHECK_VALID(FN_WRAP(L"let var = 1;"
                            L"match(var) {"
                            L"if (true) => {}"
                            L"else => {}"
                            L"}"));
    }
    SECTION("Type support.")
    {
        CHECK_VALID(FN_WRAP(L"let var = true;"
                            L"match(var) {else => {}}"));
        CHECK_VALID(FN_WRAP(L"let var = 1;"
                            L"match(var) {else => {}}"));
        CHECK_VALID(FN_WRAP(L"let var = -1;"
                            L"match(var) {else => {}}"));
        CHECK_VALID(FN_WRAP(L"let var = 1.0;"
                            L"match(var) {else => {}}"));
        CHECK_VALID(FN_WRAP(L"let var = 'a';"
                            L"match(var) {else => {}}"));
        CHECK_VALID(FN_WRAP(L"let var = \"a\";"
                            L"match(var) {else => {}}"));
        CHECK_INVALID(L"fn foo() {}"
                      L"fn boo() {match(foo) {else => {}}}");
    }
    SECTION("Literal arms.")
    {
        SECTION("Literals have the same type as a matched var.")
        {
            CHECK_VALID(FN_WRAP(L"let var = true;"
                                L"match(var) {"
                                L"  true => {}"
                                L"  else => {}"
                                L"}"));
            CHECK_VALID(FN_WRAP(L"let var = 1;"
                                L"match(var) {"
                                L"  1 => {}"
                                L"  else => {}"
                                L"}"));
            CHECK_VALID(FN_WRAP(L"let var = -1;"
                                L"match(var) {"
                                L"  -1 => {}"
                                L"  else => {}"
                                L"}"));
            CHECK_VALID(FN_WRAP(L"let var = 1.0;"
                                L"match(var) {"
                                L"  1.0 => {}"
                                L"  else => {}"
                                L"}"));
            CHECK_VALID(FN_WRAP(L"let var = 'a';"
                                L"match(var) {"
                                L"  'a' => {}"
                                L"  else => {}"
                                L"}"));
            CHECK_VALID(FN_WRAP(L"let var = \"a\";"
                                L"match(var) {"
                                L"  \"a\" => {}"
                                L"  else => {}"
                                L"}"));
            CHECK_INVALID(FN_WRAP(L"let var = 1;"
                                  L"match(var) {"
                                  L"  true => {}"
                                  L"  else => {}"
                                  L"}"));
            CHECK_INVALID(FN_WRAP(L"let var = 1;"
                                  L"match(var) {"
                                  L"  1.0 => {}"
                                  L"  else => {}"
                                  L"}"));
        }
    }
    SECTION("Guard arms.")
    {
        CHECK_VALID(FN_WRAP(L"let var = 1;"
                            L"match(var) {"
                            L"  if(true) => {}"
                            L"  else => {}"
                            L"}"));
        CHECK_INVALID(FN_WRAP(L"let var = 1;"
                              L"match(var) {"
                              L"  if(1) => {}"
                              L"  else => {}"
                              L"}"));
        CHECK_INVALID(FN_WRAP(L"let var = 1;"
                              L"match(var) {"
                              L"  if(1.0) => {}"
                              L"  else => {}"
                              L"}"));
        CHECK_INVALID(FN_WRAP(L"let var = 1;"
                              L"match(var) {"
                              L"  if(-1) => {}"
                              L"  else => {}"
                              L"}"));
        CHECK_INVALID(FN_WRAP(L"let var = 1;"
                              L"match(var) {"
                              L"  if('a') => {}"
                              L"  else => {}"
                              L"}"));
        CHECK_INVALID(FN_WRAP(L"let var = 1;"
                              L"match(var) {"
                              L"  if(\"a\") => {}"
                              L"  else => {}"
                              L"}"));
    }
}

TEST_CASE("Constant has no value assigned.")
{
    CHECK_INVALID(L"let var: u32;");
    CHECK_VALID(L"let var: u32 = 32;");
}

TEST_CASE("Constant cannot be reassigned.")
{
    CHECK_INVALID(FN_WRAP(L"let value = 5; value = 4;"));
    CHECK_VALID(FN_WRAP(L"let mut value = 5; value = 4;"));
}

TEST_CASE("Referenced value/function is not in scope.")
{
    SECTION("Default cases.")
    {
        CHECK_INVALID(
            L"fn foo(){let value = 5;} fn goo(){let new_value = value;}");
    }
    SECTION("Function arguments.")
    {
        CHECK_VALID(L"fn foo(a: u32, b: u32) {a + b;}");
        CHECK_INVALID(L"fn foo(a: u32) {a + b;}");
    }
}

TEST_CASE("Function doesn't return when its type indicates otherwise.")
{
    CHECK_INVALID(L"fn foo(x: u32)=>u32{}");
    CHECK_VALID(L"fn foo(x: u32){}");
}

TEST_CASE("Function returns the wrong type.")
{
    CHECK_INVALID(L"fn foo()=>u32{return 0.1;}");
    CHECK_INVALID(L"fn foo()=>u32{let value = 0.1; return value;}");
}

TEST_CASE("Return statement with no value in a function that does return.")
{
    CHECK_INVALID(L"fn foo(x: u32)=>u32{return;}");
}

TEST_CASE("Function returns a value when it shouldn't.")
{
    CHECK_INVALID(L"fn foo(x: u32){return 5;}");
    CHECK_INVALID(L"fn foo(x: u32){return 5;}");
}

TEST_CASE("Function parameter cannot shadow a variable/function in scope")
{
    CHECK_INVALID(L"let var = 5;"
                  L"fn foo(var: u32){}");
    CHECK_INVALID(L"fn var(){}"
                  L"fn foo(var: u32){}");
    CHECK_VALID(L"fn goo(){let var = 1;}"
                L"fn foo(var: u32){}");
    CHECK_VALID(L"fn foo(var: u32){}"
                L"fn goo(){let var = 1;}");
}

TEST_CASE("Const function cannot reference outside variables that are not "
          "passed as arguments")
{
    CHECK_INVALID(L"let var = 5;"
                  L"fn const foo(){let bar = var;}");
    CHECK_INVALID(L"let mut var = 5;"
                  L"fn const foo(){var = 4;}");
    CHECK_VALID(L"let var = 5;"
                L"fn foo(){let bar = var;}");
    CHECK_VALID(L"let mut var = 5;"
                L"fn foo(){var = 4;}");
}

TEST_CASE("Variable cannot be called 'main'.")
{
    CHECK_INVALID(L"let mut main: u32;");
    CHECK_INVALID(L"let mut main: fn();");
    CHECK_INVALID(L"let mut main: fn()=>u32;");
}

TEST_CASE("Variables and functions cannot be redefined.")
{
    CHECK_INVALID(L"let mut value: u32;"
                  L"let value = 1;");
    CHECK_INVALID(L"fn value(){}"
                  L"let value = 1;");
    CHECK_INVALID(L"let value = 1;"
                  L"fn value(){}");
    CHECK_INVALID(L"fn const value(){}"
                  L"fn value(){}");
}

TEST_CASE("Main should be of type fn()=>u32 or fn().")
{
    CHECK_VALID(L"fn main()=>u32{return 3;}");
    CHECK_INVALID(L"fn main()=>f64{return 0.1;}");
}

TEST_CASE("Main cannot have any parameters.")
{
    CHECK_INVALID(L"fn main(x: u32){}");
}

TEST_CASE("Externs.")
{
    SECTION("Valid externs.")
    {
        CHECK_VALID(L"extern foo(x: u32);");
        CHECK_VALID(L"extern foo(x: u32) => i32;");
    }
    SECTION("Main cannot be externed.")
    {
        CHECK_INVALID(L"extern main(x: u32);");
        CHECK_INVALID(L"extern main();");
        CHECK_INVALID(L"extern main()=>i32;");
    }
    SECTION("No redefinitions.")
    {
        CHECK_INVALID(L"extern foo(x: u32);"
                      L"extern foo();");
    }
}

TEST_CASE("Return statements coverage.")
{
    SECTION("If statements.")
    {
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"if (a) {}"
                    L"return 0;"
                    L"}");
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"if (a) return 1;"
                    L"return 0;"
                    L"}");
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"if (a) return 1;"
                    L"else return 0;"
                    L"}");
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"return 0;"
                    L"if (a) return 1;"
                    L"}");
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"return 0;"
                    L"if (a) return 1;"
                    L"else {}"
                    L"}");
        CHECK_INVALID(L"fn test(a: bool)=>u32{"
                      L"if (a) return 1;"
                      L"}");
        CHECK_INVALID(L"fn test(a: bool)=>u32{"
                      L"if (a) {}"
                      L"else return 0;"
                      L"}");
    }
    SECTION("Blocks.")
    {
        CHECK_VALID(L"fn test()=>u32{"
                    L"{return 1;}"
                    L"}");
    }
    SECTION("While statements.")
    {
        CHECK_INVALID(L"fn test(a: bool)=>u32{"
                      L"while (a) return 1;"
                      L"}");
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"while (a) {}"
                    L"return 1;"
                    L"}");
        CHECK_VALID(L"fn test(a: bool)=>u32{"
                    L"while (a) return 1;"
                    L"return 1;"
                    L"}");
    }
}

TEST_CASE("While, break, continue statements.")
{
    SECTION("Break and continue cannot be outside while loop.")
    {
        CHECK_INVALID(FN_WRAP(L"break;"));
        CHECK_INVALID(FN_WRAP(L"continue;"));
        CHECK_VALID(FN_WRAP(L"let a = true;"
                            L"while(a){"
                            L"continue;"
                            L"}"));
        CHECK_VALID(FN_WRAP(L"let a = true;"
                            L"while(a){"
                            L"break;"
                            L"}"));
        CHECK_VALID(FN_WRAP(L"let a = true;"
                            L"while(a){"
                            L"while(a){"
                            L"continue;"
                            L"}"
                            L"continue;"
                            L"}"));
        CHECK_VALID(FN_WRAP(L"let a = true;"
                            L"while(a){"
                            L"while(a){"
                            L"break;"
                            L"}"
                            L"break;"
                            L"}"));
    }
}