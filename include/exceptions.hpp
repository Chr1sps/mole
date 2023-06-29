#ifndef __EXCEPTIONS_HPP__
#define __EXCEPTIONS_HPP__

#include <stdexcept>

class CompilerException : public std::runtime_error
{
    std::wstring what_str;
    std::string ascii_what_str;

  protected:
    CompilerException(const char *runtime_str, const std::wstring &what_str)
        : std::runtime_error(runtime_str), what_str(what_str),
          ascii_what_str(std::string(what_str.begin(), what_str.end()))
    {
    }

  public:
    CompilerException(const std::wstring &what_str)
        : CompilerException("Compiler error", what_str)

    {
    }

    const char *what() const noexcept override
    {
        return this->ascii_what_str.c_str();
    }

    const std::wstring &wwhat() const noexcept
    {
        return this->what_str;
    }
};

class LexerException : public CompilerException
{
  public:
    LexerException(const std::wstring &what_str)
        : CompilerException("Lexer error", what_str)
    {
    }
};

class ParserException : public CompilerException
{
  public:
    ParserException(const std::wstring &what_str)
        : CompilerException("Parser error", what_str)
    {
    }
};
#endif