#ifndef __OVERLOADED_HPP__
#define __OVERLOADED_HPP__

template <typename... ts> struct overloaded : ts...
{
    using ts::operator()...;
};
#endif