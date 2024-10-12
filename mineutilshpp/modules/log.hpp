//mineutils库的日志相关操作
#pragma once
#ifndef LOG_HPP_MINEUTILS
#define LOG_HPP_MINEUTILS

#include<stdarg.h>
#include<stdio.h>
#include<string>

#include"base.hpp"
#include"str.hpp"

//命名空间::类名::函数名
#define MINE_FUNCNAME (mineutils::mstr::rsplit(mineutils::mstr::split(MINE_FUNCSIG, __func__ + std::string("("), 1).front(), " ", 1).back() + __func__).c_str()
//
//#define MINE_CLASSNAME (mineutils::mlog::_getClassName(MINE_FUNCNAME).c_str())



namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

        /*  生成带函数名的正常提示信息，将f_str中的"{}"替换为后续的...(__VA_ARGS__)参数
            @param f_str: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的...(__VA_ARGS__)参数替换。
            @param ...(__VA_ARGS__): 用于替换f_str中"{}"
            @return 生成的信息   */
#define mmsgN(f_str, ...) mineutils::mlog::_msgN(f_str, MINE_FUNCNAME, ##__VA_ARGS__)  

        /*  生成带函数名和位置的警告信息，将f_str中的"{}"替换为后续的...(__VA_ARGS__)参数
            @param f_str: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的...(__VA_ARGS__)参数替换。
            @param ...(__VA_ARGS__): 用于替换f_str中"{}"
            @return 生成的信息   */
#define mmsgW(f_str, ...) mineutils::mlog::_msgW(f_str, MINE_FUNCSIG, __FILE__, __LINE__, ##__VA_ARGS__)

        /*  生成带函数名和位置的错误信息，将f_str中的"{}"替换为后续的...(__VA_ARGS__)参数
            @param f_str: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的...(__VA_ARGS__)参数替换。
            @param ...(__VA_ARGS__): 用于替换f_str中"{}"
            @return 生成的信息   */
#define mmsgE(f_str, ...) mineutils::mlog::_msgE(f_str, MINE_FUNCSIG, __FILE__, __LINE__, ##__VA_ARGS__)

        //按printf的格式调用，打印带函数名的正常提示信息
#define mprintfN(fmt_chars, ...) mineutils::mlog::_printfN(fmt_chars, MINE_FUNCNAME, ##__VA_ARGS__)

        //按printf的格式调用，打印带函数名和位置的警告信息
#define mprintfW(fmt_chars, ...) mineutils::mlog::_printfW(fmt_chars, MINE_FUNCSIG, __FILE__, __LINE__, ##__VA_ARGS__)

        //按printf的格式调用，打印带函数名和位置的错误信息
#define mprintfE(fmt_chars, ...) mineutils::mlog::_printfE(fmt_chars, MINE_FUNCSIG, __FILE__, __LINE__, ##__VA_ARGS__)

#ifndef NDEBUG
#define mdprintf(fmt_chars, ...) printf(fmt_chars, ##__VA_ARGS__)
#define mdprintfN(fmt_chars, ...) mprintfN(fmt_chars, ##__VA_ARGS__)
#define mdprintfW(fmt_chars, ...) mprintfw(fmt_chars, ##__VA_ARGS__)
#define mdprintfE(fmt_chars, ...) mprintfe(fmt_chars, ##__VA_ARGS__)
#else
#define mdprintf(fmt_chars, ...) ((void*)0)
#define mdprintfN(fmt_chars, ...) ((void*)0)
#define mdprintfW(fmt_chars, ...) ((void*)0)
#define mdprintfE(fmt_chars, ...) ((void*)0)
#endif 



    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mlog
    {
        template <class... Ts>
        inline MINE_DEPRECATED(R"(Deprecated. Please replace with macro definition "msgW"(in log.hpp))") 
            std::string messageW(const std::string& fstr_content, const Ts... args)
        {
            return mstr::color("!Warning! ", mstr::Color::yellow) + mstr::fstr(fstr_content, args...);
        }


        template <class... Ts>
        inline MINE_DEPRECATED(R"(Deprecated. Please replace with macro definition "msgE"(in log.hpp))")
            std::string messageE(const std::string& fstr_content, const Ts... args)
        {
            return mstr::color("!!!Error!!! ", mstr::Color::red) + mstr::fstr(fstr_content, args...);
        }

        template <class... Ts>
        inline MINE_DEPRECATED(R"(Deprecated. Please replace with macro definition "msgN"(in log.hpp))")
            std::string messageN(mstr::Color str_color, const std::string& fstr_content, const Ts... args)
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

        //inline std::string _getClassName(const char* funcname)
        //{
        //    auto vec = mineutils::mstr::rsplit(funcname, "::", 1);
        //    return (vec.size() > 1 ? vec[0] : "");
        //}
        template<class... Ts>
        inline std::string _msgN(std::string f_str, const char* funcname, Ts ...args)
        {
            return mstr::fstr((mlog::_getFstrN() + f_str), funcname, args...);
        }

        template<class... Ts>
        inline std::string _msgW(std::string f_str, const char* funcname, const char* filename, int line, Ts ...args)
        {
            return mstr::fstr((mlog::_getFstrW() + f_str), funcname, filename, line, args...);
        }

        template<class... Ts>
        inline std::string _msgE(std::string f_str, const char* funcname, const char* filename, int line, Ts ...args)
        {
            return mstr::fstr((mlog::_getFstrE() + f_str), funcname, filename, line, args...);
        }

        template<class... Ts>
        inline void _printfN(const char* fmt_chars, const char* funcname, Ts ...args)
        {
            printf((mlog::_getFmtN() + (fmt_chars)).c_str(), funcname, args...);
        }

        template<class... Ts>
        inline void _printfW(const char* fmt_chars, const char* funcname, const char* filename, int line, Ts ...args)
        {
            printf((mlog::_getFmtW() + (fmt_chars)).c_str(), funcname, filename, line, args...);
        }

        template<class... Ts>
        inline void _printfE(const char* fmt_chars, const char* funcname, const char* filename, int line, Ts ...args)
        {
            printf((mlog::_getFmtE() + (fmt_chars)).c_str(), funcname, filename, line, args...);
        }
    }
}

#ifndef msgN
//已废弃
#define msgN(f_str, ...) mmsgN(f_str, ##__VA_ARGS__)
#endif

#ifndef msgW
//已废弃
#define msgW(f_str, ...) mmsgW(f_str, ##__VA_ARGS__)
#endif

#ifndef msgE
//已废弃
#define msgE(f_str, ...) mmsgE(f_str, ##__VA_ARGS__)
#endif

#ifndef printfN
//已废弃
#define printfN(fmt_chars, ...) mprintfN(fmt_chars, ##__VA_ARGS__)
#endif

#ifndef printfW
//已废弃
#define printfW(fmt_chars, ...) mprintfW(fmt_chars, ##__VA_ARGS__)
#endif

#ifndef printfE
//已废弃
#define printfE(fmt_chars, ...) mprintfE(fmt_chars, ##__VA_ARGS__)
#endif

#ifndef dprintf
//已废弃
#define dprintf(fmt_chars, ...) mdprintf(fmt_chars, ##__VA_ARGS__)
#endif

#ifndef dprintfN
//已废弃
#define dprintfN(fmt_chars, ...) mdprintfN(fmt_chars, ##__VA_ARGS__)
#endif

#ifndef dprintfW
//已废弃
#define dprintfW(fmt_chars, ...) mdprintfW(fmt_chars, ##__VA_ARGS__)
#endif

#ifndef dprintfE
//已废弃
#define dprintfE(fmt_chars, ...) mdprintfE(fmt_chars, ##__VA_ARGS__)
#endif
#endif // !LOG_HPP_MINEUTILS