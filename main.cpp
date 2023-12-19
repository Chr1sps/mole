#include "json_serializer.hpp"
#include "parser.hpp"
// #include "semantic_checker.hpp"
#include "locale.hpp"
#include <fstream>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <system_error>

int main(int argc, char **argv)
{
    llvm::cl::OptionCategory mole_opts("Mole options");
    llvm::cl::opt<std::string> input_file(llvm::cl::Positional,
                                          llvm::cl::desc("<input file>"),
                                          llvm::cl::Required);
    llvm::cl::opt<bool> dump_ast(
        "ast-dump",
        llvm::cl::desc(
            "Dump the abstract syntax tree of the file as a JSON object."),
        llvm::cl::init(false), llvm::cl::cat(mole_opts));
    llvm::cl::opt<std::string> output_file(
        "o", llvm::cl::desc("Specify the output file."),
        llvm::cl::value_desc("filename"),
        // llvm::cl::init("out.o"),
        llvm::cl::cat(mole_opts));

    llvm::InitLLVM init(argc, argv);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    llvm::cl::HideUnrelatedOptions(mole_opts);
    llvm::cl::ParseCommandLineOptions(argc, argv);

    auto path = input_file.getValue();

    auto locale = Locale("C.utf8");
    auto logger = ConsoleLogger();
    auto error_checker = ExecutionLogger();
    auto lexer = Lexer::from_file(path);
    lexer->add_logger(&logger);
    lexer->add_logger(&error_checker);
    auto parser = Parser(lexer);
    parser.add_logger(&logger);
    parser.add_logger(&error_checker);
    auto program = parser.parse();
    if (!error_checker)
    {
        return std::make_error_condition(std::errc::invalid_argument).value();
    }
    if (dump_ast.getValue())
    {
        auto serializer = JsonSerializer();
        auto result = serializer.serialize(*program);
        if (output_file.getValue() != "")
        {
            std::ofstream output;
            output.open(output_file.getValue());
            if (!output.good())
            {
                std::cout << "Error while opening the output file."
                          << std::endl;
                return std::make_error_condition(std::errc::io_error).value();
            }
            output << result.dump(4) << std::endl;
            output.close();
        }
        else
            std::cout << result.dump(4) << std::endl;
        return 0;
    }
    return 0;
}