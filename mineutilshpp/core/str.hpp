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
        //将输入转换为std::string，要求参数正确支持std::cout<<操作，可以预设浮点数的精度
        template<uint8_t float_precision = 6, class T, typename std::enable_if<mtype::StdCoutEachChecker<const T&>::value, int>::type = 0>
        inline std::string toStr(const T& arg);

        //当前只实现了简单的f-string功能，将字符串中的"{}"替换为后续的参数，要求参数正确支持std::cout<<操作，可以预设浮点数的精度
        template<uint8_t float_precision = 6, class... Args, typename std::enable_if<mtype::StdCoutEachChecker<int, const Args&...>::value, int>::type = 0>
        std::string format(const std::string& f_string, const Args& ...args);

        //将整数转换为序数词，1st、2nd等
        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
        std::string ordinalize(T number);

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

        //按字符串中的空白字符(包含空格符以及\t\n\r\f\v)分割字符串，若输入空字符串或全空白符串则返回空vector(类Python规则)
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
        template<class T, typename std::enable_if<mtype::ConstructibleFromEachChecker<std::string, const T&>::value, int>::type = 0>
        inline std::string _toStr(std::streamsize, const T& arg)
        {
            return arg;
        }

        template<class T, typename std::enable_if<!mtype::ConstructibleFromEachChecker<std::string, const T&>::value, int>::type = 0>
        inline std::string _toStr(uint8_t float_precision, const T& arg)
        {
            _MINE_THREAD_LOCAL_IF_HAVE std::ostringstream str_buf;

            str_buf.str("");
            str_buf.clear();
            str_buf << std::fixed << std::setprecision(float_precision) << arg;
            return str_buf.str();
        }

        template<uint8_t float_precision, class T, typename std::enable_if<mtype::StdCoutEachChecker<const T&>::value, int>::type>
        inline std::string toStr(const T& arg)
        {
            return mstr::_toStr(float_precision, arg);
        }

        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type>
        inline std::string ordinalize(T number)
        {
            auto num = number + 0;
            if (number == 1)
                return "1st";
            else if (number == 2)
                return "2nd";
            else if (number == 3)
                return "3rd";
            else return mstr::toStr(num).append("th");
        }


        //format函数的相关
        inline void _format(std::ostringstream& oss, const std::string& s, size_t pos_offset)
        {
            oss.write(s.data() + pos_offset, s.length() - pos_offset);
        }

        //format函数的相关
        template<class Arg, class... Args>
        inline void _format(std::ostringstream& oss, const std::string& s, size_t pos_offset, const Arg& arg, const Args& ...args)
        {
            size_t pos = s.find("{}", pos_offset);
            if (pos != std::string::npos)
            {
                oss.write(s.data() + pos_offset, pos - pos_offset);
                oss << arg;
                pos_offset = pos + 2;
                mstr::_format(oss, s, pos_offset, args...);
            }
        }

        template<uint8_t float_precision, class... Args, typename std::enable_if<mtype::StdCoutEachChecker<int, const Args&...>::value, int>::type>
        std::string format(const std::string& f_string, const Args& ...args)
        {
            _MINE_THREAD_LOCAL_IF_HAVE std::ostringstream oss;
            oss.str("");
            oss.clear();
            oss << std::fixed << std::setprecision(float_precision);
            mstr::_format(oss, f_string, 0, args...);
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
                return s;

            size_t left_pos = s.find_first_not_of(chars_to_remove);
            if (left_pos == std::string::npos)
                s.clear();
            else
            {
                s.erase(0, left_pos);
                size_t right_pos = s.find_last_not_of(chars_to_remove);
                s.erase(right_pos + 1, s.size());
            }
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
    }


#ifdef MINEUTILS_TEST_MODULES
    namespace _mstrcheck
    {
        inline void toOrdinalTest()
        {
            bool ret1;
            ret1 = (mstr::ordinalize(1) == "1st" && mstr::ordinalize(2) == "2nd" && mstr::ordinalize(3) == "3rd" && mstr::ordinalize(10) == "10th");
            printf("%s ordinalize check.\n", ret1 ? "Passed." : "Failed!");
            printf("\n");
        }
        inline void toStrTest()
        {
            bool ret1;
            ret1 = (mstr::ordinalize(1) == "1st" && mstr::ordinalize(2) == "2nd" && mstr::ordinalize(3) == "3rd" && mstr::ordinalize(10) == "10th");
            printf("%s ordinalize check:%d.\n", ret1 ? "Passed." : "Failed!", ret1);
            ret1 = (mstr::toStr(1.112) == "1.112000");
            printf("%s toStr(1.112):%s\n", ret1 ? "Passed." : "Failed!", mstr::toStr(1.112).c_str());
            printf("\n");
        }

        //inline void zfillTest()
        //{
        //    bool ret1;
        //    ret1 = (mstr::paddingInt(123, 3, '0') == "123" && mstr::paddingInt(123, 5, '0') == "00123" && mstr::paddingInt(123, 2, '0') == "123");
        //    printf("%s paddingInt(123, 3, '0'):%s  paddingInt(123, 5, '0'):%s  paddingInt(123, 2, '0'):%s\n", ret1 ? "Passed." : "Failed!", 
        //        mstr::paddingInt(123, 3, '0').c_str(), mstr::paddingInt(123, 5, '0').c_str(), mstr::paddingInt(123, 2, '0').c_str());
        //    ret1 = (mstr::paddingFloat(12, 3, 0, '0') == "012" && mstr::paddingFloat(12, 3, 3, '0') == "012.000" && mstr::paddingFloat(12.111111, 3, 3, '0') == "012.111" && mstr::paddingFloat(1234.111111, 3, 3, '0') == "1234.111");
        //    printf("%s mstr::paddingFloat(12, 3, 0, '0'):%s  mstr::paddingFloat(12, 3, 3, '0'):%s  mstr::paddingFloat(12.111111, 3, 3, '0'):%s  mstr::paddingFloat(1234.111111, 3, 3, '0'):%s\n", ret1 ? "Passed." : "Failed!",
        //        mstr::paddingFloat(12, 3, 0, '0').c_str(), mstr::paddingFloat(12, 3, 3, '0').c_str(), mstr::paddingFloat(12.111111, 3, 3, '0').c_str(), mstr::paddingFloat(1234.111111, 3, 3, '0').c_str());
        //    printf("\n");
        //}

        inline void formatTest()
        {
            bool ret1;
            ret1 = (mstr::format("I need {} and {}  ", 1, 2) == "I need 1 and 2  " && mstr::format("I need {}, {} and {}  ", 1, 2) == "I need 1, 2 and {}  " && mstr::format("I need {}  ", 1, 2) == "I need 1  ");
            printf("%s mstr::format check\n", ret1 ? "Passed." : "Failed!");
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
            splitTest();
            trimTest();
            printf("--------------------check mstr end--------------------\n\n");
        }
    }
#endif
}

#endif // !STR_HPP_MINEUTILS