#ifndef __STRING_BUILDER_HPP__
#define __STRING_BUILDER_HPP__
#include <sstream>

// Why, out of all the headers possible, is the std::format one unavailable in
// the gcc std lib on ubuntu 22.04? And why has noone thought about bringing it
// in? For crying out loud, c++23 has arrived and yet we have to struggle with
// something as mundane as string formatting.

template <typename... StringArgs> class StringBuilder
{
    std::wstringstream stream;

  public:
    StringBuilder(StringArgs &&...args)
    {
        this->add_to_wstring(args...);
    }

    template <typename T, typename... Args>
    void add_to_wstring(const T &value, Args &&...rest)
    {
        this->stream << value;
        this->add_to_wstring(rest...);
    }

    template <typename T> void add_to_wstring(const T &value)
    {
        this->stream << value;
    }

    std::wstring consume()
    {
        auto result = this->stream.str();
        this->stream.str(L"");
        return result;
    };
};

template <typename... Args> std::wstring build_wstring(Args &&...elements)
{
    return StringBuilder<Args...>(std::forward<Args>(elements)...).consume();
}
#endif