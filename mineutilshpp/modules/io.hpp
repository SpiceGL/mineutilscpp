//mineutils库的终端输入输出相关
#pragma once
#ifndef IO_HPP_MINEUTILS
#define IO_HPP_MINEUTILS

#include<array>
#include<deque>
#include<exception>
#include<forward_list>
#include<iostream>
#include<list>
#include<map>
#include<mutex>
#include<queue>
#include<set>
#include<stack>
#include<stdio.h>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>

#ifdef __GNUC__ 
#include<cxxabi.h>
#else
#include<typeinfo>
#endif

#include"base.hpp"
#include"type.hpp"
#include"math.hpp"
#include"log.hpp"


#ifndef OPENCV_CORE_HPP
namespace cv
{
    class Mat;
    class MatExpr;
}
#endif

#ifndef NCNN_NET_H
namespace ncnn
{
    class Mat;
}
#endif // !NCNN_NET_H



namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mio
    {
        /*  实现类似Python的print打印功能，基于std::cout
            -可以接受任意数量、任意类型的参数
            -可以正常打印任意支持std::cout<<的内置数据类型
            -可以正常打印正确重载了operator<<的自定义类型
            -拓展了STL容器的打印
            -拓展了OpenCV、NCNN的部分数据类型的打印(需要导入cv.hpp及ncnn.hpp模块)
            -注意wchar_t、char16_t等宽字符型无法正确打印(宽字符型本身不支持std::cout)
            -未支持的类型将会打印<类型名: 地址>
            -在不混用print函数和std::cout时，线程安全  */
        template<class T, class... Args>
        void print(const T& arg, const Args&... args);
    }







    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mio
    {
        template<class T, class... Args>
        void _recurPrint(const T& arg, const Args&... args);

        void _recurPrint();

        template<class T, size_t N>
        void _print(const std::array<T, N>& arr);

        template<class T, class...Ts>
        void _print(const std::stack<T, Ts...>& st);

        void _print(const std::string& str);

        template<class KeyT, class VT, class... Ts>
        void _print(const std::map<KeyT, VT, Ts...>& m);

        template<class KeyT, class VT, class... Ts>
        void _print(const std::multimap<KeyT, VT, Ts...>& m);

        template<class KeyT, class VT, class... Ts>
        void _print(const std::unordered_map<KeyT, VT, Ts...>& m);

        template<class KeyT, class VT, class... Ts>
        void _print(const std::unordered_multimap<KeyT, VT, Ts...>& m);

        template<class T, class... Ts>
        void _print(const std::forward_list<T, Ts...>& l);

        template<class T, class...Ts>
        void _print(const std::priority_queue<T, Ts...>& qe);

        template<class T1, class T2>
        void _print(const std::pair<T1, T2>& pa);

        template<size_t Idx = 0, class... Ts>
        void _print(const std::tuple<Ts...>& tp);

        template<size_t Idx, class... Ts>
        void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, std::true_type bool_tag);

        template<size_t Idx, class... Ts>
        void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, std::false_type bool_tag);

        template<template<class KeyT, class VT, class... Ts> class CTer, class KeyT, class VT, class... Ts>
        void _printMap(const CTer<KeyT, VT, Ts...>& m);

        template<template<class U, class... Us> class CTer, class T, class... Ts, typename std::enable_if<mtype::StdBeginEndChecker<const CTer<T, Ts...>>::value, int>::type = 0>
        void _print(const CTer<T, Ts...>& cter);

        template<class T, typename std::enable_if<mtype::StdCoutChecker<const T>::value, int>::type = 0>
        void _print(const T& arg);

        template<class T, typename std::enable_if<!mtype::StdCoutChecker<const T>::value, int>::type = 0>
        void _print(const T& arg);

        template<class T, int N>
        void _print(const T(&arr)[N]);

        void _print(const cv::Mat& img);

        void _print(const cv::MatExpr& img);

        void _print(const ncnn::Mat& m);



        /*-------------------------------------定义--------------------------------------*/
        inline std::mutex& _getPrintMtx()
        {
            static std::mutex mtx;
            return mtx;
        }

        template<class T, class... Args>
        inline void print(const T& arg, const Args&... args)
        {
            std::lock_guard<std::mutex> lk(_getPrintMtx());
            mio::_recurPrint(arg, args...);
        }

        template<class T, class... Args>
        inline void _recurPrint(const T& arg, const Args&... args)
        {
            mio::_print(arg);
            std::cout << " ";
            mio::_recurPrint(args...);
            //int _a[] = { (_print(args), std::cout << " ", 0)... };   //另一种写法
        }

        //_recurPrint递归结束位置
        inline void _recurPrint()
        {
            std::cout << std::endl;
        }

        //为print函数添加对std::array类型的支持
        template<class T, size_t N>
        inline void _print(const std::array<T, N>& arr)
        {
            std::cout << "{";
            if (N > 0)
            {
                auto bg = arr.begin();
                for (int i = 0; i < N - 1; ++i)
                {
                    mio::_print(*bg);
                    std::cout << " ";
                    ++bg;
                }
                mio::_print(*bg);
            }
            std::cout << "}";
        }

        template<class T, class...Ts>
        void _print(const std::stack<T, Ts...>& st)
        {
            auto tmp_st = st;
            std::cout << "{";
            int size = tmp_st.size();
            if (size > 0)
            {
                for (int i = 0; i < size - 1; ++i)
                {
                    mio::_print(tmp_st.top());
                    tmp_st.pop();
                    std::cout << " ";
                }
                mio::_print(tmp_st.top());
            }
            std::cout << "}";
        }

        template<class T, class...Ts>
        void _print(const std::priority_queue<T, Ts...>& qe)
        {
            auto tmp_qe = qe;
            std::cout << "{";
            int size = tmp_qe.size();
            if (size > 0)
            {
                for (int i = 0; i < size - 1; ++i)
                {
                    mio::_print(tmp_qe.top());
                    tmp_qe.pop();
                    std::cout << " ";
                }
                mio::_print(tmp_qe.top());
            }
            std::cout << "}";
        }


        //为print函数添加对std::pair类型的支持
        template<class T1, class T2>
        inline void _print(const std::pair<T1, T2>& pa)
        {
            std::cout << "{";
            mio::_print(pa.first);
            std::cout << " ";
            mio::_print(pa.second);
            std::cout << "}";
        }

        //为print函数添加对std::tuple类型的支持
        template<size_t Idx, class... Ts>
        inline void _print(const std::tuple<Ts...>& tp)
        {
            constexpr int size_tp = std::tuple_size<std::tuple<Ts...>>::value;
            if (Idx == 0)
                std::cout << "{";
            mio::_printTuple<Idx, Ts...>(tp, size_tp, std::integral_constant<bool, (Idx < size_tp) >());
        }

        template<size_t Idx, class... Ts>
        inline void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, std::true_type bool_tag)
        {
            if (Idx < size_tp - 1)
            {
                mio::_print(std::get<Idx>(tp));
                std::cout << " ";
                mio::_print<Idx + 1, Ts...>(tp);
            }
            else if (Idx == size_tp - 1)
            {
                mio::_print(std::get<Idx>(tp));
                mio::_print<Idx + 1, Ts...>(tp);
            }
            else throw "Error Idx!";
        }

        template<size_t Idx, class... Ts>
        inline void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, std::false_type bool_tag)
        {
            std::cout << "}";
        }

        template<template<class KeyT, class VT, class... Ts> class CTer, class KeyT, class VT, class... Ts>
        inline void _printMap(const CTer<KeyT, VT, Ts...>& m)
        {
            std::cout << "{";
            int size = m.size();
            if (size > 0)
            {
                auto it = m.begin();
                for (int i = 0; i < size - 1; ++i)
                {
                    mio::_print((*it).first);
                    std::cout << ":";
                    mio::_print((*it).second);
                    std::cout << ", ";
                    ++it;
                }
                mio::_print((*it).first);
                std::cout << ":";
                mio::_print((*it).second);
            }
            std::cout << "}";
        }

        //为print函数添加对std::map类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _print(const std::map<KeyT, VT, Ts...>& m)
        {
            mio::_printMap(m);
        }

        //为print函数添加对std::multimap类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _print(const std::multimap<KeyT, VT, Ts...>& m)
        {
            mio::_printMap(m);
        }


        //为print函数添加对std::unordered_map类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _print(const std::unordered_map<KeyT, VT, Ts...>& m)
        {
            mio::_printMap(m);
        }

        //为print函数添加对std::unordered_multimap类型的支持
        template<class KeyT, class VT, class... Ts>
        inline void _print(const std::unordered_multimap<KeyT, VT, Ts...>& m)
        {
            mio::_printMap(m);
        }

        template<class T, class... Ts>
        void _print(const std::forward_list<T, Ts...>& l)
        {
            std::cout << "{";
            for (auto it = l.begin(); it != l.end(); )
            {
                mio::_print(*it);
                it++;
                if (it != l.end())
                    std::cout << " ";
            }
            std::cout << "}";
        }

        //添加对STL大部分标准容器的支持
        template<template<class U, class... Us> class CTer, class T, class... Ts, typename std::enable_if<mtype::StdBeginEndChecker<const CTer<T, Ts...>>::value, int>::type>
        inline void _print(const CTer<T, Ts...>& cter)   //虽然用了双层模板，但单层也可以达成目的
        {
            std::cout << "{";
            for (auto it = cter.begin(); it != cter.end(); )
            {
                mio::_print(*it);
                it++;
                if (it != cter.end())
                    std::cout << " ";
            }
            std::cout << "}";
        }

        //为print函数添加对std::string类型的支持
        inline void _print(const std::string& str)
        {
            //std::cout << "\"" <<  str << "\"";
            std::cout << str;
        }

        //为print函数拓展其他支持std::cout<<的类型
        template<class T, typename std::enable_if<mtype::StdCoutChecker<const T>::value, int>::type>
        inline void _print(const T& arg)
        {
            //std::cout << "test: "<< mtype::isInTypes<T, const char*>() << "\n";
            //if (mtype::isInTypes<T, char*, const char*>())
            //    std::cout << "\"" << arg << "\"";
            //else if (mtype::isInTypes<T, char>())
            //    std::cout << "\'" << arg << "\'";
            //else std::cout << arg;
            std::cout << arg;
        }

        //为print函数拓展其他不支持std::cout<<的类型
        template<class T, typename std::enable_if<!mtype::StdCoutChecker<const T>::value, int>::type>
        inline void _print(const T& arg)
        {
#ifdef __GNUC__
            std::cout << "<" << abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr) << ": " << std::hex << &arg << std::dec << ">";
#else
            std::cout << "<" << typeid(T).name() << ": 0x" << std::hex << &arg << std::dec << ">";
#endif // __GNUC__
        }

        //为print函数添加对数组类型的支持
        template<class T, int N>
        inline void _print(const T(&arr)[N])
        {
            if (mtype::isInTypes<T, char>())
                //std::cout << "\"" << arr << "\"";
                std::cout << arr;
            else
            {
                std::cout << "{";
                for (int i = 0; i < N - 1; i++)
                {
                    mio::_print(arr[i]);
                    std::cout << " ";
                }
                mio::_print(arr[N - 1]);
                std::cout << "}";
            }
        }
    }
}

#endif // !IO_HPP_MINEUTILS
