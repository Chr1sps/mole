#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
enum class LogLevel : uint8_t
{
    INFO,
    WARNING,
    ERROR
};

struct LogMessage
{
    std::wstring text;
    LogLevel log_level;
};

class Logger
{
  protected:
    static std::map<LogLevel, std::wstring> log_level_strings;

  public:
    virtual void log(const LogMessage &) = 0;
};

using LoggerPtr = std::shared_ptr<Logger>;

class ConsoleLogger : public Logger
{
    std::wostream &out;

  public:
    ConsoleLogger() : out(std::wcout)
    {
    }

    void log(const LogMessage &msg) override;
};

class ExecutionLogger : public Logger
{
    bool run;
    LogLevel threshold;

  public:
    ExecutionLogger(const LogLevel &threshold)
        : run(true), threshold(threshold)
    {
    }

    ExecutionLogger() : ExecutionLogger(LogLevel::ERROR)
    {
    }

    void log(const LogMessage &msg) override;

    operator bool()
    {
        return this->run;
    }
};

class FileLogger : public Logger

{
    std::filesystem::path file_path;
    std::wofstream output;

  public:
    FileLogger(const FileLogger &) = delete;

    FileLogger(const std::filesystem::path &path) : file_path(path)
    {
    }

    void log(const LogMessage &msg) override;
};

class DebugLogger : public Logger
{
    std::vector<LogMessage> messages;

  public:
    DebugLogger() = default;
    void log(const LogMessage &msg) override;
    const std::vector<LogMessage> &get_messages() const;
};

class Reporter
{
  protected:
    std::unordered_set<Logger *> loggers;
    virtual std::wstring wrap_error_msg(const std::wstring &msg) const = 0;

  public:
    void report_error(const std::wstring &error_msg) const
    {
        auto wrapped = this->wrap_error_msg(error_msg);
        auto log_entry = LogMessage(wrapped, LogLevel::ERROR);
        for (const auto &logger : this->loggers)
        {
            logger->log(log_entry);
        }
    }

    void add_logger(Logger *logger)
    {
        this->loggers.insert(logger);
    }

    void remove_logger(Logger *logger)
    {
        this->loggers.erase(logger);
    }
};
#endif