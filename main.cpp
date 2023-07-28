#include "exceptions.hpp"
#include "parser.hpp"
#include "print_visitor.hpp"

int main()
{
    auto locale = Locale("en_US.utf8");
    auto parser = Parser(Lexer::from_file("../example.mole"));
    auto visitor = PrintVisitor(std::wcout);
    try
    {
        visitor.visit(*(parser.parse()));
    }
    catch (const CompilerException &e)
    {
        std::wcerr << e.wwhat() << std::endl;
        return 1;
    }
    return 0;
}