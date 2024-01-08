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
#include<typeinfo>
#include<vector>

#include"base.hpp"
#include"type.hpp"
#include"math.hpp"


#ifdef MINE_DEBUG
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...) ((void*)0)
#endif 


#ifndef OPENCV_CORE_HPP
namespace cv
{
	class Mat;
	class MatExpr;
	template<typename _Tp> class Point_;
	template<typename _Tp> class Point3_;
	template<typename _Tp> class Size_;
	struct MatSize;
	template<typename _Tp> class Rect_;
	template<typename _Tp, int cn> class Vec;
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
		/*-------------------------------------声明--------------------------------------*/
		template<class T, class... Args>
		void print(const T& arg, const Args&... args);

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
		void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, mbase::CaseTag1& tag);

		template<size_t Idx, class... Ts>
		void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, mbase::CaseTag0& tag);

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

		void _print(const bool& arg);

		template<class T>
		void _print(const T& arg);

		template<class T, int N>
		void _print(const T(&arr)[N]);

//#ifdef CV_HPP_MINEUTILS
		void _print(const cv::Mat& img);

		void _print(const cv::MatExpr& img);

		template<class T>
		void _print(const cv::Point_<T>& pt);

		template<class T>
		void _print(const cv::Point3_<T>& pt);

		template<class T>
		void _print(const cv::Size_<T>& sz);

		void _print(const cv::MatSize& sz);

		template<class T>
		void _print(const cv::Rect_<T>& rect);
//#endif // CV_HPP_MINEUTILS

//#ifdef NCNN_HPP_MINEUTILS
		void _print(const ncnn::Mat& m);
//#endif // NCNN_HPP_MINEUTILS


		/*-------------------------------------定义--------------------------------------*/
		inline std::mutex& _getPrintMtx()
		{
			static std::mutex mtx;
			return mtx;
		}

		static std::mutex& print_mtx = _getPrintMtx();


		/*	实现类似Python的print打印功能
			-线程安全
			-支持int、float、char、std::string等基本类型数据的输出
			-支持int[]、float[]等基本多维数组类型数据的输出
			-支持std::vector、std::tuple等常用STL容器内容的输出
			-Warning: 输出cv::Mat时可能被cv::print接管   */
		template<class T, class... Args>
		inline void print(const T& arg, const Args&... args)
		{
			std::lock_guard<std::mutex> lk(print_mtx);
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
		void _print(const std::array<T, N>& arr)
		{
			std::cout << "[";
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
			std::cout << "]";
		}

		//为print函数添加对std::pair类型的支持
		template<class T1, class T2>
		inline void _print(const std::pair<T1, T2>& pa)
		{
			std::cout << "[";
			mio::_print(pa.first);
			std::cout << " ";
			mio::_print(pa.second);
			std::cout << "]";
		}

		//为print函数添加对std::tuple类型的支持
		template<size_t Idx, class... Ts>
		inline void _print(const std::tuple<Ts...>& tp)
		{
			constexpr int size_tp = std::tuple_size<std::tuple<Ts...>>::value;
			if (Idx == 0)
				std::cout << "[";

			constexpr int type_id = (Idx < size_tp);
			auto& case_tag = std::get<type_id>(mbase::BOOL_CASE_TAGS);
			mio::_printTuple<Idx, Ts...>(tp, size_tp, case_tag);
		}

		template<size_t Idx, class... Ts>
		inline void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, mbase::CaseTag1& tag)
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
		inline void _printTuple(const std::tuple<Ts...>& tp, const int& size_tp, mbase::CaseTag0& tag)
		{
			std::cout << "]";
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
			std::cout << "[";
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
			std::cout << "]";
		}

		//为print函数添加对std::string类型的支持
		inline void _print(const std::string& str)
		{
			std::cout << str;
		}

		//为print函数添加对std::bool类型的支持
		inline void _print(const bool& arg)
		{
			if (arg)
				std::cout << "true";
			else std::cout << "false";
		}

		//为print函数添加对基本类型的支持
		template<class T>
		inline void _print(const T& arg)
		{
			std::cout << arg;
		}

		//为print函数添加对数组类型的支持
		template<class T, int N>
		inline void _print(const T(&arr)[N])
		{
			if (mtype::isInTypes<T, char, wchar_t>())
				std::cout << arr;
			else
			{
				std::cout << "[";
				for (int i = 0; i < N - 1; i++)
				{
					mio::_print(arr[i]);
					std::cout << " ";
				}
				mio::_print(arr[N - 1]);
				std::cout << "]";
			}
		}
	}
}

#endif // !IO_HPP_MINEUTILS
