#include "logger.hpp"

const std::map<LogLevel, std::wstring> Logger::log_level_strings = {
    {LogLevel::INFO, L"INFO"},
    {LogLevel::WARNING, L"WARNING"},
    {LogLevel::ERROR, L"ERROR"}};

void FileLogger::log(const LogMessage &msg) noexcept
{
    this->output << "[" << this->log_level_strings.at(msg.log_level) << "] "
                 << msg.text << std::endl;
}

void DebugLogger::log(const LogMessage &msg) noexcept
{
    this->messages.push_back(msg);
}

const std::vector<LogMessage> &DebugLogger::get_messages() const
{
    return this->messages;
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