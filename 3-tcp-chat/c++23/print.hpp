#pragma once

#if _HAS_CXX23
    #include <print>

    #define Print(...) std::print(__VA_ARGS__)
    #define Println(...) std::println(__VA_ARGS__)
#else
    #include <format>

    template< class... Args >
    void Print( std::format_string<Args...> fmt, Args&&... args ) {
        Print(stdout, fmt, std::forward<Args>(args)...);
    }

    template< class... Args >
    void Print( std::FILE* stream,
                std::format_string<Args...> fmt, Args&&... args )
    {   
        std::string str = std::format(fmt, std::forward<Args>(args)...);
        std::fwrite(str.data(), 1, str.size(), stream);
    }

    template< class... Args >
    void Println( std::format_string<Args...> fmt, Args&&... args ) {
        Println(stdout, fmt, std::forward<Args>(args)...);
    }

    template< class... Args >
    void Println( std::FILE* stream,
                  std::format_string<Args...> fmt, Args&&... args )
    {
        std::string str = std::format(fmt, std::forward<Args>(args)...);
        str.append("\n");
        std::fwrite(str.data(), 1, str.size(), stream);
    }

    inline void Println() {
        Println("");
    }

    inline void Println( std::FILE* stream ) {
        Println(stream, "");
    }
#endif