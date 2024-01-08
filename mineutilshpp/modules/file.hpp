//mineutils库的文件相关操作
#pragma once
#ifndef FILE_HPP_MINEUTILS
#define FILE_HPP_MINEUTILS

#include<fstream>
#include<iostream>
#include<string>
#include<vector>

#include"str.hpp"
#include"log.hpp"


namespace mineutils
{
	namespace mfile
	{
		/*	ios::app：　　　 //以追加的方式打开文件
			ios::ate：　　　 //文件打开后定位到文件尾，ios:app就包含有此属性
			ios::binary：　 //以二进制方式打开文件，缺省的方式是文本方式。两种方式的区别见前文
			ios::in：　　　  //文件以输入方式打开（文件数据输入到内存）
			ios::out：　　　 //文件以输出方式打开（内存数据输出到文件）
			ios::nocreate： //不建立文件，所以文件不存在时打开失败
			ios::noreplace：//不覆盖文件，所以打开文件时如果文件存在失败
			ios::trunc：　  //如果文件存在，把文件长度设为0   */

		//读写ini文件。
		//ini文件结构：
		//[section]   #注释1
		//key=value   ;注释2
		//由#或;表示注释
		//可以没有section，section不可重复不可分段，一个section下的key不可重复
		class IniFile
		{
		public:
			IniFile() { }
			IniFile(const IniFile& file) = delete;
			IniFile& operator=(const IniFile& file) = delete;

			~IniFile()
			{
				this->close();
			}

			/*	@brief 自定义作为注释符的符号，默认{"#", ";"}
			*	@param note_signs: 作为注释符的符号，以vector输入  */
			void setNoteSigns(std::vector<std::string> note_signs = { "#", ";" })
			{
				this->note_signs_ = note_signs;
			}

			//打开ini文件
			int open(const std::string& path)
			{
				if (file_.is_open())
				{
					printf("IniFile: File was opened. Open the file again.");
					std::cout << mlog::messageN(mstr::Color::purple, "{}: File was opened. Open the file again.\n", __FUNCTION__);
					file_.close();
				}

				file_path_ = path;
				file_.open(file_path_, std::ios::binary | std::ios::in);
				if (!file_.is_open())
				{
					//printf("!Warning! %s: Failed to open %s! Please check if the file exists.\n", __FUNCTION__, path.c_str());
					std::cout << mlog::messageN(mstr::Color::cyan, "{}: Failed to open {}! Please check if the file exists.\n", __FUNCTION__, file_path_);
					return -1;
				}
				else
				{
					std::string line;
					content_.clear();
					while (file_.good())   //按行读取内容，并去掉\r和\n符号
					{
						line.clear();
						std::getline(file_, line);
						if (line.size() > 0)
						{
							line = mstr::split(line, "\n")[0];
							line = mstr::split(line, "\r")[0];
						}
						content_.push_back(line);
					}
					return 0;
				}
			}

			//关闭并保存
			void close()
			{
				if (rwstatus_ == 'w')
					saveContent();
				content_.clear();
				file_.close();
				rwstatus_ = 'r';
				file_.clear();
			}

			//根据section和key获取值
			std::string getValue(const std::string& section, const std::string& key)
			{
				std::string value;
				int section_start, section_end;
				if (section.size() > 0)
				{
					section_start = findSection(section);
					section_end = findSectionEnd(section_start);
				}
				else
				{
					section_start = 0;
					section_end = content_.size();
				}
				if (section_start >= 0 && section_end >= 0)
				{
					int key_idx = findKey(section_start, section_end, key);
					if (key_idx >= 0)
					{
						value = mstr::split(content_[key_idx], "=")[1];
						value = mstr::split(value)[0];
						for (const std::string& sign : note_signs_)
							value = mstr::split(value, sign)[0];
					}
					else
					{
						std::cout << mlog::messageW("{}: key={} not exists!\n", __FUNCTION__, key);
					}
				}
				else
				{
					std::cout << mlog::messageW("{}: section={} not exists!\n", __FUNCTION__, section);
				}
				file_.seekg(0);
				return value;
			}

			//根据key获得值，没有section时使用
			std::string getValue(const std::string& key)
			{
				return getValue("", key);
			}

			/*	@brief 根据section和key设置值
			*	@param value: int、float等数字类型或char、string等字符类型
			*/
			template<class T>
			void setValue(const std::string& section, const std::string& key, const T& value)
			{
				std::string s_value = mstr::toStr(value);
				rwstatus_ = 'w';
				int section_start, section_end;
				if (section.size() > 0)
				{
					section_start = findSection(section);
					section_end = findSectionEnd(section_start);
				}
				else
				{
					section_start = 0;
					section_end = content_.size();
				}
				if (section_start >= 0 && section_end >= section_start + 1)
				{
					int key_idx = findKey(section_start, section_end, key);
					if (key_idx >= 0)
					{
						content_[key_idx] = key + "=" + s_value;// +"   " + note;
					}
					else
					{
						content_.insert(content_.begin() + section_end, key + "=" + s_value);
					}
				}
				else
				{
					content_.push_back("[" + section + "]");
					content_.push_back(key + "=" + s_value);
				}
			}

			/*	@brief 根据key设置值
			*	@param value: int、float等数字类型或char、string等字符类型，使用sstream转换为字符串   */
			template<class T>
			void setValue(const std::string& key, const T& value)
			{
				setValue("", key, value);
			}

		private:
			std::string file_path_;
			std::fstream file_;
			char rwstatus_ = 'r';
			std::vector<std::string> content_;
			std::vector<std::string> note_signs_ = { "#", ";" };

			//查找section所在位置，未找到则返回-1
			int findSection(const std::string& section) const
			{
				for (int i = 0; i < content_.size(); ++i)
				{
					if (content_[i] == "[" + section + "]")
						return i;
				}
				return -1;
			}

			//查找key所在位置，未找到则返回-1
			int findKey(int section_start, int section_end, const std::string& key) const
			{
				if (section_start >= 0 && section_end >= 0)
				{
					for (int i = section_start; i < section_end; ++i)
					{
						if (content_[i].size() > 0)
						{
							if (content_[i].find("=") != -1 && mstr::split(mstr::split(content_[i], "=")[0])[0] == key)
								return i;
						}
					}
				}
				else return -1;
			}

			//查找Section的最后一行的下一行所在位置
			int findSectionEnd(int section_start) const
			{
				if (section_start < 0)
					return section_start;
				for (int i = section_start + 1; i < content_.size(); ++i)
				{
					if (content_[i].size() > 0)
					{
						if (content_[i][0] == '[')
							return i;
					}
				}
				return content_.size();
			}

			//查找Section的最后一行的下一行所在位置
			int findSectionEnd(const std::string& section) const
			{
				int section_start = findSection(section);
				if (section_start >= 0)
					return findSectionEnd(section_start);
				else return -1;
			}

			void saveContent()
			{
				file_.close();
				file_.open(file_path_, std::ios::binary | std::ios::trunc | std::ios::out);
				for (int i = 0; i < content_.size(); ++i)
				{
					if (i == content_.size() - 1)
						file_ << content_[i];
					else file_ << content_[i] << std::endl;
				}
			}
		};
	}
}

#endif // !FILE_HPP_MINEUTILS