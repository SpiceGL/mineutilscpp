//mineutils库的std::string字符串相关工具
#pragma once
#ifndef STR_HPP_MINEUTILS
#define STR_HPP_MINEUTILS

#include<algorithm>
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

        //将整数转换为序数词，1st、2nd等，只接受正整数
        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
        std::string toOrdinal(T number);

        //将输入转换为std::string，要求参数正确支持std::cout<<操作，对浮点数默认保留6位小数
        template<class T, typename std::enable_if<mtype::StdCoutChecker<const T&>::value, int>::type = 0>
        inline std::string toStr(const T& arg);

        //将输入转换为std::string，可预设浮点数精度，要求参数正确支持std::cout<<操作
        template<class T, typename std::enable_if<mtype::StdCoutChecker<const T&>::value, int>::type = 0>
        inline std::string toStr(std::streamsize float_precision, const T& arg);


        /*  类似Python字符串的zfill函数，且直接将整型的数字转换为字符串并在前面添加字符
            @param n：输入的整型数字
            @param min_len：转换结果的长度
            @param padding：用于填充的字符
            @return 转换结果   */
        std::string zfillInt(long long n, size_t min_len = 0, char padding = '0');

        /*  类似Python字符串的zfill函数，将浮点型的数字转换为字符串并在前面添加字符
            @param f：输入的浮点数字
            @param min_len_int：转换结果整数部分的长度
            @param flt_precision：转换结果小数部分的精度
            @param int_padding：用于填充整数部分的字符
            @return 转换结果   */
        std::string zfillFlt(long double f, size_t min_len_int = 0, std::streamsize flt_precision = 6, char int_padding = '0');

        //实现类似于python的f-string功能，将字符串中的"{}"替换为后续的参数，对于浮点数保留6位小数
        template<class... Args, typename std::enable_if<mtype::StdCoutEachChecker<int, const Args&...>::value, int>::type = 0>
        std::string fstr(const std::string& f_string, const Args& ...args);

        //实现类似于python的f-string功能，将字符串中的"{}"替换为后续的参数，可以预设浮点数的精度
        template<class... Args, typename std::enable_if<mtype::StdCoutEachChecker<int, const Args&...>::value, int>::type = 0>
        std::string fstr(std::streamsize float_precision, const std::string& f_string, const Args& ...args);

        /*  实现正向查找sep对字符串分割的功能，以vector形式返回。分割空字符串会返回包含一个空字符串的vector(类Python规则)
            @param s：待分割的字符串
            @param sep：分割符，不可为空字符串
            @param max_split_times：最大分割次数，-1代表全部分割
            @return 分割结果，至少返回包含一个元素的vector   */
        std::vector<std::string> split(const std::string& s, const std::string& sep, size_t max_split_times = -1);

        /*  实现反向查找sep对字符串分割的功能，以vector形式返回。分割空字符串会返回包含一个空字符串的vector(类Python规则)
            @param s：待分割的字符串
            @param sep：分割符，不可为空字符串
            @param max_split_times：最大分割次数，-1代表全部分割
            @return 分割结果，至少返回包含一个元素的vector，元素排列顺序不会反向   */
        std::vector<std::string> rsplit(const std::string& s, const std::string& sep, size_t max_split_times = -1);

        //按字符串中的空白字符(包含空格符以及\t\n\r\f\v)分割字符串，若输入空字符串或全空格符串则返回空vector(类Python规则)
        std::vector<std::string> split(const std::string& s);

        //去除字符串首尾的指定字符
        std::string trim(std::string s, const char* chars_to_remove = " \t\n\r\f\v");

        //去除字符串首端的指定字符
        std::string ltrim(std::string s, const char* chars_to_remove = " \t\n\r\f\v");

        //去除字符串尾端的指定字符
        std::string rtrim(std::string s, const char* chars_to_remove = " \t\n\r\f\v");
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
            mstr::_getColoStrOn() = ColorStr_on;
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
            if (mstr::_getColoStrOn())
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

        template<class T, typename std::enable_if<mtype::ConstructibleFromEachChecker<std::string, const T&>::value, int>::type = 0>
        inline std::string _toStr(std::streamsize, const T& arg)
        {
            return arg;
        }


        template<class T, typename std::enable_if<!mtype::ConstructibleFromEachChecker<std::string, const T&>::value, int>::type = 0>
        inline std::string _toStr(std::streamsize float_precision, const T& arg)
        {
            MINE_THREAD_LOCAL_IF_HAVE std::ostringstream str_buf;

            str_buf.str("");
            str_buf.clear();
            if (float_precision < 0)
                float_precision = 0;
            str_buf << std::fixed << std::setprecision(float_precision) << arg;
            return str_buf.str();
        }

        template<class T, typename std::enable_if<mtype::StdCoutChecker<const T&>::value, int>::type>
        inline std::string toStr(const T& arg)
        {
            return mstr::_toStr(6, arg);
        }

        template<class T, typename std::enable_if<mtype::StdCoutChecker<const T&>::value, int>::type>
        inline std::string toStr(std::streamsize float_precision, const T& arg)
        {
            return mstr::_toStr(float_precision, arg);
        }

        //将数字转换为序数词，1st、2nd等，只接受正整数，否则会返回空字符串
        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type>
        inline std::string toOrdinal(T number)
        {
            if (number <= 0)
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
            else return mstr::toStr(number).append("th");
        }

        inline std::string zfillInt(long long n, size_t min_len, char padding)
        {
            std::string s = mstr::toStr(n);
            if (s.length() < min_len)
                s.insert(0, min_len - s.length(), padding);
            return s;
        }

        inline std::string zfillFlt(long double f, size_t min_len_int, std::streamsize flt_precision, char int_padding)
        {
            std::string s = mstr::toStr(flt_precision, f);
            size_t point_pos = s.find(".");

            if (point_pos < min_len_int)  //找到"."且整数部分低于目标
                s.insert(0, min_len_int - point_pos, int_padding);
            else if (s.length() < min_len_int)  //没找到"."且整数部分低于目标
                s.insert(0, min_len_int - s.length(), int_padding);
            return s;
        }

        //fstr函数的相关
        inline void _fstr(std::ostringstream& oss, const std::string& s, size_t pos_offset)
        {
            oss.write(s.data() + pos_offset, s.length() - pos_offset);
        }

        //fstr函数的相关
        template<class Arg, class... Args>
        inline void _fstr(std::ostringstream& oss, const std::string& s, size_t pos_offset, const Arg& arg, const Args& ...args)
        {
            size_t pos = s.find("{}", pos_offset);
            if (pos != std::string::npos)
            {
                oss.write(s.data() + pos_offset, pos - pos_offset);
                oss << arg;
                pos_offset = pos + 2;
                mstr::_fstr(oss, s, pos_offset, args...);
            }
        }

        template<class... Args, typename std::enable_if<mtype::StdCoutEachChecker<int, const Args&...>::value, int>::type>
        inline std::string fstr(const std::string& f_string, const Args& ...args)
        {
            return mstr::fstr(6, f_string, args...);
        }

        template<class... Args, typename std::enable_if<mtype::StdCoutEachChecker<int, const Args&...>::value, int>::type>
        std::string fstr(std::streamsize float_precision, const std::string& f_string, const Args& ...args)
        {
            MINE_THREAD_LOCAL_IF_HAVE std::ostringstream oss;
            oss.str("");
            oss.clear();
            if (float_precision < 0)
                float_precision = 0;
            oss << std::fixed << std::setprecision(float_precision);
            mstr::_fstr(oss, f_string, 0, args...);
            return oss.str();
        }

        inline std::vector<std::string> split(const std::string& s, const std::string& sep, size_t max_split_times)
        {
            if (s.empty() || max_split_times == 0)
                return { s };
            if (sep.empty())
            {
                printf("!!!Error!!! \"%s\"[%s](line %d): param sep is empty!\n", MINE_FUNCSIG, __FILE__, __LINE__);
                return { s };
            }
            std::vector<std::string> strs;

            size_t now_split_times = 0;
            size_t start_pos = 0;
            size_t sep_pos;
            while (sep_pos = s.find(sep, start_pos), sep_pos != std::string::npos)
            {
                strs.emplace_back(s.substr(start_pos, sep_pos - start_pos));
                start_pos = sep_pos + sep.length();
                now_split_times++;
                if (now_split_times >= max_split_times)
                    break;
            }
            strs.emplace_back(s.substr(start_pos));
            return strs;
        }


        inline std::vector<std::string> rsplit(const std::string& s, const std::string& sep, size_t max_split_times)
        {
            if (s.empty() || max_split_times == 0)
                return { s };
            if (sep.empty())
            {
                printf("!!!Error!!! \"%s\"[%s](line %d): param sep is empty!\n", MINE_FUNCSIG, __FILE__, __LINE__);
                return { s };
            }
            std::vector<std::string> strs;

            size_t now_split_times = 0;
            size_t start_pos = std::string::npos;
            size_t sep_pos = std::string::npos, tmp_sep_pos;
            while (tmp_sep_pos = s.rfind(sep, start_pos), tmp_sep_pos != std::string::npos)
            {
                sep_pos = tmp_sep_pos;
                strs.emplace(strs.begin(), s.substr(sep_pos + sep.length(), start_pos - (sep_pos + sep.length()) + 1));
                start_pos = sep_pos - 1;
                now_split_times++;
                if (now_split_times >= max_split_times || sep_pos == 0)
                    break;
            }
            strs.emplace(strs.begin(), s.substr(0, sep_pos));
            return strs;
        }

        inline std::vector<std::string> split(const std::string& s)
        {
            if (s.empty())
                return { };

            std::vector<std::string> strs;
            size_t lpos = 0;
            size_t rpos = 0;
            while (lpos != std::string::npos)
            {
                lpos = s.find_first_not_of(" \t\n\r\f\v", rpos);
                if (lpos == std::string::npos)
                    break;
                rpos = s.find_first_of(" \t\n\r\f\v", lpos);
                if (lpos != rpos)
                    strs.emplace_back(s.substr(lpos, rpos - lpos));
            }

            return strs;
        }

        inline std::string trim(std::string s, const char* chars_to_remove)
        {
            if (s.empty())
            {
                return "";
            }

            size_t left_pos = s.find_first_not_of(chars_to_remove);
            if (left_pos == std::string::npos)
                return "";
            s.erase(0, left_pos);
            size_t right_pos = s.find_last_not_of(chars_to_remove);
            s.erase(right_pos + 1, s.size());
            return s;
        }

        inline std::string ltrim(std::string s, const char* chars_to_remove)
        {
            if (s.empty())
                return s;

            size_t left_pos = s.find_first_not_of(chars_to_remove);
            if (left_pos == std::string::npos)
                s.clear();
            else s.erase(0, left_pos);
            return s;
        }

        inline std::string rtrim(std::string s, const char* chars_to_remove)
        {
            if (s.empty())
                return s;
           
            size_t right_pos = s.find_last_not_of(chars_to_remove);
            if (right_pos == std::string::npos)
                s.clear();
            else s.erase(right_pos + 1, s.size());
            return s;
        }



        mdeprecated(R"(Deprecated! Please use an alternative overload(in str.hpp))")
            inline std::string zfillFlt(long double f, size_t min_len_int, size_t flt_precision, char int_padding, char flt_padding)
        {
            std::string s = mstr::toStr(f);

            /*找到输入小数的整数部分和小数部分，分别处理并合并*/
            std::string int_part, flt_part;
            size_t point_pos = s.find(".");
            if (point_pos != std::string::npos)
            {
                int_part = s.substr(0, point_pos);
                flt_part = s.substr(point_pos + 1, flt_precision);
            }
            else
            {
                int_part = std::move(s);
                flt_part.clear();
            }
            s.clear();
            if (int_part.length() < min_len_int)
                s = std::string(min_len_int - int_part.length(), int_padding);
            s.append(int_part);
            if (flt_precision > 0)
                s.append(".").append(flt_part);
            if (flt_precision > flt_part.length())
                s.append(std::string(flt_precision - flt_part.length(), flt_padding));
            return s;
        }

        //废弃实现

        //inline std::vector<std::string> split(const std::string& s)
        //{
        //    if (s.empty())
        //    {
        //        return { };
        //    }

        //    MINE_THREAD_LOCAL_IF_HAVE std::stringstream ss;
        //    ss.str("");
        //    ss.clear();
        //    ss << s;
        //    std::vector<std::string> strs;
        //    std::string s_tmp;
        //    ss >> s_tmp;

        //    while (!s_tmp.empty())
        //    {
        //        strs.emplace_back(std::move(s_tmp));
        //        s_tmp.clear();
        //        ss >> s_tmp;
        //    }
        //    return strs;
        //}

        //inline std::vector<std::string> split(const std::string& s)
        //{
        //    if (s.empty())
        //    {
        //        return { };
        //    }

        //    std::vector<std::string> strs;
        //    std::string::const_iterator it = s.begin();
        //    std::string::const_iterator end = s.end();

        //    while (it != end)
        //    {
        //        it = std::find_if_not(it, end, [](unsigned char c) { return std::isspace(c); });
        //        if (it == end)
        //            break;
        //        auto start = it;
        //        it = std::find_if(start, end, [](unsigned char c) { return std::isspace(c); });

        //        if (start != it)
        //            strs.emplace_back(std::string(start, it));
        //    }

        //    return strs;
        //}
    }


#ifdef MINEUTILS_TEST_MODULES
    namespace _mstr
    {
        inline void toOrdinalTest()
        {
            bool ret1;
            ret1 = (mstr::toOrdinal(1) == "1st" && mstr::toOrdinal(2) == "2nd" && mstr::toOrdinal(3) == "3rd" && mstr::toOrdinal(10) == "10th");
            printf("%s toOrdinal check.\n", ret1 ? "Passed." : "Failed!");
            printf("\n");
        }
        inline void toStrTest()
        {
            bool ret1;
            ret1 = (mstr::toOrdinal(1) == "1st" && mstr::toOrdinal(2) == "2nd" && mstr::toOrdinal(3) == "3rd" && mstr::toOrdinal(10) == "10th");
            printf("%s toOrdinal check:%d.\n", ret1 ? "Passed." : "Failed!", ret1);
            ret1 = (mstr::toStr(1.112) == "1.112000");
            printf("%s toStr(1.112):%s\n", ret1 ? "Passed." : "Failed!", mstr::toStr(1.112).c_str());
            printf("\n");
        }

        inline void zfillTest()
        {
            bool ret1;
            ret1 = (mstr::zfillInt(123, 3, '0') == "123" && mstr::zfillInt(123, 5, '0') == "00123" && mstr::zfillInt(123, 2, '0') == "123");
            printf("%s zfillInt(123, 3, '0'):%s  zfillInt(123, 5, '0'):%s  zfillInt(123, 2, '0'):%s\n", ret1 ? "Passed." : "Failed!", 
                mstr::zfillInt(123, 3, '0').c_str(), mstr::zfillInt(123, 5, '0').c_str(), mstr::zfillInt(123, 2, '0').c_str());
            ret1 = (mstr::zfillFlt(12, 3, 0, '0') == "012" && mstr::zfillFlt(12, 3, 3, '0') == "012.000" && mstr::zfillFlt(12.111111, 3, 3, '0') == "012.111" && mstr::zfillFlt(1234.111111, 3, 3, '0') == "1234.111");
            printf("%s mstr::zfillFlt(12, 3, 0, '0'):%s  mstr::zfillFlt(12, 3, 3, '0'):%s  mstr::zfillFlt(12.111111, 3, 3, '0'):%s  mstr::zfillFlt(1234.111111, 3, 3, '0'):%s\n", ret1 ? "Passed." : "Failed!",
                mstr::zfillFlt(12, 3, 0, '0').c_str(), mstr::zfillFlt(12, 3, 3, '0').c_str(), mstr::zfillFlt(12.111111, 3, 3, '0').c_str(), mstr::zfillFlt(1234.111111, 3, 3, '0').c_str());
            printf("\n");
        }

        inline void fstrTest()
        {
            bool ret1;
            ret1 = (mstr::fstr("I need {} and {}  ", 1, 2) == "I need 1 and 2  " && mstr::fstr("I need {}, {} and {}  ", 1, 2) == "I need 1, 2 and {}  " && mstr::fstr("I need {}  ", 1, 2) == "I need 1  ");
            printf("%s mstr::fstr check\n", ret1 ? "Passed." : "Failed!");
            printf("\n");
        }

        inline void splitTest()
        {
            bool ret1;
            ret1 = (mstr::split("I need {} and {}  ") == std::vector<std::string>({ "I", "need", "{}", "and", "{}" }) && mstr::split("I need {} and {}  ", "{}") == std::vector<std::string>({ "I need ", " and ", "  " }));
            printf("%s mstr::split check1\n", ret1 ? "Passed." : "Failed!");

            ret1 = (mstr::split("I need {} and {}  ", "x") == std::vector<std::string>({ "I need {} and {}  " }) && mstr::split("I need {} and {}  ", "{}", 1) == std::vector<std::string>({ "I need ", " and {}  " }));
            printf("%s mstr::split check2\n", ret1 ? "Passed." : "Failed!");


            ret1 = (mstr::rsplit("I need {} and {}  ", "{}") == std::vector<std::string>({ "I need ", " and ", "  " }) );
            printf("%s mstr::rsplit check1\n", ret1 ? "Passed." : "Failed!");

            ret1 = (mstr::rsplit("I need {} and {}  ", "x") == std::vector<std::string>({ "I need {} and {}  " }) && mstr::rsplit("I need {} and {}  ", "{}", 1) == std::vector<std::string>({ "I need {} and ", "  " }));
            printf("%s mstr::rsplit check2\n", ret1 ? "Passed." : "Failed!");
            printf("\n");
        }

        inline void trimTest()
        {
            bool ret1;
            ret1 = (mstr::trim(" \n\t 123 \v\r") == "123" && mstr::trim(" \n\t 123") == "123", mstr::trim("123 \v\r") == "123");
            printf("%s mstr::trim check\n", ret1 ? "Passed." : "Failed!");

            ret1 = (mstr::ltrim(" \n\t 123 \v\r") == "123 \v\r" && mstr::ltrim(" \n\t 123") == "123", mstr::ltrim("123 \v\r") == "123 \v\r");
            printf("%s mstr::ltrim check\n", ret1 ? "Passed." : "Failed!");

            ret1 = (mstr::rtrim(" \n\t 123 \v\r") == " \n\t 123" && mstr::rtrim(" \n\t 123") == " \n\t 123", mstr::rtrim("123 \v\r") == "123");
            printf("%s mstr::rtrim check\n", ret1 ? "Passed." : "Failed!");
            printf("\n");
        }

        inline void check()
        {
            printf("\n--------------------check mstr start--------------------\n\n");
            toOrdinalTest();
            toStrTest();
            zfillTest();
            splitTest();
            trimTest();
            printf("--------------------check mstr end--------------------\n\n");
        }
    }
#endif
}

#endif // !STR_HPP_MINEUTILS