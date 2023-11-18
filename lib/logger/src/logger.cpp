#include "logger.hpp"

std::map<LogLevel, std::wstring> Logger::log_level_strings = {
    {LogLevel::INFO, L"INFO"},
    {LogLevel::WARNING, L"WARNING"},
    {LogLevel::ERROR, L"ERROR"}};

void FileLogger::log(const LogMessage &msg)
{
    this->output << "[" << this->log_level_strings[msg.log_level] << "]"
                 << msg.text << std::endl;
}

void DebugLogger::log(const LogMessage &msg)
{
    this->messages.push_back(msg);
}