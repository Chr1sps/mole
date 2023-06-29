#include "parser.hpp"
#include "print_visitor.hpp"

int main()
{
    ReaderPtr reader =
        std::make_unique<FileReader>(FileReader("../example.mole"));
    auto lexer = std::make_unique<Lexer>(Lexer(reader));
    auto parser = Parser(lexer);
    auto visitor = PrintVisitor(std::wcout);
    try
    {
        visitor.visit(*(parser.parse()));
    }
    catch (const LexerException &e)
    {
        std::wcerr << e.wwhat() << std::endl;
        return 1;
    }
    catch (const ParserException &e)
    {
        std::wcerr << e.wwhat() << std::endl;
        return 1;
    }
    return 0;
}