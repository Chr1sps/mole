#include "exceptions.hpp"
#include "parser.hpp"
#include "print_visitor.hpp"
#include "semantic_checker.hpp"

int main()
{
    auto locale = Locale("en_US.utf8");
    auto parser = Parser(Lexer::from_file("../example.mole"));
    auto visitor = PrintVisitor(std::wcout);
    auto checker = SemanticChecker();
    try
    {
        auto program = parser.parse();
        checker.visit(*(program));
        visitor.visit(*(program));
    }
    catch (const CompilerException &e)
    {
        std::wcerr << e.wwhat() << std::endl;
        return 1;
    }
    return 0;
}