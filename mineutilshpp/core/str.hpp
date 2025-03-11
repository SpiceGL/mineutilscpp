//mineutils库的std::string字符串相关工具
#pragma once
#ifndef STR_HPP_MINEUTILS
#define STR_HPP_MINEUTILS

#include<algorithm>
#include<array>
#include<atomic>
#include<deque>
#include<exception>
#include<forward_list>
#include<initializer_list>
#include<iomanip>
#include<iostream>
#include<list>
#include<map>
#include<mutex>
#include<queue>
#include<set>
#include<sstream>
#include<stack>
#include<stdio.h>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>

#include"base.hpp"
#include"type.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mstr
    {
        /*  将输入转换为std::string
            - 基于std::ostringstream实现，可以接收任意类型arg
            - 支持重载了std::ostream& operator<<(std::ostream&, const T&)的T对象
            - 扩展了对非volatile限定的STL容器对象的支持
            - 有无符号的char都会被当作字符处理
            - 宽字符会被当作数字处理
            - 未支持的类型会被转换为<ClassName: Address>形式的字符串  
            - float_precision为负时表示自由精度，可能会用科学计数法表示  */
        template<int8_t float_precision = -1, class T>
        std::string toStr(const T& arg);

        /*  将字符串中的"{}"替换为后续的参数
            - 基于std::ostringstream实现，可以接收任意类型arg
            - 支持重载了std::ostream& operator<<(std::ostream&, const T&)的T对象
            - 扩展了对非volatile限定的STL容器对象的支持
            - 有无符号的char都会被当作字符处理
            - 宽字符会被当作数字处理
            - 未支持的类型会被转换为<ClassName: Address>形式的字符串  
            - float_precision为负时表示自由精度，可能会用科学计数法表示  */
        template<int8_t float_precision = -1, class... Args>
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

        //在字符串首端添加字符至目标长度
        std::string lpad(std::string s, size_t target_len, const char pad_char);
        //在字符串尾端添加字符至目标长度
        std::string rpad(std::string s, size_t target_len, const char pad_char);
    }










    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mstr
    {
        template<class T>
        void _osInput(std::ostream& oss, std::initializer_list<T> arg);

        template<class T, size_t n>
        void _osInput(std::ostream& oss, const std::array<T, n>& arr);

        template<class T, class...Ts>
        void _osInput(std::ostream& oss, const std::stack<T, Ts...>& st);

        void _osInput(std::ostream& oss, const std::string& str);

        //void _osInput(std::ostream& oss, const std::wstring& str);

        template<class KeyT, class VT, class... Ts>
        void _osInput(std::ostream& oss, const std::map<KeyT, VT, Ts...>& m);

        template<class KeyT, class VT, class... Ts>
        void _osInput(std::ostream& oss, const std::multimap<KeyT, VT, Ts...>& m);

        template<class KeyT, class VT, class... Ts>
        void _osInput(std::ostream& oss, const std::unordered_map<KeyT, VT, Ts...>& m);

        template<class KeyT, class VT, class... Ts>
        void _osInput(std::ostream& oss, const std::unordered_multimap<KeyT, VT, Ts...>& m);

        template<template<class KeyT, class VT, class... Ts> class CTer, class KeyT, class VT, class... Ts>
        void _osInputMap(std::ostream& oss, const CTer<KeyT, VT, Ts...>& m);

        //按照出队顺序打印
        template<class T, class...Ts>
        void _osInput(std::ostream& oss, const std::priority_queue<T, Ts...>& qe);

        template<class T1, class T2>
        void _osInput(std::ostream& oss, const std::pair<T1, T2>& pa);

        template<size_t idx = 0, class... Ts>
        void _osInput(std::ostream& oss, const std::tuple<Ts...>& tp);

        template<size_t idx, class... Ts>
        void _osInputTuple(std::ostream& oss, const std::tuple<Ts...>& tp, size_t size_tp, std::true_type);

        template<size_t idx, class... Ts>
        void _osInputTuple(std::ostream& oss, const std::tuple<Ts...>&, size_t, std::false_type);

        template<template<class U, class... Us> class CTer, class T, class... Ts, typename std::enable_if<mtype::_StdBeginEndChecker<const CTer<T, Ts...>>::value, int>::type = 0>
        void _osInput(std::ostream& oss, const CTer<T, Ts...>& cter);

        template<class T, int n>
        void _osInput(std::ostream& oss, const T(&arr)[n]);

        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && (std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value), int>::type = 0>
        void _osInput(std::ostream& oss, const T& arg);

        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && !(std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value) && std::is_pointer<T>::value&& mtype::InTypesChecker<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char, signed char, unsigned char>::value, int>::type = 0>
        void _osInput(std::ostream& oss, const T& arg);

        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && !(std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value) && std::is_pointer<T>::value&& !mtype::InTypesChecker<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char, signed char, unsigned char>::value, int>::type = 0>
        void _osInput(std::ostream& oss, const T& arg);

        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && !(std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value) && !std::is_pointer<T>::value, int>::type = 0>
        void _osInput(std::ostream& oss, const T& arg);

        template<class T, typename std::enable_if<!mtype::StdCoutEachChecker<const T>::value, int>::type = 0>
        void _osInput(std::ostream& oss, const T& arg);


        template<int8_t float_precision, class T, typename std::enable_if<!mtype::ConstructibleFromEachChecker<std::string, const T&>::value, int>::type = 0>
        inline std::string _toStrDispath(const T& arg)
        {
            _MINE_THREAD_LOCAL_IF_HAVE std::ostringstream oss;
            oss.clear();
            oss.str("");
            if (float_precision >= 0)
                oss << std::fixed << std::setprecision(float_precision);
            //else oss << std::fixed;
            mstr::_osInput(oss, arg);
            return oss.str();
        }

        template<int8_t float_precision, class T, typename std::enable_if<mtype::ConstructibleFromEachChecker<std::string, const T&>::value, int>::type = 0>
        inline std::string _toStrDispath(const T& arg)
        {
            return arg;
        }

        template<int8_t float_precision, class T>
        inline std::string toStr(const T& arg)
        {
            return mstr::_toStrDispath<float_precision>(arg);
        }

        template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type>
        inline std::string ordinalize(T number)
        {
            auto num = number + 0;   //将字符型当作整型
            const int abs_num = std::abs(num);

            if ((abs_num % 10 == 1) && (abs_num % 100 != 11))
                return mstr::toStr(num).append("st");
            else if ((abs_num % 10 == 2) && (abs_num % 100 != 12))
                return mstr::toStr(num).append("nd");
            else if ((abs_num % 10 == 3) && (abs_num % 100 != 13))
                return mstr::toStr(num).append("rd");
            else return mstr::toStr(num).append("th");
        }


        //format函数的相关
        inline void _format(std::ostream& oss, const std::string& s, size_t pos_offset)
        {
            oss.write(s.data() + pos_offset, s.length() - pos_offset);
        }

        //format函数的相关
        template<class Arg, class... Args>
        inline void _format(std::ostream& oss, const std::string& s, size_t pos_offset, const Arg& arg, const Args& ...args)
        {
            size_t pos = s.find("{}", pos_offset);
            if (pos != std::string::npos)
            {
                oss.write(s.data() + pos_offset, pos - pos_offset);
                mstr::_osInput(oss, arg);
                pos_offset = pos + 2;
                mstr::_format(oss, s, pos_offset, args...);
            }
        }


        template<int8_t float_precision, class... Args>
        inline std::string format(const std::string& f_string, const Args& ...args)
        {
            _MINE_THREAD_LOCAL_IF_HAVE std::ostringstream oss;
            oss.str("");
            oss.clear();
            if (float_precision >= 0)
                oss << std::fixed << std::setprecision(float_precision);
            //else oss << std::fixed;
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

        inline std::string lpad(std::string s, size_t target_len, const char pad_char)
        {
            if (s.size() < target_len)
                s.insert(0, target_len - s.size(), pad_char);
            return s;
        }

        inline std::string rpad(std::string s, size_t target_len, const char pad_char)
        {
            if (s.size() < target_len)
                s.insert(s.size(), target_len - s.size(), pad_char);
            return s;
        }

        template<class T>
        void _osInput(std::ostream& oss, std::initializer_list<T> arg)
        {
            oss << "{";
            for (auto it = arg.begin(); it != arg.end(); )
            {
                mstr::_osInput(oss, *it);
                it++;
                if (it != arg.end())
                    oss << ", ";
            }
            oss << "}";
        }

        //添加对std::array类型的支持
        template<class T, size_t n>
        inline void _osInput(std::ostream& oss, const std::array<T, n>& arr)
        {
            oss << "{";
            if (n > 0)
            {
                auto bg = arr.begin();
                for (int i = 0; i < n - 1; ++i)
                {
                    mstr::_osInput(oss, *bg);
                    oss << ", ";
                    ++bg;
                }
                mstr::_osInput(oss, *bg);
            }
            oss << "}";
        }

        template<class T, class...Ts>
        inline void _osInput(std::ostream& oss, const std::stack<T, Ts...>& st)
        {
            auto tmp_st = st;
            oss << "{";
            size_t size = tmp_st.size();
            if (size > 0)
            {
                for (size_t i = 0; i < size - 1; ++i)
                {
                    mstr::_osInput(oss, tmp_st.top());
                    tmp_st.pop();
                    oss << ", ";
                }
                mstr::_osInput(oss, tmp_st.top());
            }
            oss << "}";
        }

        inline void _osInput(std::ostream& oss, const std::string& str)
        {
            oss << str;
        }

        //inline void _osInput(std::ostream& oss, const std::wstring& str)
        //{
        //    oss << "<" << mtype::getTypeName<std::wstring>() << ": " << std::showbase << std::hex << uintptr_t(&str) << std::dec << ">";
        //}

        template<class T, class...Ts>
        inline void _osInput(std::ostream& oss, const std::priority_queue<T, Ts...>& qe)
        {
            auto tmp_qe = qe;
            oss << "{";
            size_t size = tmp_qe.size();
            if (size > 0)
            {
                for (size_t i = 0; i < size - 1; ++i)
                {
                    mstr::_osInput(oss, tmp_qe.top());
                    tmp_qe.pop();
                    oss << ", ";
                }
                mstr::_osInput(oss, tmp_qe.top());
            }
            oss << "}";
        }

        //添加对std::pair类型的支持
        template<class T1, class T2>
        inline void _osInput(std::ostream& oss, const std::pair<T1, T2>& pa)
        {
            oss << "{";
            mstr::_osInput(oss, pa.first);
            oss << ", ";
            mstr::_osInput(oss, pa.second);
            oss << "}";
        }

        //添加对std::tuple类型的支持
        template<size_t idx, class... Ts>
        inline void _osInput(std::ostream& oss, const std::tuple<Ts...>& tp)
        {
            constexpr size_t size_tp = std::tuple_size<std::tuple<Ts...>>::value;
            if (idx == 0)
                oss << "{";
            mstr::_osInputTuple<idx, Ts...>(oss, tp, size_tp, std::integral_constant<bool, (idx < size_tp) >());
        }

        template<size_t idx, class... Ts>
        inline void _osInputTuple(std::ostream& oss, const std::tuple<Ts...>& tp, size_t size_tp, std::true_type)
        {
            if (idx < size_tp - 1)
            {
                mstr::_osInput(oss, std::get<idx>(tp));
                oss << ", ";
                mstr::_osInput<idx + 1, Ts...>(oss, tp);
            }
            else if (idx == size_tp - 1)
            {
                mstr::_osInput(oss, std::get<idx>(tp));
                mstr::_osInput<idx + 1, Ts...>(oss, tp);
            }
            else throw "Error idx!";
        }

        template<size_t idx, class... Ts>
        inline void _osInputTuple(std::ostream& oss, const std::tuple<Ts...>&, size_t, std::false_type)
        {
            oss << "}";
        }

        template<template<class KeyT, class VT, class... Ts> class CTer, class KeyT, class VT, class... Ts>
        inline void _osInputMap(std::ostream& oss, const CTer<KeyT, VT, Ts...>& m)
        {
            oss << "{";
            size_t size = m.size();
            if (size > 0)
            {
                auto it = m.begin();
                for (size_t i = 0; i < size - 1; ++i)
                {
                    mstr::_osInput(oss, it->first);
                    oss << ":";
                    mstr::_osInput(oss, it->second);
                    oss << ", ";
                    ++it;
                }
                mstr::_osInput(oss, it->first);
                oss << ":";
                mstr::_osInput(oss, it->second);
            }
            oss << "}";
        }

        //添加对std::map类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _osInput(std::ostream& oss, const std::map<KeyT, VT, Ts...>& m)
        {
            mstr::_osInputMap(oss, m);
        }

        //添加对std::multimap类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _osInput(std::ostream& oss, const std::multimap<KeyT, VT, Ts...>& m)
        {
            mstr::_osInputMap(oss, m);
        }


        //添加对std::unordered_map类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _osInput(std::ostream& oss, const std::unordered_map<KeyT, VT, Ts...>& m)
        {
            mstr::_osInputMap(oss, m);
        }

        //添加对std::unordered_multimap类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _osInput(std::ostream& oss, const std::unordered_multimap<KeyT, VT, Ts...>& m)
        {
            mstr::_osInputMap(oss, m);
        }

        //添加对STL标准容器的支持
        template<template<class U, class... Us> class CTer, class T, class... Ts, typename std::enable_if<mtype::_StdBeginEndChecker<const CTer<T, Ts...>>::value, int>::type>
        inline void _osInput(std::ostream& oss, const CTer<T, Ts...>& cter)   //虽然用了双层模板，但单层也可以达成目的
        {
            oss << "{";
            for (auto it = cter.begin(); it != cter.end(); )
            {
                mstr::_osInput(oss, *it);
                it++;
                if (it != cter.end())
                    oss << ", ";
            }
            oss << "}";
        }

        //添加对数组类型的支持
        template<class T, int n>
        inline void _osInput(std::ostream& oss, const T(&arr)[n])
        {
            if (mtype::InTypesChecker<T, char, signed char, unsigned char>::value)
                oss << arr;
            else if (mtype::InTypesChecker<T, volatile char, volatile signed char, volatile unsigned char>::value)
            {
                for (int i = 0; i < n - 1; i++)
                {
                    mstr::_osInput(oss, arr[i]);
                }
            }
            else
            {
                oss << "{";
                for (int i = 0; i < n - 1; i++)
                {
                    mstr::_osInput(oss, arr[i]);
                    oss << ", ";
                }
                mstr::_osInput(oss, arr[n - 1]);
                oss << "}";
            }
        }

        //添加对函数及函数指针的支持
        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && (std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value), int>::type>
        inline void _osInput(std::ostream& oss, const T& arg)
        {
            oss << mtype::getTypeName<T>();
        }

        //添加对char指针的支持
        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && !(std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value) && std::is_pointer<T>::value && mtype::InTypesChecker<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char, signed char, unsigned char>::value, int>::type>
        inline void _osInput(std::ostream& oss, const T& arg)
        {
            //remove_pointer对多级指针只会移除一层
            if (mtype::InTypesChecker<typename std::remove_const<typename std::remove_pointer<T>::type>::type, char, signed char, unsigned char>::value)
                oss << arg;
            else
            {
                for (int i = 0; arg[i] != '\0'; i++)
                {
                    oss << char(arg[i]);
                }
            }
        }

        //添加对其他指针的支持
        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && !(std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value) && std::is_pointer<T>::value&& !mtype::InTypesChecker<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char, signed char, unsigned char>::value, int>::type>
        inline void _osInput(std::ostream& oss, const T& arg)
        {
            oss << std::showbase << std::hex << uintptr_t(arg) << std::dec;
        }

        //添加对其他支持operator<<的类型的支持
        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<const T>::value && !(std::is_function<typename std::remove_pointer<const T>::type>::value || std::is_member_function_pointer<const T>::value) && !std::is_pointer<T>::value, int>::type>
        inline void _osInput(std::ostream& oss, const T& arg)
        {
            oss << arg;
        }

        //扩展其他不支持operator<<的类型
        template<class T, typename std::enable_if<!mtype::StdCoutEachChecker<const T>::value, int>::type>
        inline void _osInput(std::ostream& oss, const T& arg)
        {
            oss << "<" << mtype::getTypeName<T>() << ": " << std::showbase << std::hex << uintptr_t(&arg) << std::dec << ">";
        }


    }



#ifdef MINEUTILS_TEST_MODULES
    namespace _mstrcheck
    {
        inline void ordinalizeTest()
        {
            bool ret0;
            ret0 = (mstr::ordinalize(1) == "1st" && mstr::ordinalize(2) == "2nd" && mstr::ordinalize(3) == "3rd" && mstr::ordinalize(10) == "10th");
            if (!ret0) mprintfE(R"(Failed when check: mstr::ordinalize(1) == "1st" && mstr::ordinalize(2) == "2nd" && mstr::ordinalize(3) == "3rd" && mstr::ordinalize(10) == "10th")""\n");
            ret0 = (mstr::ordinalize(21) == "21st" && mstr::ordinalize(22) == "22nd" && mstr::ordinalize(-23) == "-23rd");
            if (!ret0) mprintfE(R"(Failed when check: mmstr::ordinalize(21) == "21st" && mstr::ordinalize(22) == "22nd" && mstr::ordinalize(-23) == "-23rd")""\n");
            ret0 = (mstr::ordinalize(11) == "11th" && mstr::ordinalize(12) == "12th" && mstr::ordinalize(-13) == "-13th");
            if (!ret0) mprintfE(R"(Failed when check: mstr::ordinalize(11) == "11th" && mstr::ordinalize(12) == "12th" && mstr::ordinalize(-13) == "-13th")""\n");
        }
        inline void toStrTest()
        {
            bool ret0;
            ret0 = (mstr::toStr(std::vector<int>({ 1, 2, 3 })) == "{1, 2, 3}");
            if (!ret0) mprintfE(R"(Failed when check: mstr::toStr(std::vector<int>({ 1, 2, 3 })) == "{1, 2, 3}")""\n");

            ret0 = (mstr::toStr(std::vector<std::map<int, int>>({ {{1, 2}, {3, 4}} })) == "{{1:2, 3:4}}");
            if (!ret0) mprintfE(R"(Failed when check: mstr::toStr(std::vector<std::map<int, int>>({ {{1, 2}, {3, 4}} })) == "{{1:2, 3:4}}"")""\n");
            auto tp = std::tuple<std::map<int, int>>({ {1, 2}, {3, 4} });
            ret0 = (mstr::toStr(tp) == "{{1:2, 3:4}}");
            if (!ret0) mprintfE(R"(Failed when check: mstr::toStr(std::tuple<std::map<int, int>>({ {{1, 2}, {3, 4}} })) == "{{1:2, 3:4}}")""\n");

            const char* s1 = "hello";
            const char volatile* s2 = "hello";
            const char* volatile s3 = "hello";
            const wchar_t* s4 = L"hello";
            const char* const s5 = "hello";
            volatile const char s6[] = "hello";
            const char* s7 = nullptr;   //处理这种非法地址会直接异常
            std::wstring s8 = L"hello";
            const unsigned char* s9 = (const unsigned char*)"hello";
            const char* s10[] = { s1 };
            const char** s11 = &s1;

            ret0 = (mstr::toStr(s1) == "hello") && (mstr::toStr(s2) == "hello") && (mstr::toStr(s3) == "hello");
            if (!ret0) mprintfE(R"(Failed when check: (mstr::toStr(s1) == "hello") && (mstr::toStr(s2) == "hello") && (mstr::toStr(s3) == "hello"))""\n");
            ret0 = (mstr::toStr(s5) == "hello") && (mstr::toStr(s6) == "hello") && (mstr::toStr(s9) == "hello");
            if (!ret0) mprintfE(R"(Failed when check: (mstr::toStr(s5) == "hello") && (mstr::toStr(s6) == "hello") && (mstr::toStr(s9) == "hello"))""\n");
            ret0 = (mstr::toStr(s10) == "{hello}");
            if (!ret0) mprintfE(R"(Failed when check: mstr::toStr(s10) == "{hello}")""\n");

            printf("User check: mstr::Got toStr(s4) result(0x...):%s, mstr::toStr(s8) result({104, 101, 108, 108, 111}):%s\n", mstr::toStr(s4).c_str(), mstr::toStr(s8).c_str());
            printf("User check: mstr::Got toStr(s11) result(0x...):%s\n", mstr::toStr(s11).c_str());
        }

        inline void formatTest()
        {
            bool ret0;
            ret0 = (mstr::format("I need {} and {}  ", 1, 2) == "I need 1 and 2  ");
            if (!ret0) mprintfE(R"(Failed when check: mstr::format("I need {} and {}  ", 1, 2) == "I need 1 and 2  ")""\n");
            ret0 = (mstr::format("I need {}, {} and {}  ", 1, 2) == "I need 1, 2 and {}  ");
            if (!ret0) mprintfE(R"(Failed when check: mstr::format("I need {}, {} and {}  ", 1, 2) == "I need 1, 2 and {}  "")""\n");
            ret0 = (mstr::format("I need {}  ", 1, 2) == "I need 1  ");
            if (!ret0) mprintfE(R"(Failed when check: mstr::format("I need {}  ", 1, 2) == "I need 1  ")""\n");

            ret0 = (mstr::format("vector: {}", std::vector<std::vector<int>>({ {0, 1}, {2, 3} })) == "vector: {{0, 1}, {2, 3}}");
            if (!ret0) mprintfE(R"(Failed when check: mstr::format("vector: {}", std::vector<std::vector<int>>({ {0, 1}, {2, 3} })) == "vector: {{0, 1}, {2, 3}}")""\n");
        }

        inline void splitTest()
        {
            bool ret0;
            ret0 = (mstr::split("I need {} and {}  ") == std::vector<std::string>({ "I", "need", "{}", "and", "{}" }));
            if (!ret0) mprintfE(R"(Failed when check: mstr::split("I need {} and {}  ") == std::vector<std::string>({ "I", "need", "{}", "and", "{}" }))""\n");
            ret0 = (mstr::split("I need {} and {}  ", "{}") == std::vector<std::string>({ "I need ", " and ", "  " }));
            if (!ret0) mprintfE(R"(Failed when check: mstr::split("I need {} and {}  ", "{}") == std::vector<std::string>({ "I need ", " and ", "  " }))""\n");

            ret0 = (mstr::split("I need {} and {}  ", "x") == std::vector<std::string>({ "I need {} and {}  " }));
            if (!ret0) mprintfE(R"(Failed when check: mstr::split("I need {} and {}  ", "x") == std::vector<std::string>({ "I need {} and {}  " }))""\n");
            ret0 = (mstr::split("I need {} and {}  ", "{}", 1) == std::vector<std::string>({ "I need ", " and {}  " }));
            if (!ret0) mprintfE(R"(Failed when check: mstr::split("I need {} and {}  ", "{}", 1) == std::vector<std::string>({ "I need ", " and {}  " }))""\n");


            ret0 = (mstr::rsplit("I need {} and {}  ", "{}") == std::vector<std::string>({ "I need ", " and ", "  " }) );
            if (!ret0) mprintfE(R"(Failed when check: mstr::rsplit("I need {} and {}  ", "{}") == std::vector<std::string>({ "I need ", " and ", "  " }))""\n");

            ret0 = (mstr::rsplit("I need {} and {}  ", "x") == std::vector<std::string>({ "I need {} and {}  " }));
            if (!ret0) mprintfE(R"(Failed when check: mstr::rsplit("I need {} and {}  ", "x") == std::vector<std::string>({ "I need {} and {}  " }))""\n");
            ret0 = (mstr::rsplit("I need {} and {}  ", "{}", 1) == std::vector<std::string>({ "I need {} and ", "  " }));
            if (!ret0) mprintfE(R"(Failed when check: mstr::rsplit("I need {} and {}  ", "{}", 1) == std::vector<std::string>({ "I need {} and ", "  " }))""\n");
        }

        inline void trimTest()
        {
            bool ret0;
            ret0 = (mstr::trim(" \n\t 123 \v\r") == "123" && mstr::trim(" \n\t 123") == "123", mstr::trim("123 \v\r") == "123");
            if (!ret0) mprintfE(R"(Failed when check: mstr::trim(" \n\t 123 \v\r") == "123" && mstr::trim(" \n\t 123") == "123", mstr::trim("123 \v\r") == "123")""\n");

            ret0 = (mstr::ltrim(" \n\t 123 \v\r") == "123 \v\r" && mstr::ltrim(" \n\t 123") == "123", mstr::ltrim("123 \v\r") == "123 \v\r");
            if (!ret0) mprintfE(R"(Failed when check: mstr::ltrim(" \n\t 123 \v\r") == "123 \v\r" && mstr::ltrim(" \n\t 123") == "123", mstr::ltrim("123 \v\r") == "123 \v\r")""\n");

            ret0 = (mstr::rtrim(" \n\t 123 \v\r") == " \n\t 123" && mstr::rtrim(" \n\t 123") == " \n\t 123", mstr::rtrim("123 \v\r") == "123");
            if (!ret0) mprintfE(R"(Failed when check: mstr::rtrim(" \n\t 123 \v\r") == " \n\t 123" && mstr::rtrim(" \n\t 123") == " \n\t 123", mstr::rtrim("123 \v\r") == "123")""\n");
        }

        inline void padTest()
        {
            bool ret0;
            ret0 = (mstr::lpad("5", 5, '0') == "00005");
            if (!ret0) mprintfE(R"(Failed when check: mstr::lpad("5", 5, '0') == "00005")""\n");
            ret0 = (mstr::rpad("1.5", 3, '0') == "1.5000");
            if (!ret0) mprintfE(R"(Failed when check: mstr::rpad("1.5", 3, '0') == "1.5000")""\n");
        }

        inline void check()
        {
            printf("\n--------------------check mstr start--------------------\n");
            ordinalizeTest();
            toStrTest();
            splitTest();
            trimTest();
            printf("---------------------check mstr end---------------------\n\n");
        }
    }
#endif
}

#endif // !STR_HPP_MINEUTILS