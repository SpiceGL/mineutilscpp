// mineutils库的文件相关操作
#pragma once
#ifndef FILE_HPP_MINEUTILS
#define FILE_HPP_MINEUTILS

#include <cctype>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "base.hpp"
#include "str.hpp"


namespace mineutils
{
/*--------------------------------------------用户接口--------------------------------------------*/

namespace mfile
{
    /*  读写ini文件
        - 文件以UTF-8编码和UTF-8 BOM格式读取
        - 支持带section和不带section的ini文件
        - section和key必须以字母为起始，其他字符可为字母、数字、下划线和短横线'-'，否则该行会被忽略
        - section和key前后可以有空白字符，它们会被自动忽略
        - section由中括号[]包裹，如果有多个同名section，会被当成一个section处理
        - 如果同一个section中有多个同名key，最后一个key会覆盖前面的key
        - key-value行的第一个分隔符到注释标记之间的内容，除去首尾的空白字符都属于value
        - 空白字符指" \t\n\r\f\v"
        - 参考格式如下，不包含section和key-value的行会被忽略：
            [section1]
            key1 = value1
            key2 = value2
            [section2]
            key3 = value3
            key4 = value4
            ...                     */
    class IniFile final
    {
    public:
        IniFile() = default;

        /*  打开ini文件，不存在的文件无法打开
            @param path: 文件路径
            @param key_value_sep: 分割key和value的字符，默认为'='
            @param note_sign: 注释标记符，默认为'#'
            @return 0代表正常，其他代表失败   */
        int open(std::string path, char key_value_sep = '=', char note_sign = '#');

        // 关闭并保存文件，文件已关闭也能close；返回0代表正常，其他代表失败
        int close();

        // 通过section和key获取value的值，section为空代表无section
        std::string getValue(const std::string& section, const std::string& key);

        // 通过key获取value的值，只能获取无section的key-value条目
        std::string getValue(const std::string& key);

        // 设置和添加key-value条目，，section为空代表无section，value可以是任意正确支持std::cout<<的类型
        template <class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type = 0>
        void setValue(const std::string& section, const std::string& key, const T& value);

        // 设置和添加无section的key-value条目，value可以是任意正确支持std::cout<<的类型
        template <class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type = 0>
        void setValue(const std::string& key, const T& value);

        // 打印读取的文件内容
        void printContents();

        // 禁止拷贝和移动
        IniFile(const IniFile& file) = delete;
        IniFile& operator=(const IniFile& file) = delete;
        ~IniFile();

    private:
        struct SectionInfo;
        struct KeyInfo;

        bool searchSection(const std::string& line, SectionInfo& section_info, size_t offset);
        bool searchKey(const std::string& line, KeyInfo& key_info, size_t offset);
        bool checkSectionKeyFmt(const std::string& section_or_key);
        int saveContents();

        std::string file_path_;
        std::fstream file_;
        char rwstatus_ = 'r';
        char sep_ = '=';
        char note_sign_ = '#';
        std::list<std::string> content_list_;
        std::map<std::string, SectionInfo> section_map_;
        std::map<std::string, std::map<std::string, KeyInfo>> key_map_;
    };

}  // namespace mfile





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
        size_t key_pos = std::string::npos;
        ;
        size_t key_len = 0;
        size_t value_pos = std::string::npos;
        ;
        size_t value_len = 0;
    };

    // 打开ini文件
    inline int IniFile::open(std::string path, char key_value_sep, char note_sign)
    {
        /*  ios::app：　　　 //以追加的方式打开文件
            ios::ate：　　　 //文件打开后定位到文件尾，ios:app就包含有此属性
            ios::binary：　 //以二进制方式打开文件，缺省的方式是文本方式。两种方式的区别见前文
            ios::in：　　　  //文件以输入方式打开（文件数据输入到内存）
            ios::out：　　　 //文件以输出方式打开（内存数据输出到文件）
            ios::nocreate： //不建立文件，所以文件不存在时打开失败
            ios::noreplace：//不覆盖文件，所以打开文件时如果文件存在失败
            ios::trunc：　  //如果文件存在，把文件长度设为0   */
        if (key_value_sep == '[' || key_value_sep == ']')
        {
            mprintfE("Invalid key-value separator: %c\n", key_value_sep);
            return -1;
        }
        if (note_sign == '[' || note_sign == ']')
        {
            mprintfE("Invalid note sign: %c\n", note_sign);
            return -1;
        }
        if (key_value_sep == note_sign)
        {
            mprintfE("Key-value separator and note sign cannot be the same character: %c\n", key_value_sep);
            return -1;
        }

        if (file_.is_open())
        {
            mprintfW("Duplicated open!\n");
            return 1;
        }

        file_.open(path, std::ios::binary | std::ios::in);
        if (!file_.is_open())
        {
            // printf("!Warning! %s: Failed to open %s! Please check if the file exists.\n", __FUNCTION__, path.c_str());
            mprintfE("Failed to open %s! Please check if the file exists.\n", path.c_str());
            return -1;
        }
        else
        {
            file_path_ = std::move(path);
            sep_ = key_value_sep;
            note_sign_ = note_sign;

            std::string line;
            std::vector<std::string> line_split;
            // content_list_.clear();
            int line_id = -1;
            std::string now_section;
            while (file_.good())  // 按行读取内容，并去掉\r和\n符号
            {
                size_t offset = 0;
                line_id++;
                line.clear();
                std::getline(file_, line);
                if (line_id == 0)
                {
                    if (line.size() > 3 && line.substr(0, 3) == "\xEF\xBB\xBF")
                    {
                        mprintfI("Process the file:%s with UTF-8 BOM encoding\n", file_path_.c_str());
                        offset = 3;
                    }
                    else
                        mprintfI("Process the file:%s with UTF-8 encoding\n", file_path_.c_str());
                }
                if (!line.empty())
                {
                    line = mstr::rtrim(std::move(line));
                }
                content_list_.emplace_back(line);
                if (line.empty())
                    continue;
                SectionInfo section_info;
                if (this->searchSection(line, section_info, offset))
                {
                    now_section = line.substr(section_info.pos, section_info.len);
                    if (section_map_.find(now_section) != section_map_.end())
                    {
                        mprintfW("Duplicate section:%s at line:%d!\n", now_section.c_str(), line_id + 1);
                    }
                    section_info.line = --content_list_.end();
                    section_info.last = --content_list_.end();

                    section_map_[now_section] = section_info;
                    key_map_[now_section] = {};
                    continue;
                }

                KeyInfo key_info;
                if (this->searchKey(line, key_info, offset))
                {
                    section_map_[now_section].last = --content_list_.end();

                    std::string key = line.substr(key_info.key_pos, key_info.key_len);

                    if (key_map_.find(now_section) != key_map_.end() && key_map_[now_section].find(key) != key_map_[now_section].end())
                    {
                        mprintfW("Duplicate key:%s in section:%s at line:%d!\n", key.c_str(), now_section.c_str(), line_id + 1);
                    }
                    key_info.line = --content_list_.end();
                    key_map_[now_section][key] = key_info;
                    continue;
                }
            }
            while (true)
            {
                if (content_list_.back().empty())
                    content_list_.pop_back();
                else
                    break;
            }
            return 0;
        }
    }

    // 关闭并保存
    inline int IniFile::close()
    {
        if (rwstatus_ == 'w' && file_.is_open())
        {
            if (this->saveContents() != 0)
                return -1;
        }
        content_list_.clear();
        section_map_.clear();
        key_map_.clear();
        file_.close();
        rwstatus_ = 'r';
        file_.clear();
        return 0;
    }

    inline std::string IniFile::getValue(const std::string& section, const std::string& key)
    {
        if (!file_.is_open())
        {
            mprintfE("File not opened!\n");
            return std::string();
        }
        if (key_map_.find(section) == key_map_.end())
        {
            mprintfW("The section:%s is not exist! Please check it.\n", section.c_str());
            return std::string();
        }
        if (key_map_[section].find(key) == key_map_[section].end())
        {
            mprintfW("The key:%s is not exist! Please check it.\n", key.c_str());
            return std::string();
        }
        KeyInfo& key_info = key_map_[section][key];
        // std::string& line = *key_info.line;
        return (*key_info.line).substr(key_info.value_pos, key_info.value_len);
    }

    inline std::string IniFile::getValue(const std::string& key)
    {
        return this->getValue(std::string(), key);
    }

    template <class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type>
    inline void IniFile::setValue(const std::string& section, const std::string& key, const T& value)
    {
        if (!file_.is_open())
        {
            mprintfE("File not opened!\n");
            return;
        }
        rwstatus_ = 'w';
        if (key_map_.find(section) == key_map_.end())
        {
            if (section.empty())
            {
                std::string value_str = mstr::toStr(value);
                content_list_.emplace_back(key + sep_ + value_str);
                section_map_[section].last = --content_list_.end();

                key_map_[section][key].line = --content_list_.end();
                key_map_[section][key].key_pos = 0;
                key_map_[section][key].key_len = key.size();
                key_map_[section][key].value_pos = key.size() + 1;
                key_map_[section][key].value_len = value_str.size();
            }
            else
            {
                content_list_.emplace_back("[" + section + "]");
                section_map_[section].line = --content_list_.end();
                section_map_[section].last = --content_list_.end();
                section_map_[section].pos = 1;
                section_map_[section].len = section.size();
                key_map_[section] = {};
                this->setValue(section, key, value);
            }
        }
        else
        {
            if (key_map_[section].find(key) == key_map_[section].end())
            {
                std::string value_str = mstr::toStr(value);
                auto bak = section_map_[section].last;
                key_map_[section][key].line = content_list_.emplace(++section_map_[section].last, key + sep_ + value_str);
                section_map_[section].last = ++bak;
                key_map_[section][key].key_pos = 0;
                key_map_[section][key].key_len = key.size();
                key_map_[section][key].value_pos = key.size() + 1;
                key_map_[section][key].value_len = value_str.size();
            }
            else
            {
                std::string value_str = mstr::toStr(value);
                *key_map_[section][key].line = key + sep_ + value_str;
                key_map_[section][key].value_len = value_str.size();
            }
        }
    }

    inline void IniFile::printContents()
    {
        printf("IniFile %s:\n", file_path_.c_str());
        for (auto& line : content_list_)
        {
            printf("    %s\n", line.c_str());
        }
        printf("\n");
    }

    template <class T, typename std::enable_if<mtype::StdCoutEachChecker<T>::value, int>::type>
    inline void IniFile::setValue(const std::string& key, const T& value)
    {
        this->setValue(std::string(), key, value);
    }

    inline IniFile::~IniFile()
    {
        this->close();
    }

    inline bool IniFile::searchSection(const std::string& line, SectionInfo& section_info, size_t offset)
    {
        // 找到注释的位置
        size_t note_pos = line.size();
        size_t tmp_pos = line.find(note_sign_);
        if (tmp_pos < note_pos)
            note_pos = tmp_pos;

        size_t pos0 = line.find_first_not_of(" \t\n\r\f\v", offset);
        if (pos0 >= note_pos || line[pos0] != '[')
            return false;
        size_t pos1 = line.find_last_not_of(" \t\n\r\f\v", note_pos - 1);
        if (pos1 >= note_pos || line[pos1] != ']')
            return false;
        if (pos1 <= pos0 + 1)
            return false;

        size_t sec_pos0 = line.find_first_not_of(" \t\n\r\f\v", pos0 + 1);
        size_t sec_pos1 = line.find_last_not_of(" \t\n\r\f\v", pos1 - 1);

        if (sec_pos1 >= sec_pos0)
        {
            std::string value = line.substr(sec_pos0, sec_pos1 - sec_pos0 + 1);
            if (!this->checkSectionKeyFmt(value))
            {
                mprintfW("Invalid section:%s in line:%s! Ignored\n", value.c_str(), line.c_str());
                return false;
            }
            section_info.pos = sec_pos0;
            section_info.len = sec_pos1 - sec_pos0 + 1;
            return true;
        }
        else
            return false;
    }

    inline bool IniFile::searchKey(const std::string& line, KeyInfo& key_info, size_t offset)
    {
        // 找到注释的位置
        size_t note_pos = line.size();
        size_t tmp_pos = line.find(note_sign_);
        if (tmp_pos < note_pos)
            note_pos = tmp_pos;

        size_t sep_pos = line.find(sep_);
        if (sep_pos >= note_pos)
            return false;

        std::string key = mstr::trim(line.substr(offset, sep_pos), " \t\n\r\f\v");
        if (!this->checkSectionKeyFmt(key))
        {
            mprintfW("Invalid key:%s in line:%s! Ignored\n", key.c_str(), line.c_str());
            return false;
        }

        key_info.key_pos = line.rfind(key, sep_pos);
        key_info.key_len = key.size();

        size_t v_pos0 = line.find_first_not_of(" \t\n\r\f\v", sep_pos + 1);
        size_t v_pos1 = line.find_last_not_of(" \t\n\r\f\v", note_pos - 1);

        if (v_pos1 >= v_pos0)
        {
            std::string value = line.substr(v_pos0, v_pos1 - v_pos0 + 1);
            key_info.value_pos = v_pos0;
            key_info.value_len = v_pos1 - v_pos0 + 1;
            return true;
        }
        else
            return false;
    }

    inline bool IniFile::checkSectionKeyFmt(const std::string& section_or_key)
    {
        if (section_or_key.empty() || !std::isalnum(section_or_key[0]))
            return false;
        for (size_t i = 1; i < section_or_key.size(); ++i)
        {
            const auto& c = section_or_key[i];
            if (!std::isalnum(c) && c != '_' && c != '-')
                return false;
        }
        return true;
    }

    inline int IniFile::saveContents()
    {
        file_.close();
        file_.open(file_path_, std::ios::binary | std::ios::trunc | std::ios::out);
        if (!file_.is_open())
        {
            mprintfE("Open %s failed!\n", file_path_.c_str());
            return -1;
        }
        size_t i = 0;
        for (auto& content : content_list_)
        {
            if (i < content_list_.size() - 1)
                file_ << content << "\n";
            else
                file_ << content;
            i++;
        }
        return 0;
    }

}  // namespace mfile





/*--------------------------------------------单元测试--------------------------------------------*/
#ifdef MINEUTILS_TEST_MODULES
namespace _miocheck
{
    inline void IniFileTest()
    {
    }

}  // namespace _miocheck
#endif
}  // namespace mineutils

#endif  // !FILE_HPP_MINEUTILS