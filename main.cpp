#include "exceptions.hpp"
#include "parser.hpp"
#include "print_visitor.hpp"
#include "semantic_checker.hpp"
#include <system_error>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "No filename provided. Usage: ./molec <filename>."
                  << std::endl;
        return std::make_error_condition(std::errc::invalid_argument).value();
    }
    else
    {
        try
        {
            auto locale = Locale("en_US.utf8");
            auto parser = Parser(Lexer::from_file(argv[1]));
            auto visitor = PrintVisitor(std::wcout);
            auto checker = SemanticChecker();
            auto program = parser.parse();
            checker.visit(*(program));
            visitor.visit(*(program));
        }
        catch (const CompilerException &e)
        {
            std::wcerr << e.wwhat() << std::endl;
            return std::make_error_condition(std::errc::invalid_argument)
                .value();
        }
        catch (const std::ios_base::failure &e)
        {
            std::cerr << e.what() << std::endl;
            return std::make_error_condition(std::errc::io_error).value();
        }
        return 0;
    }
}