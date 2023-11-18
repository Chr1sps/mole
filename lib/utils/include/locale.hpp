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

    std::locale current_locale;

  public:
    Locale(const std::string &locale_name)
        : current_locale(std::locale(locale_name))
    {
        std::locale::global(this->current_locale);
    }

    ~Locale()
    {
        std::locale::global(std::locale::classic());
    }
};

#endif
