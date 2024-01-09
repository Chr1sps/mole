#include "logger.hpp"

const std::map<LogLevel, std::wstring> Logger::log_level_strings = {
    {LogLevel::INFO, L"INFO"},
    {LogLevel::WARNING, L"WARNING"},
    {LogLevel::ERROR, L"ERROR"}};

void DebugLogger::log(const LogMessage &msg) noexcept
{
    this->messages.push_back(msg);
    switch (msg.log_level)
    {
    case LogLevel::WARNING:
        this->warning_flag = true;
        break;
    case LogLevel::ERROR:
        this->error_flag = true;
        break;

    default:
        break;
    }
}

const std::vector<LogMessage> &DebugLogger::get_messages() const
{
    return this->messages;
}

bool DebugLogger::contains_errors() const noexcept
{
    return this->error_flag;
}

bool DebugLogger::contains_warnings() const noexcept
{
    return this->warning_flag;
}

void ExecutionLogger::log(const LogMessage &msg) noexcept
{
    if (msg.log_level >= this->threshold)
        this->run = false;
}

void ConsoleLogger::log(const LogMessage &msg) noexcept
{
    this->out << "[" << this->log_level_strings.at(msg.log_level) << "] "
              << msg.text << std::endl;
}