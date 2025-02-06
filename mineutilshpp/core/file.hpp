﻿//mineutils库的文件相关操作
#pragma once
#ifndef FILE_HPP_MINEUTILS
#define FILE_HPP_MINEUTILS

#include<fstream>
#include<iostream>
#include<list>
#include<map>
#include<string>
#include<vector>

#include"base.hpp"
#include"str.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mfile
    {
        //读写ini文件
        class IniFile
        {
        public:
            IniFile() = default;

            /*  打开ini文件，不存在的文件无法打开
                @param path: 文件路径
                @param key_value_sep: 分割key和value的字符，默认为'='
                @param note_signs: 注释标记符，默认为{ "#", ";" }
                @return 0代表正常，其他代表失败   */
            int open(std::string path, const char key_value_sep = '=', std::vector<std::string> note_signs = { "#", ";" });

            //关闭并保存文件，文件已关闭也能close；返回0代表正常，其他代表失败
            int close();

            //通过section和key获取value的值，section为空代表无section
            std::string getValue(const std::string& section, const std::string& key);

            //通过key获取value的值，只能获取无section的key-value条目
            std::string getValue(const std::string& key);

            //设置和添加key-value条目，，section为空代表无section，value可以是任意正确支持std::cout<<的类型
            template<class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type = 0>
            void setValue(const std::string& section, const std::string& key, const T& value);

            //设置和添加无section的key-value条目，value可以是任意正确支持std::cout<<的类型
            template<class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type = 0>
            void setValue(const std::string& key, const T& value);

            //禁止拷贝和移动
            IniFile(const IniFile& file) = delete;
            IniFile& operator=(const IniFile& file) = delete;
            ~IniFile();

        private:
            struct SectionInfo;
            struct KeyInfo;

            bool searchSection(const std::string& line, SectionInfo& section_info);
            bool searchKey(const std::string& line, KeyInfo& key_info);
            int saveContent();

            std::string file_path_;
            std::fstream file_;
            char rwstatus_ = 'r';
            char sep_ = '=';
            std::vector<std::string> note_signs_ = { "#", ";" };
            std::list<std::string> content_list_;
            std::map<std::string, SectionInfo> section_map_;
            std::map<std::string, std::map<std::string, KeyInfo>> key_map_;
        };

    }









    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mfile
    {
        struct IniFile::SectionInfo
        {
            std::list<std::string>::iterator line;
            std::list<std::string>::iterator last;
            size_t pos = std::string::npos;
            size_t len = 0;
        };

        struct IniFile::KeyInfo
        {
            std::list<std::string>::iterator line;
            size_t key_pos = std::string::npos;;
            size_t key_len = 0;
            size_t value_pos = std::string::npos;;
            size_t value_len = 0;
        };
 
        //打开ini文件
        inline int IniFile::open(std::string path, const char key_value_sep, std::vector<std::string> note_signs)
        {
            /*  ios::app：　　　 //以追加的方式打开文件
                ios::ate：　　　 //文件打开后定位到文件尾，ios:app就包含有此属性
                ios::binary：　 //以二进制方式打开文件，缺省的方式是文本方式。两种方式的区别见前文
                ios::in：　　　  //文件以输入方式打开（文件数据输入到内存）
                ios::out：　　　 //文件以输出方式打开（内存数据输出到文件）
                ios::nocreate： //不建立文件，所以文件不存在时打开失败
                ios::noreplace：//不覆盖文件，所以打开文件时如果文件存在失败
                ios::trunc：　  //如果文件存在，把文件长度设为0   */
            if (this->file_.is_open())
            {
                mprintfW("Duplicated open!\n");
                return 1;
            }

            this->file_.open(path, std::ios::binary | std::ios::in);
            if (!this->file_.is_open())
            {
                //printf("!Warning! %s: Failed to open %s! Please check if the file exists.\n", __FUNCTION__, path.c_str());
                mprintfE("Failed to open %s! Please check if the file exists.\n", path.c_str());
                return -1;
            }
            else
            {
                this->file_path_ = std::move(path);
                this->sep_ = key_value_sep;
                this->note_signs_ = std::move(note_signs);

                std::string line;
                std::vector<std::string> line_split;
                //content_list_.clear();
                int line_id = -1;
                std::string now_section = "";
                while (this->file_.good())   //按行读取内容，并去掉\r和\n符号
                {

                    line_id++;
                    line.clear();
                    std::getline(this->file_, line);
                    if (!line.empty())
                    {
                        line = mstr::rtrim(std::move(line));
                    }
                    this->content_list_.emplace_back(line);
                    if (line.empty())
                        continue;
                    SectionInfo section_info;
                    if (this->searchSection(line, section_info))
                    {
                        now_section = line.substr(section_info.pos, section_info.len);
                        if (this->section_map_.find(now_section) != this->section_map_.end())
                        {
                            mprintfW("Duplicate section:%s at line:%d!\n", now_section.c_str(), line_id+1);
                            continue;
                        }
                        section_info.line = --this->content_list_.end();
                        section_info.last = --this->content_list_.end();
                                        
                        this->section_map_[now_section] = section_info;
                        continue;
                    }

                    KeyInfo key_info;
                    if (this->searchKey(line, key_info))
                    {        
                        this->section_map_[now_section].last = --this->content_list_.end();

                        std::string key = line.substr(key_info.key_pos, key_info.key_len);
                        
                        if (this->key_map_.find(now_section) != this->key_map_.end() && this->key_map_[now_section].find(key) != this->key_map_[now_section].end())
                        {
                            mprintfW("Duplicate key:%s in section:%s at line:%d!\n", key.c_str(), now_section.c_str(), line_id+1);
                            continue;
                        }                       
                        key_info.line = --this->content_list_.end();
                        this->key_map_[now_section][key] = key_info;
                        continue;
                    }
                }
                while (true)
                {
                    if (this->content_list_.back().empty())
                        this->content_list_.pop_back();
                    else break;
                }
                return 0;
            }
        }

        //关闭并保存
        inline int IniFile::close()
        {
            if (this->rwstatus_ == 'w' && this->file_.is_open())
            {
                if (this->saveContent() != 0)
                    return -1;
            }
            this->content_list_.clear();
            this->section_map_.clear();
            this->key_map_.clear();
            this->file_.close();
            this->rwstatus_ = 'r';
            this->file_.clear();
            return 0;
        }

        inline std::string IniFile::getValue(const std::string& section, const std::string& key)
        {
            if (!this->file_.is_open())
            {
                mprintfE("File not opened!\n");
                return "";
            }
            if (this->key_map_.find(section) == this->key_map_.end())
            {
                mprintfW("The section:%s is not exist! Please check it.\n", section.c_str());
                return "";
            }
            if (this->key_map_[section].find(key) == this->key_map_[section].end())
            {
                mprintfW("The key:%s is not exist! Please check it.\n", key.c_str());
                return "";
            }
            KeyInfo& key_info = this->key_map_[section][key];
            //std::string& line = *key_info.line;
            return (*key_info.line).substr(key_info.value_pos, key_info.value_len);
        }

        inline std::string IniFile::getValue(const std::string& key)
        {
            return this->getValue("", key);
        }

        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type>
        inline void IniFile::setValue(const std::string& section, const std::string& key, const T& value)
        {
            if (!this->file_.is_open())
            {
                mprintfE("File not opened!\n");
                return;
            }
            this->rwstatus_ = 'w';
            if (this->key_map_.find(section) == this->key_map_.end())
            {
                if (section.empty())
                {
                    std::string value_str = mstr::toStr(value);
                    this->content_list_.emplace_back(key + this->sep_ + value_str);
                    this->section_map_[section].last = --this->content_list_.end();

                    this->key_map_[section][key].line = --this->content_list_.end();
                    this->key_map_[section][key].key_pos = 0;
                    this->key_map_[section][key].key_len = key.size();
                    this->key_map_[section][key].value_pos = key.size() + 1;
                    this->key_map_[section][key].value_len = value_str.size();
                }
                else
                {
                    this->content_list_.emplace_back("[" + section + "]");
                    this->section_map_[section].line = --this->content_list_.end();
                    this->section_map_[section].last = --this->content_list_.end();
                    this->section_map_[section].pos = 1;
                    this->section_map_[section].len = section.size();
                    this->key_map_[section] = { };
                    this->setValue(section, key, value);
                }
            }
            else
            {
                if (this->key_map_[section].find(key) == this->key_map_[section].end())
                {
                    std::string value_str = mstr::toStr(value);
                    auto bak = this->section_map_[section].last;
                    this->key_map_[section][key].line = this->content_list_.emplace(++this->section_map_[section].last, key + this->sep_ + value_str);
                    this->section_map_[section].last = ++bak;
                    this->key_map_[section][key].key_pos = 0;
                    this->key_map_[section][key].key_len = key.size();
                    this->key_map_[section][key].value_pos = key.size() + 1;
                    this->key_map_[section][key].value_len = value_str.size();
                }
                else
                {
                    std::string value_str = mstr::toStr(value);
                    *this->key_map_[section][key].line = key + this->sep_ + value_str;
                    this->key_map_[section][key].value_len = value_str.size();
                }
            }
        }

        template<class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type>
        inline void IniFile::setValue(const std::string& key, const T& value)
        {
            this->setValue("", key, value);
        }

        inline IniFile::~IniFile()
        {
            this->close();
        }

        inline bool IniFile::searchSection(const std::string& line, SectionInfo& section_info)
        {
            //找到注释的位置
            size_t note_pos = line.size();
            for (std::string& note_sign : this->note_signs_)
            {
                size_t tmp_pos = line.find(note_sign);
                if (tmp_pos < note_pos)
                    note_pos = tmp_pos;
            }

            size_t pos0 = line.find_first_not_of(' ');
            if (pos0 >= note_pos || line[pos0] != '[')
                return false;
            size_t pos1 = line.find_last_not_of(' ', note_pos - 1);
            if (pos1 >= note_pos || line[pos1] != ']')
                return false;
            if (pos1 <= pos0 + 1)
                return false;

            size_t sec_pos0 = line.find_first_not_of(" ", pos0 + 1);
            size_t sec_pos1 = line.find_last_not_of(" ", pos1 - 1);

            if (sec_pos1 >= sec_pos0)
            {
                std::string value = line.substr(sec_pos0, sec_pos1 - sec_pos0 + 1);
                section_info.pos = sec_pos0;
                section_info.len = sec_pos1 - sec_pos0 + 1;
                return true;
            }
            else return false;
        }

        inline bool IniFile::searchKey(const std::string& line, KeyInfo& key_info)
        {
            //找到注释的位置
            size_t note_pos = line.size();
            for (std::string& note_sign : this->note_signs_)
            {
                size_t tmp_pos = line.find(note_sign);
                if (tmp_pos < note_pos)
                    note_pos = tmp_pos;
            }

            size_t sep_pos = line.find(this->sep_);
            if (sep_pos >= note_pos)
                return false;

            std::string key = line.substr(0, sep_pos);
            std::vector<std::string> key_split = mstr::split(key);
            if (!key_split.empty())
            {
                key_info.key_pos = line.rfind(key_split.back(), sep_pos);
                key_info.key_len = key_split.back().size();
            }
            else return false;

            size_t v_pos0 = line.find_first_not_of(" ", sep_pos + 1);
            size_t v_pos1 = line.find_last_not_of(" ", note_pos - 1);

            if (v_pos1 >= v_pos0)
            {
                std::string value = line.substr(v_pos0, v_pos1 - v_pos0 + 1);
                key_info.value_pos = v_pos0;
                key_info.value_len = v_pos1 - v_pos0 + 1;
                return true;
            }
            else return false;
        }


        inline int IniFile::saveContent()
        {
            this->file_.close();
            this->file_.open(file_path_, std::ios::binary | std::ios::trunc | std::ios::out);
            if (!this->file_.is_open())
            {
                mprintfE("Open %s failed!\n", this->file_path_.c_str());
                return -1;
            }
            size_t i = 0;
            for (auto& content: this->content_list_)
            {
                if (i < this->content_list_.size() - 1)
                    this->file_ << content << "\n";
                else this->file_ << content;
                i++;
            }
            return 0;
        }
    }
}

#endif // !FILE_HPP_MINEUTILS