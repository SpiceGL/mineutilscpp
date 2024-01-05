//mineutils库的std::string字符串相关工具
#pragma once
#ifndef STR_HPP_MINEUTILS
#define STR_HPP_MINEUTILS

#include<iomanip>
#include<iostream>
#include<sstream>
#include<string>
#include<vector>


namespace mineutils
{
	namespace mstr
	{
		inline bool& _createColorStrOn()
		{
			static bool _ColorStr_On = false;
			return _ColorStr_On;
		}

		static bool& _ColorStr_On = _createColorStrOn();   //ColorStr功能全局开关，默认关闭


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


		//设置是否开启彩色字体的效果
		//部分终端不一定支持彩色字体显示，因此默认关闭该功能
		inline void setColorStrOn(bool ColorStr_on)
		{
			_ColorStr_On = ColorStr_on;
		}

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

		inline std::string color(const std::string& str, const mstr::Color str_color)
		{
			if (_ColorStr_On)
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

		//将输入直接转换为字符串
		template<class T>
		inline std::string toStr(const T& arg)
		{
			std::ostringstream str_buf;   //给toStr函数使用的字符串流
			str_buf.clear();
			str_buf.str("");
			str_buf << arg;
			return str_buf.str();
		}

		/*	类似Python字符串的zfill函数，将整型的数字转换为字符串并在前面添加字符
			@param n：输入的整型数字
			@param min_len：转换结果的长度
			@param padding：用于填充的字符
			@return 转换结果   */
		inline std::string zfillInt(long long n, int min_len = 0, char padding = '0')
		{
			std::string s = toStr(n);
			if (s.length() < min_len)
			{
				s = std::string(min_len - s.length(), padding) + s;
			}
			return s;
		}

		/*	类似Python字符串的zfill函数，将浮点型的数字转换为字符串并在前后添加字符
			@param f：输入的浮点数字
			@param min_len_int：转换结果整数部分的长度
			@param flt_precision：转换结果小数部分的精度
			@param int_padding：用于填充整数部分的字符
			@param flt_padding：用于填充小数部分的字符
			@return 转换结果   */
		inline std::string zfillFlt(long double f, int min_len_int = 0, int flt_precision = 4,
			char int_padding = ' ', char flt_padding = '0')
		{
			//static_assert(std::is_floating_point<FT>::value, "Class FT must be floating_point!");
			std::ostringstream buffer;
			buffer << std::setprecision(flt_precision) << f;
			std::string s = buffer.str();

			/*找到输入小数的整数部分和小数部分，分别处理并合并*/
			std::string int_part, flt_part;
			size_t point_pos = s.find(".");
			if (point_pos != -1)
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

		/*--------------------------------------------------------------------------*/
		//fstr函数的相关
		inline std::string _fstr(std::string& s, size_t pos_offset)
		{
			size_t pos = s.find("{}", pos_offset);
			if (pos != -1)
			{
				std::cout << mstr::color(std::string("!Warning! ") + __FUNCTION__ + ": ", mstr::Color::yellow) << "function need more params!\n";
			}
			return s;
		}

		//fstr函数的相关
		template<class T, class... Ts>
		inline std::string _fstr(std::string& s, size_t pos_offset, const T& arg, const Ts&... args)
		{
			size_t pos = s.find("{}", pos_offset);
			if (pos != -1)
			{
				std::string arg_s = toStr(arg);
				s.replace(pos, 2, arg_s);
				pos_offset = pos + arg_s.size();
			}
			else
			{
				std::cout << mstr::color(std::string("!Warning! ") + __FUNCTION__ + ": ", mstr::Color::yellow) << "function got too much params!\n";
				return s;
			}
			return mstr::_fstr(s, pos_offset, args...);
		}

		//实现类似于python的f-string功能，将字符串中的"{}"替换为后续的参数
		template<class... Ts>
		inline std::string fstr(std::string s, const Ts& ...args)
		{
			return mstr::_fstr(s, 0, args...);
		}

		/*	实现对字符串的分割功能，以vector形式返回
			@param s：待分割的字符串
			@param sep：分割符
			@return 分割结果   */
		inline std::vector<std::string> split(std::string s, const std::string& sep)
		{
			std::vector<std::string> strs;
			size_t sep_pos;
			std::string s_in;   //s_in存放已处理的字段，s存放待处理的字段

			while ((sep_pos = s.find(sep)) != -1)
			{
				s_in = s.substr(0, sep_pos);
				s = s.substr(sep_pos + sep.length());
				strs.push_back(s_in);
				s_in.clear();
			}
			strs.push_back(s);
			return strs;
		}

		//按字符串中的空格分割字符串，若输入空字符串或全空格字符串则返回空vector
		inline std::vector<std::string> split(std::string s)
		{
			std::vector<std::string> strs;
			std::stringstream ss;
			ss << s;
			std::string s_in;
			ss >> s_in;
			while (s_in.size() > 0)
			{
				strs.push_back(s_in);
				s_in.clear();
				ss >> s_in;
			}
			return strs;
		}
	}
}

#endif // !STR_HPP_MINEUTILS