#include "compiled_program.hpp"
#include "json_serializer.hpp"
#include "locale.hpp"
#include "parser.hpp"
#include "semantic_checker.hpp"
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
                                          llvm::cl::init("../example.mole")
                                          //   llvm::cl::Required
    );
    llvm::cl::opt<bool> dump_ast(
        "ast-dump",
        llvm::cl::desc(
            "Dump the abstract syntax tree of the file as a JSON object."),
        llvm::cl::init(false), llvm::cl::cat(mole_opts));
    llvm::cl::opt<bool> dump_ir("ir-dump", llvm::cl::desc("Dump the LLVM IR."),
                                llvm::cl::init(false),
                                llvm::cl::cat(mole_opts));
    llvm::cl::opt<bool> dump_bc(
        "bc-dump", llvm::cl::desc("Dump the LLVM bytecode."),
        llvm::cl::init(false), llvm::cl::cat(mole_opts));
    llvm::cl::opt<bool> optimize(
        "optimize", llvm::cl::desc("Optimize the created llvm ir."),
        llvm::cl::init(false), llvm::cl::cat(mole_opts));
    llvm::cl::opt<std::string> output_file(
        "o", llvm::cl::desc("Specify the output file."),
        llvm::cl::value_desc("filename"), llvm::cl::cat(mole_opts));

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

    LexerPtr lexer;
    try
    {
        lexer = Lexer::from_file(path);
    }
    catch (const std::ios_base::failure &e)
    {
        std::cerr << e.what() << '\n';
        return std::make_error_condition(std::errc::io_error).value();
    }

    lexer->add_logger(&logger);
    lexer->add_logger(&error_checker);

    auto parser = Parser(std::move(lexer));
    parser.add_logger(&logger);
    parser.add_logger(&error_checker);

    auto semantic_checker = SemanticChecker();
    semantic_checker.add_logger(&logger);
    semantic_checker.add_logger(&error_checker);

    auto program = parser.parse();
    if (!error_checker)
    {
        return std::make_error_condition(std::errc::invalid_argument).value();
    }

    semantic_checker.check(*program);
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
                std::cerr << "Error while opening the output file."
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
    else
    {
        try
        {
            auto compiled = CompiledProgram(*program);
            if (optimize.getValue())
            {
                compiled.optimize();
            }
            std::error_code ec;
            auto path = output_file.getValue();
            if (path.empty())
            {
                if (dump_ir.getValue())
                    path = "./out.ll";
                else if (dump_bc.getValue())
                    path = "./out.bc";
                else
                    path = "./out.o";
            }
            auto output = llvm::raw_fd_ostream(path, ec);
            if (ec)
            {
                std::cerr << "Error while opening the output file."
                          << std::endl;
                return std::make_error_condition(std::errc::io_error).value();
            }
            if (dump_ir.getValue())
                compiled.output_ir(output);
            else if (dump_bc.getValue())
                compiled.output_bytecode(output);
            else
                compiled.output_object_file(output);
        }
        catch (const CompilationException &e)
        {
            std::cerr << e.what() << '\n';
            return std::make_error_condition(std::errc::invalid_argument)
                .value();
        }
    }
    return 0;
}