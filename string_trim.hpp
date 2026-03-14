#pragma once
#include <string>
#include <fstream>

template <typename StringT>
class basic_string_trim
{
    using CharT = typename StringT::value_type;

public:

    static StringT trim_left(StringT &str, CharT ch = ' ')
    {
        str.erase(0, str.find_first_not_of(ch)); // prefixing spaces
        return str;
    }
    static StringT trim_right(StringT &str, CharT ch = ' ')
    {
        str.erase(str.find_last_not_of(ch) + 1); // suffixing spaces
        return str;
    }
    static StringT trim(StringT &str, CharT ch = ' ')
    {
        trim_left(str, ch);     // prefixing spaces
        trim_right(str, ch);    // suffixing spaces
        return str;
    }
};

using string_trim  = basic_string_trim<std::string>;
using wstring_trim = basic_string_trim<std::wstring>;