//mineutils库的日志相关操作
#pragma once
#ifndef LOG_HPP_MINEUTILS
#define LOG_HPP_MINEUTILS

#include<stdio.h>
#include<string>

#include"str.hpp"


namespace mineutils
{
    namespace mlog
    {

/*  生成带函数名的正常提示信息，将f_str中的"{}"替换为后续的...(__VA_ARGS__)参数
    @param f_str: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的...(__VA_ARGS__)参数替换。
    @param ...(__VA_ARGS__): 用于替换f_str中"{}"
    @return 生成的信息   */
#define msgN(f_str, ...) mineutils::mstr::fstr((mineutils::mlog::_getFstrN() + (f_str)), MINE_FUNCSIG, ##__VA_ARGS__)

/*  生成带函数名和位置的警告信息，将f_str中的"{}"替换为后续的...(__VA_ARGS__)参数
    @param f_str: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的...(__VA_ARGS__)参数替换。
    @param ...(__VA_ARGS__): 用于替换f_str中"{}"
    @return 生成的信息   */
#define msgW(f_str, ...) mineutils::mstr::fstr((mineutils::mlog::_getFstrW() + (f_str)), MINE_FUNCSIG, __FILE__, __LINE__, ##__VA_ARGS__)

/*  生成带函数名和位置的错误信息，将f_str中的"{}"替换为后续的...(__VA_ARGS__)参数
    @param f_str: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的...(__VA_ARGS__)参数替换。
    @param ...(__VA_ARGS__): 用于替换f_str中"{}"
    @return 生成的信息   */
#define msgE(f_str, ...) mineutils::mstr::fstr((mineutils::mlog::_getFstrE() + (f_str)), MINE_FUNCSIG, __FILE__, __LINE__, ##__VA_ARGS__)

//按printf的格式调用，打印带函数名的正常提示信息
#define printfN(fmt_chars, ...) printf((mineutils::mlog::_getFmtN() + (fmt_chars)).c_str(), MINE_FUNCSIG, fmt_chars, ##__VA_ARGS__)

//按printf的格式调用，打印带函数名和位置的警告信息
#define printfW(fmt_chars, ...) printf((mineutils::mlog::_getFmtW() + (fmt_chars)).c_str(), MINE_FUNCSIG, __FILE__, __LINE__, fmt_chars, ##__VA_ARGS__)

//按printf的格式调用，打印带函数名和位置的错误信息
#define printfE(fmt_chars, ...) printf((mineutils::mlog::_getFmtE() + (fmt_chars)).c_str(), MINE_FUNCSIG, __FILE__, __LINE__, fmt_chars, ##__VA_ARGS__)

#ifndef NDEBUG
#define dprintf(fmt_chars, ...) printf(fmt_chars, ##__VA_ARGS__)
#define dprintfN(fmt_chars, ...) printfN(fmt_chars, ##__VA_ARGS__)
#define dprintfW(fmt_chars, ...) printfw(fmt_chars, ##__VA_ARGS__)
#define dprintfE(fmt_chars, ...) printfe(fmt_chars, ##__VA_ARGS__)
#else
#define dprintf(fmt_chars, ...) ((void*)0)
#define dprintfN(fmt_chars, ...) ((void*)0)
#define dprintfW(fmt_chars, ...) ((void*)0)
#define dprintfE(fmt_chars, ...) ((void*)0)
#endif 

        template <class... Ts>
        inline MINE_DEPRECATED("Function \"messageW\" has been deprecated, please replace with macro definition \"msgW\"(in log.hpp)!") std::string messageW(const std::string& fstr_content, const Ts... args)
        {
            return mstr::color("!Warning! ", mstr::Color::yellow) + mstr::fstr(fstr_content, args...);
        }


        template <class... Ts>
        inline MINE_DEPRECATED("Function \"messageE\" has been deprecated, please replace with macro definition \"msgE\"(in log.hpp)!") std::string messageE(const std::string& fstr_content, const Ts... args)
        {
            return mstr::color("!!!Error!!! ", mstr::Color::red) + mstr::fstr(fstr_content, args...);
        }

        template <class... Ts>
        inline MINE_DEPRECATED("Function \"messageN\" has been deprecated, please replace with macro definition \"msgN\"(in log.hpp)!") std::string messageN(mstr::Color str_color, const std::string& fstr_content, const Ts... args)
        {
            return mstr::color(mstr::fstr(fstr_content, args...), str_color);
        }


        inline std::string& _getFstrN()
        {
            static std::string warning_message("\"{}\": ");
            return warning_message;
        }

        inline std::string& _getFstrW()
        {
            static std::string warning_message("!Warning! \"{}\"[{}](line {}): ");
            return warning_message;
        }
        inline std::string& _getFstrE()
        {
            static std::string error_message("!!!Error!!! \"{}\"[{}](line {}): ");
            return error_message;
        }

        inline std::string& _getFmtN()
        {
            static std::string warning_message("\"%s\": ");
            return warning_message;
        }

        inline std::string& _getFmtW()
        {
            static std::string warning_message("!Warning! \"%s\"[%s](line %d): ");
            return warning_message;
        }
        inline std::string& _getFmtE()
        {
            static std::string error_message("!!!Error!!! \"%s\"[%s](line %d): ");
            return error_message;
        }
    }


}

#endif // !LOG_HPP_MINEUTILS