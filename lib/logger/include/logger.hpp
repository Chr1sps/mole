#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>
enum class LogLevel
{
    INFO,
    WARNING,
    ERROR
};

struct LogMessage
{
    std::wstring text;
    LogLevel log_level;
    friend bool operator==(const LogMessage &, const LogLevel &);
    friend bool operator!=(const LogMessage &, const LogLevel &);
};

class Logger
{
  protected:
    static std::map<LogLevel, std::wstring> log_level_strings;

  public:
    virtual void log(const LogMessage &) = 0;
};

using LoggerPtr = std::shared_ptr<Logger>;

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
#endif