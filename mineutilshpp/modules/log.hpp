//mineutils库的日志相关操作
#pragma once
#ifndef LOG_HPP_MINEUTILS
#define LOG_HPP_MINEUTILS

#include<string>

#include"str.hpp"


namespace mineutils
{
    namespace mlog
    {
        /*  生成警告信息，将fstr_content中的"{}"替换为后续的args...参数
            @param fstr_content: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的args...参数替换。
            @param args...: 用于替换fstr_content中"{}"
            @return 生成的信息   */
        template <class... Ts>
        inline std::string messageW(const std::string& fstr_content, const Ts... args)
        {
            return mstr::color("!Warning! ", mstr::Color::yellow) + mstr::fstr(fstr_content, args...);
        }

        /*  生成错误信息，将fstr_content中的"{}"替换为后续的args...参数
            @param fstr_content: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的args...参数替换。
            @param args...: 用于替换fstr_content中"{}"
            @return 生成的信息   */
        template <class... Ts>
        inline std::string messageE(const std::string& fstr_content, const Ts... args)
        {
            return mstr::color("!!!Error!!! ", mstr::Color::red) + mstr::fstr(fstr_content, args...);
        }


        /*  生成正常提示信息，将fstr_content中的"{}"替换为后续的args...参数
            @param str_color: 输出信息的颜色，在mstr::setColorStrOn设置开启时生效
                可选择: black、blue、cyan、fuchsia、green、red、white、yellow
            @param fstr_content: 要生成的信息主体, 如 "hello {}", 其中 "{}"标记将会按顺序被后续的args...参数替换。
            @param args...: 用于替换fstr_content中"{}"
            @return 生成的信息   */
        template <class... Ts>
        inline std::string messageN(mstr::Color str_color, const std::string& fstr_content, const Ts... args)
        {
            return mstr::color(mstr::fstr(fstr_content, args...), str_color);
        }
    }
}

#endif // !LOG_HPP_MINEUTILS