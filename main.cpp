#include "parser.hpp"
#include "print_visitor.hpp"

int main()
{
    ReaderPtr reader =
        std::make_unique<FileReader>(FileReader("../example.mole"));
    auto lexer = std::make_unique<Lexer>(Lexer(reader));
    auto parser = Parser(lexer);
    auto visitor = PrintVisitor(std::wcout);
    visitor.visit(*(parser.parse()));
    return 0;
}