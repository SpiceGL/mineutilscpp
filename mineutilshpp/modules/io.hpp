//mineutils库的终端输入输出相关
#pragma once
#ifndef IO_HPP_MINEUTILS
#define IO_HPP_MINEUTILS

#include<array>
#include<exception>
#include<iostream>
#include<list>
#include<map>
#include<mutex>
#include<set>
#include<stdio.h>
#include<string>
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
    namespace mio
    {
        /*--------------------------------------------用户接口--------------------------------------------*/

        /*  实现类似Python的print打印功能
            -可以接受任意数量、任意类型的参数
            -可以正常打印任意支持std::cout<<的内置数据类型
            -可以正常打印任意在同一个namespace内重载了std::cout<<的自定义类型
            -拓展了std::vector、std::tuple等常用STL容器及OpenCV、NCNN的部分数据类型的打印
            -字符和字符串类型只能正确打印char和st::string类型，wchar_t、char16_t等宽字符型无法正确打印
            -不支持的类型将会打印<类型名: 地址>
            -在不混用print函数和std::cout时，线程安全  */
        template<class T, class... Args>
        void print(const T& arg, const Args&... args);








        /*--------------------------------------------内部实现--------------------------------------------*/

        template<class T, class... Args>
        void _recurPrint(const T& arg, const Args&... args);

        void _recurPrint();

        template<class T, size_t N>
        void _print(const std::array<T, N>& arr);

        template<class T1, class T2>
        void _print(const std::pair<T1, T2>& pa);

        template<size_t Idx = 0, class... Ts>
        void _print(const std::tuple<Ts...>& tp);

        template<size_t Idx, class... Ts>
        void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, std::true_type bool_tag);

        template<size_t Idx, class... Ts>
        void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, std::false_type bool_tag);

        template<class T1, class T2, class... Ts>
        void _print(const std::map<T1, T2, Ts...>& m);

        template<class T, class... Ts>
        void _print(const std::list<T, Ts...>& l);

        template<class T, class... Ts>
        void _print(const std::set<T, Ts...>& st);

        template<class T, class... Ts>
        void _print(const std::vector<T, Ts...>& vec);

        template<template<class C, class... Cs> class CTer, class T, class... Ts>
        void _print_stdcter(const CTer<T, Ts...>& cter);

        void _print(const std::string& str);
        
        template<class T, typename std::enable_if<std::is_same<decltype(std::cout << std::declval<T>()), std::ostream&>::value, int>::type = 0>
        void _print(const T& arg);

        template<class T, typename std::enable_if<!std::is_same<decltype(std::cout << std::declval<T>()), std::ostream&>::value, int>::type = 0>
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
            mio::_printTuple<Idx, Ts...>(tp, size_tp, std::integral_constant<bool, Idx < size_tp>());
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

        //为print函数添加对std::map类型的支持
        template<class T1, class T2, class... Ts>
        inline void _print(const std::map<T1, T2, Ts...>& m)
        {
            std::cout << "{";
            int size = m.size();
            if (size > 0)
            {
                auto bg = m.begin();
                for (int i = 0; i < size - 1; ++i)
                {
                    mio::_print((*bg).first);
                    std::cout << ": ";
                    mio::_print((*bg).second);
                    std::cout << ", ";
                    ++bg;
                }
                mio::_print((*bg).first);
                std::cout << ": ";
                mio::_print((*bg).second);
            }
            std::cout << "}";
        }

        //为print函数添加对std::list类型的支持
        template<class T, class... Ts>
        inline void _print(const std::list<T, Ts...>& l)
        {
            mio::_print_stdcter(l);
        }

        //为print函数添加对std::set类型的支持
        template<class T, class... Ts>
        inline void _print(const std::set<T, Ts...>& st)
        {
            mio::_print_stdcter(st);
        }

        //为print函数添加对std::vector类型的支持
        template<class T, class... Ts>
        inline void _print(const std::vector<T, Ts...>& vec)
        {
            mio::_print_stdcter(vec);
        }

        template<template<class C, class... Cs> class CTer, class T, class... Ts>
        inline void _print_stdcter(const CTer<T, Ts...>& cter)   //虽然用了双层模板，但单层也可以达成目的
        {
            std::cout << "{";
            int size = cter.size();
            if (size > 0)
            {
                auto bg = cter.begin();
                for (int i = 0; i < size - 1; ++i)
                {
                    mio::_print(*bg);
                    std::cout << " ";
                    ++bg;
                }
                mio::_print(*bg);
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
        template<class T, typename std::enable_if<std::is_same<decltype(std::cout << std::declval<T>()), std::ostream&>::value, int>::type>
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
        template<class T, typename std::enable_if<!std::is_same<decltype(std::cout << std::declval<T>()), std::ostream&>::value, int>::type>
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
