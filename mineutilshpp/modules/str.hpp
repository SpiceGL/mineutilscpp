//mineutils库的std::string字符串相关工具
#pragma once
#ifndef STR_HPP_MINEUTILS
#define STR_HPP_MINEUTILS

#include<atomic>
#include<iomanip>
#include<iostream>
#include<sstream>
#include<string>
#include<vector>

#include"base.hpp"
#include"type.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mstr
    {
        enum class Color
        {
            black = 0,
            red = 1,
            green = 2,
            yellow = 3,
            blue = 4,
            purple = 5,
            cyan = 6,
            white = 7
        };

        //为字符串添加颜色标记，因不同终端互不兼容，效果默认关闭
        std::string color(const std::string& str, const mstr::Color str_color);

        //设置是否开启彩色字体的效果
        //部分终端不一定支持彩色字体显示，因此默认关闭彩色字体
        void setColorStrOn(bool ColorStr_on);

        //将数字转换为序数词，1st、2nd等，只接受正整数
        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
        std::string toOrdinal(T number);

        //将输入直接转换为字符串
        template<class T, typename std::enable_if<mtype::StdCoutChecker<T>::value, int>::type = 0>
        std::string toStr(const T& arg);

        /*  类似Python字符串的zfill函数，将整型的数字转换为字符串并在前面添加字符
            @param n：输入的整型数字
            @param min_len：转换结果的长度
            @param padding：用于填充的字符
            @return 转换结果   */
        std::string zfillInt(long long n, unsigned int min_len = 0, char padding = '0');

        /*  类似Python字符串的zfill函数，将浮点型的数字转换为字符串并在前后添加字符
            @param f：输入的浮点数字
            @param min_len_int：转换结果整数部分的长度
            @param flt_precision：转换结果小数部分的精度
            @param int_padding：用于填充整数部分的字符
            @param flt_padding：用于填充小数部分的字符
            @return 转换结果   */
        std::string zfillFlt(long double f, unsigned int min_len_int = 0, unsigned int flt_precision = 4,
            char int_padding = ' ', char flt_padding = '0');

        //实现类似于python的f-string功能，将字符串中的"{}"替换为后续的参数
        template<class... Ts>
        std::string fstr(std::string s, const Ts& ...args);

        /*  实现正向查找sep对字符串分割的功能，以vector形式返回。分割空字符串会返回包含一个空字符串的vector(类Python规则)
            @param s：待分割的字符串
            @param sep：分割符，不可为空字符串
            @param max_split_times：最大分割次数，-1代表全部分割
            @return 分割结果，至少返回包含一个元素的vector   */
        std::vector<std::string> split(std::string s, const std::string& sep, size_t max_split_times = -1);

        /*  实现反向查找sep对字符串分割的功能，以vector形式返回。分割空字符串会返回包含一个空字符串的vector(类Python规则)
            @param s：待分割的字符串
            @param sep：分割符，不可为空字符串
            @param max_split_times：最大分割次数，-1代表全部分割
            @return 分割结果，至少返回包含一个元素的vector   */
        std::vector<std::string> rsplit(std::string s, const std::string& sep, size_t max_split_times = -1);

        //按字符串中的空白符（包括空格、多空格、\n、\t等）分割字符串，若输入空字符串或全空格符串则返回空vector(类Python规则)
        std::vector<std::string> split(std::string s);
    }






    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mstr
    {
        inline std::atomic<bool>& _getColoStrOn()
        {
            static std::atomic<bool> ColorStr_On(false);
            return ColorStr_On;
        }

        inline void setColorStrOn(bool ColorStr_on)
        {
            _getColoStrOn() = ColorStr_on;
        }



        inline std::string color(const std::string& str, const mstr::Color str_color)
        {
            /*
            字色              背景              颜色
            ---------------------------------------
            30                40              黑色
            31                41              红色
            32                42              绿色
            33                43              黃色
            34                44              蓝色
            35                45              紫红色
            36                46              青蓝色
            37                47              白色
            ——————————————————  */
            if (_getColoStrOn())
            {
                if (str_color == mstr::Color::black)
                    return "\033[0;30m" + str + "\033[0m";
                else if (str_color == mstr::Color::red)
                    return "\033[0;31m" + str + "\033[0m";
                else if (str_color == mstr::Color::green)
                    return "\033[0;32m" + str + "\033[0m";
                else if (str_color == mstr::Color::yellow)
                    return "\033[0;33m" + str + "\033[0m";
                else if (str_color == mstr::Color::blue)
                    return "\033[0;34m" + str + "\033[0m";
                else if (str_color == mstr::Color::purple)
                    return "\033[0;35m" + str + "\033[0m";
                else if (str_color == mstr::Color::cyan)
                    return "\033[0;36m" + str + "\033[0m";
                else if (str_color == mstr::Color::white)
                    return "\033[0;37m" + str + "\033[0m";
            }
            return str;
        }

        //将数字转换为序数词，1st、2nd等，只接受正整数，否则会返回空字符串
        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type>
        inline std::string toOrdinal(T number)
        {
            if(number <= 0)
            { 
                printf("!Warning! \"%s\"[%s](line %d): Param number:%d must be positive integer!\n", MINE_FUNCSIG, __FILE__, __LINE__, number);
                return "";
            }
            if (number == 1)
                return "1st";
            else if (number == 2)
                return "2nd";
            else if (number == 3)
                return "3rd";
            else return mstr::toStr(number) + "th";
        }

        template<class T, typename std::enable_if<mtype::StdCoutChecker<T>::value, int>::type>
        inline std::string toStr(const T& arg)
        {
            MINE_THREAD_LOCAL std::ostringstream str_buf;
            str_buf.clear();
            str_buf.str("");
            str_buf << arg;
            return str_buf.str();
        }

        inline std::string zfillInt(long long n, unsigned int min_len, char padding)
        {
            std::string s = toStr(n);
            if (s.length() < min_len)
            {
                s = std::string(min_len - s.length(), padding) + s;
            }
            return s;
        }

        inline std::string zfillFlt(long double f, unsigned int min_len_int, unsigned int flt_precision,
            char int_padding, char flt_padding)
        {
            //static_assert(std::is_floating_point<FT>::value, "Class FT must be floating_point!");
            MINE_THREAD_LOCAL std::ostringstream buffer;
            buffer.clear();
            buffer << std::setprecision(flt_precision) << f;
            std::string s = buffer.str();

            /*找到输入小数的整数部分和小数部分，分别处理并合并*/
            std::string int_part, flt_part;
            size_t point_pos = s.find(".");
            if (point_pos != std::string::npos)
            {
                int_part = s.substr(0, point_pos);
                flt_part = s.substr(point_pos + 1);
            }
            else
            {
                int_part = s;
                flt_part = "";
            }
            if (int_part.length() < min_len_int)
                int_part = std::string(min_len_int - int_part.length(), int_padding) + int_part;
            if (flt_part.length() < flt_precision)
                flt_part = flt_part + std::string(flt_precision - flt_part.length(), flt_padding);
            s = int_part + "." + flt_part;
            return s;
        }


        //fstr函数的相关
        inline std::string _fstr(std::string& s, size_t)
        {
            return s;
        }

        //fstr函数的相关
        template<class T, class... Ts>
        inline std::string _fstr(std::string& s, size_t pos_offset, const T& arg, const Ts&... args)
        {
            size_t pos = s.find("{}", pos_offset);
            if (pos != std::string::npos)
            {
                std::string arg_s = mstr::toStr(arg);
                s.replace(pos, 2, arg_s);
                pos_offset = pos + arg_s.size();
                return mstr::_fstr(s, pos_offset, args...);
            }
            else return s;            
        }

        template<class... Ts>
        inline std::string fstr(std::string s, const Ts& ...args)
        {
            s.reserve(s.size() + 64);
            return mstr::_fstr(s, 0, args...);
        }

        inline std::vector<std::string> split(std::string s, const std::string& sep, size_t max_split_times)
        {
            if (s.empty() || max_split_times == 0)
            {
                return { s };
            }
            if (sep.empty())
            {
                printf("!!!Error!!! \"%s\"[%s](line %d): param sep is empty!\n", MINE_FUNCSIG, __FILE__, __LINE__);
                return { s };
            }
            std::vector<std::string> strs;
            size_t sep_pos;
            std::string s_tmp;   //s_tmp存放已处理的字段，s存放待处理的字段

            size_t now_split_times = 0;
            while ((sep_pos = s.find(sep)) != std::string::npos)
            {
                s_tmp = s.substr(0, sep_pos);
                s = s.substr(sep_pos + sep.length());
                strs.emplace_back(s_tmp);
                s_tmp.clear();
                now_split_times++;
                if (now_split_times >= max_split_times)
                    break;
            }
            strs.emplace_back(s);
            return strs;
        }

        inline std::vector<std::string> rsplit(std::string s, const std::string& sep, size_t max_split_times)
        {
            if (s.empty() || max_split_times == 0)
            {
                return { s };
            }
            if (sep.empty())
            {
                printf("!!!Error!!! \"%s\"[%s](line %d): param sep is empty!\n", MINE_FUNCSIG, __FILE__, __LINE__);
                return { s };
            }
            std::vector<std::string> strs;
            size_t sep_pos;
            std::string s_tmp;   //s_tmp存放已处理的字段，s存放待处理的字段

            size_t now_split_times = 0;
            while ((sep_pos = s.rfind(sep)) != std::string::npos)
            {
                s_tmp = s.substr(sep_pos + sep.length());
                s = s.substr(0, sep_pos);
                strs.emplace(strs.begin(), s_tmp);
                s_tmp.clear();
                now_split_times++;
                if (now_split_times >= max_split_times)
                    break;
            }
            strs.emplace(strs.begin(), s);
            return strs;
        }

        inline std::vector<std::string> split(std::string s)
        {
            if (s.empty())
            {
                return { };
            }

            MINE_THREAD_LOCAL std::stringstream ss;

            ss.clear();
            ss << s;
            std::vector<std::string> strs;
            std::string s_tmp;
            ss >> s_tmp;

            while (!s_tmp.empty())
            {
                strs.emplace_back(s_tmp);
                s_tmp.clear();
                ss >> s_tmp;
            }
            return strs;
        }
    }
}

#endif // !STR_HPP_MINEUTILS