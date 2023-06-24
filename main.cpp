#include <iostream>
#include <locale>

int main()
{
    std::wcout << "User-preferred locale setting is " << std::locale("").name().c_str() << '\n';
    // on startup, the global locale is the "C" locale
    std::wcout << 1000.01 << '\n';

    // replace the C++ global locale and the "C" locale with the user-preferred locale
    auto prev = std::locale::global(std::locale("en_US.UTF8"));
    // use the new global locale for future wide character output
    std::wcout << "User-preferred locale setting is " << prev.name().c_str() << '\n';
    std::wcout.imbue(std::locale());

    // output the same number again
    std::wcout << 1000.01 << '\n';
    return 0;
}