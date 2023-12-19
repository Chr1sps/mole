#include "parser.hpp"
#include "print_visitor.hpp"
// #include "semantic_checker.hpp"
#include "locale.hpp"
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
        auto locale = Locale("C.utf8");
        auto logger = ConsoleLogger();
        auto error_checker = ExecutionLogger();
        auto lexer = Lexer::from_file(argv[1]);
        lexer->add_logger(&logger);
        lexer->add_logger(&error_checker);
        auto parser = Parser(lexer);
        parser.add_logger(&logger);
        parser.add_logger(&error_checker);
        auto program = parser.parse();
        if (!error_checker)
        {
            return std::make_error_condition(std::errc::invalid_argument)
                .value();
        }
        auto serializer = JsonSerializer();
        auto result = serializer.serialize(*program);
        std::cout << result << std::endl;
        return 0;
    }
}