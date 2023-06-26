#include "parser.hpp"

int main()
{
    auto lexer = Lexer::from_wstring(L"fn penis() {}");
    auto parser = Parser(lexer);
    parser.parse();
    return 0;
}