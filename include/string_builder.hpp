#ifndef __STRING_BUILDER_HPP__
#define __STRING_BUILDER_HPP__
#include <sstream>

// Why, out of all the headers possible, is the std::format one unavailable in
// the gcc std lib on ubuntu 22.04? And why has noone thought about bringing it
// in? For crying out loud, c++23 has arrived and yet we have to struggle with
// something as mundane as string formatting.

template <typename T>
concept StringStreamLike =
    requires {
        typename T::char_type;
        typename T::traits_type;
        typename T::allocator_type;
    } &&
    std::derived_from<T, std::basic_stringstream<typename T::char_type,
                                                 typename T::traits_type,
                                                 typename T::allocator_type>>;

template <StringStreamLike StringStream, typename... StringArgs>
class StringBuilder
{
    using StringType = std::basic_string<typename StringStream::char_type>;
    StringStream stream;

  public:
    StringBuilder(StringArgs &&...args)
    {
        this->add_to_string(args...);
    }

    template <typename T, typename... Args>
    void add_to_string(const T &value, Args &&...rest)
    {
        this->stream << value;
        this->add_to_string(rest...);
    }

    template <typename T> void add_to_string(const T &value)
    {
        this->stream << value;
    }

    StringType consume()
    {
        auto result = this->stream.str();
        this->stream.str(StringType());
        return result;
    };
};

template <typename... Args> std::wstring build_wstring(Args &&...elements)
{
    return StringBuilder<std::wstringstream, Args...>(
               std::forward<Args>(elements)...)
        .consume();
}

template <typename... Args> std::string build_string(Args &&...elements)
{
    return StringBuilder<std::stringstream, Args...>(
               std::forward<Args>(elements)...)
        .consume();
}
#endif