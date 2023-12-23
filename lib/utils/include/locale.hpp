#ifndef __LOCALE_HPP__
#define __LOCALE_HPP__

#include <locale>
#include <memory>
#include <string>

class Locale
{
    Locale(const Locale &) = delete;
    Locale(Locale &&) = delete;
    void operator=(const Locale &) = delete;
    void operator=(Locale &&) = delete;

    std::locale last_locale, current_locale;

  public:
    Locale(const std::string &locale_name)
        : last_locale(std::locale()), current_locale(std::locale(locale_name))
    {
        std::locale::global(this->current_locale);
    }

    ~Locale()
    {
        std::locale::global(this->last_locale);
    }
};

#endif
