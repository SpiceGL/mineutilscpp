// mineutils库的终端输入输出相关
#pragma once
#ifndef IO_HPP_MINEUTILS
#define IO_HPP_MINEUTILS

#include <array>
#include <cctype>
#include <deque>
#include <exception>
#include <forward_list>
#include <initializer_list>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifdef __GNUC__
#include <cxxabi.h>
#endif
#include <typeinfo>

#include "base.hpp"
#include "type.hpp"
#include "str.hpp"


namespace mineutils
{
/*--------------------------------------------用户接口--------------------------------------------*/

namespace mio
{
    /*  实现类似Python的print打印功能，基于std::cout
        - 基于std::ostringstream实现，可以接收任意数量和类型的参数
        - 支持重载了std::ostream& operator<<(std::ostream&, const T&)的T对象
        - 扩展了对非volatile限定的STL容器对象的支持
        - 有无符号的char都会被当作字符处理
        - 宽字符会被当作数字处理
        - 未支持的类型会被转换为<ClassName: Address>形式的字符串
        - 在不混用print函数和std::cout时，线程安全  */
    template <class T, class... Args>
    void print(const T& arg, const Args&... args);

    // qnx的gcc4.7.3对std::array的列表初始化支持不好，因此创建BooleanOption类用于方便地列表初始化
    class BooleanOption final
    {
    public:
        BooleanOption(std::string shortflag, std::string longflag, std::string description);

        std::string& operator[](size_t id);
        const std::string& operator[](size_t id) const;

    private:
        std::array<std::string, 3> data_;
    };

    // qnx的gcc4.7.3对std::array的列表初始化支持不好，因此创建ValueOption类用于方便地列表初始化
    class ValueOption final
    {
    public:
        ValueOption(std::string shortflag, std::string longflag, std::string description, std::string default_value);

        std::string& operator[](size_t id);
        const std::string& operator[](size_t id) const;

    private:
        std::array<std::string, 4> data_;
    };

    /*  main函数的参数解析工具，只支持解析ASCII字符
        - 先调用parse，再调用其他方法
        - 短标志以单横线 '-'起始，接ASCII单字母，如 -s
        - 长标志以双横线 '--'起始，后续至少两位，首位接ASCII字母，后续位支持ASCII字母数字下划线及分隔符'-'，如 --flag, --long-flag
        - 支持短标志合并，但值选项只能作为合并的最后一位，如 -v value -b -B 合并为 -bBv value  
        - main函数的参数中，未在预设中的标志会被忽略，值选项的值禁止以'-'开头 */
    class ArgumentParser final
    {
    public:
        ArgumentParser();

        /*  解析main函数接收的参数，不在preset中的参数无法被解析
            @param boolopts_preset: 预设的布尔选项参数，格式为{{"-s", "--longflag", "description"}, ...}短标志和长标志至少需要给出一个
            @param valueopts_preset: 预设的值选项参数，格式为{{"-s", "--longflag", "description", "default value"}, ...}，短标志和长标志至少需要给出一个，默认值为空代表必需由用户提供的参数
            @return 0代表成功，负数代表预设或命令行参数格式有误，正数代表正常解析但未解析到必需提供的值参数  */
        int parse(int argc, char* argv[], std::vector<BooleanOption> boolopts_preset, std::vector<ValueOption> valueopts_preset);

        // 获取布尔选项flag的解析结果，注意flag必须带有"-"，即"-b"或"--bool"形式
        bool getBoolOpt(const std::string& flag);

        // 获取值选项flag的解析结果，生命周期为ArgumentParser对象销毁或重新parse之前。注意flag必须带有"-"，即"-a"或"--arg"形式
        const std::string& getValueOpt(const std::string& flag);

        /*  按一定格式打印预设的选项与描述
            Preset Boolean Options:
                -shortflag  --longflag    Description: description
                ...
            Preset Value Options:
                -shortflag  --longflag    Description: description    Default VALUE: default value
                -shortflag  --longflag    Description: description    Required
                ...                                                             */
        void printPreset();

        /*  按一定格式打印解析后的选项值
            Parsed Boolean Options:
                -shortflag  --longflag    Parsed boolean value: true
                -shortflag  --longflag    Parsed boolean value: false
                ...
            Parsed Value Options:
                -shortflag  --longflag    Parsed value: parsed value
                -shortflag  --longflag    No value parsed!
                ...                                                             */
        void printParsed();

        // 禁止拷贝和移动
        ArgumentParser(const ArgumentParser& tmp) = delete;
        ArgumentParser& operator=(const ArgumentParser& tmp) = delete;

    private:
        bool checkPresetsAreValid(const std::vector<BooleanOption>& boolopts_preset, const std::vector<ValueOption>& valueopts_preset);

        std::vector<BooleanOption> boolopts_preset_;
        std::vector<ValueOption> valueopts_preset_;
        std::unordered_set<std::string> boolopts_parsed_;
        std::unordered_map<std::string, std::string> valueopts_parsed_;
        size_t max_flag_size_;
        const std::string empty_str_;
    };

}  // namespace mio
}  // namespace mineutils





namespace mineutils
{
/*--------------------------------------------内部实现--------------------------------------------*/

namespace mio
{
    template <class T, class... Args>
    void _recurPrint(const T& arg, const Args&... args);

    void _recurPrint();

    _MINE_EXPORT inline std::mutex& _immutableGetPrintlock()
    {
        static std::mutex lk;
        return lk;
    }

    template <class T, class... Args>
    inline void print(const T& arg, const Args&... args)
    {
        std::lock_guard<std::mutex> lk(mio::_immutableGetPrintlock());
        // std::cout << std::fixed;
        mio::_recurPrint(arg, args...);
    }

    template <class T, class... Args>
    inline void _recurPrint(const T& arg, const Args&... args)
    {
        mstr::_osInput(std::cout, arg);
        std::cout << " ";
        mio::_recurPrint(args...);
    }

    //_recurPrint递归结束位置
    inline void _recurPrint()
    {
        std::cout << std::endl;
    }

    inline BooleanOption::BooleanOption(std::string shortflag, std::string longflag, std::string description)
    {
        data_[0] = std::move(shortflag);
        data_[1] = std::move(longflag);
        data_[2] = std::move(description);
    }

    inline std::string& BooleanOption::operator[](size_t id)
    {
        return data_[id];
    }

    inline const std::string& BooleanOption::operator[](size_t id) const
    {
        return data_[id];
    }

    inline ValueOption::ValueOption(std::string shortflag, std::string longflag, std::string description, std::string default_value)
    {
        data_[0] = std::move(shortflag);
        data_[1] = std::move(longflag);
        data_[2] = std::move(description);
        data_[3] = std::move(default_value);
    }

    inline std::string& ValueOption::operator[](size_t id)
    {
        return data_[id];
    }

    inline const std::string& ValueOption::operator[](size_t id) const
    {
        return data_[id];
    }

    inline ArgumentParser::ArgumentParser()
    {
        max_flag_size_ = 0;
    }

    inline int ArgumentParser::parse(int argc, char* argv[], std::vector<BooleanOption> boolopts_preset, std::vector<ValueOption> valueopts_preset)
    {
        if (!this->checkPresetsAreValid(boolopts_preset, valueopts_preset))
            return -1;
        std::string cmd_str;
        for (int i = 1; i < argc; i++) {
            cmd_str.append(argv[i]).append(" ");
        }
        mprintfI("Parsing command line: %s\n", cmd_str.c_str());
        max_flag_size_ = 0;
        boolopts_preset_ = std::move(boolopts_preset);
        valueopts_preset_ = std::move(valueopts_preset);

        std::vector<int> boolop_values;  // 存放布尔开关的值
        boolop_values.reserve(boolopts_preset_.size());
        std::unordered_map<std::string, int*> boolop_keys;  // 存放布尔开关的键
        boolop_keys.reserve(boolopts_preset_.size() * 2);

        size_t now_vsize;
        for (auto& boolop : boolopts_preset_)
        {
            boolop_values.push_back(0);
            boolop_keys[boolop[0]] = &boolop_values.back();
            boolop_keys[boolop[1]] = &boolop_values.back();

            if (boolop[0].empty() || boolop[1].empty())
                now_vsize = boolop[0].size() + boolop[1].size();
            else
                now_vsize = boolop[0].size() + boolop[1].size() + 2;
            max_flag_size_ = now_vsize > max_flag_size_ ? now_vsize : max_flag_size_;
        }

        std::vector<std::string> valueop_values;
        valueop_values.reserve(valueopts_preset_.size());
        std::unordered_map<std::string, std::string*> valueop_keys;
        valueop_keys.reserve(valueopts_preset_.size() * 2);

        for (auto& valueop : valueopts_preset_)
        {
            valueop_values.push_back(valueop[3]);
            valueop_keys[valueop[0]] = &valueop_values.back();
            valueop_keys[valueop[1]] = &valueop_values.back();

            if (valueop[0].empty() || valueop[1].empty())
                now_vsize = valueop[0].size() + valueop[1].size();
            else
                now_vsize = valueop[0].size() + valueop[1].size() + 2;

            max_flag_size_ = now_vsize > max_flag_size_ ? now_vsize : max_flag_size_;
        }

        if (argc < 1)
        {
            mprintfE("Wrong value of argc:%d!\n", argc);
            return -1;
        }
        for (int i = 1; i < argc; i++)
        {
            std::string flag_maybe = argv[i];
            if (flag_maybe.size() < 2)
                continue;
            if (flag_maybe.data()[0] == '-' && flag_maybe.data()[1] != '-' && flag_maybe.size() > 2)
            {
                for (size_t j = 1; j < flag_maybe.size() - 1; j++)
                {
                    std::string sflag = std::string("-").append(flag_maybe.substr(j, 1));
                    if (boolop_keys.find(sflag) != boolop_keys.end())
                        *boolop_keys[sflag] = 1;
                }
                flag_maybe = std::string("-").append(flag_maybe.substr(flag_maybe.size() - 1, 1));
            }
            if (boolop_keys.find(flag_maybe) != boolop_keys.end())
            {
                *boolop_keys[flag_maybe] = 1;
            }
            else if (valueop_keys.find(flag_maybe) != valueop_keys.end())
            {
                // 如果value option是最后一个argv，或value option的下一个argv是另一个opthion，init失败
                if (i < argc - 1 && argv[i + 1][0] != '-')
                {
                    *valueop_keys[flag_maybe] = argv[i + 1];
                    i++;
                }
                else
                {
                    mprintfE("Invalid value of option %s! Please check command line arguments.\n", flag_maybe.c_str());
                    return -1;
                }
            }
        }
        boolopts_parsed_.clear();
        valueopts_parsed_.clear();
        for (auto& boolop_key : boolop_keys)
        {
            if (*boolop_key.second)
                boolopts_parsed_.emplace(boolop_key.first);
        }
        for (auto& valueop_key : valueop_keys)
        {
            valueopts_parsed_[valueop_key.first] = *valueop_key.second;
        }
        for (const auto& valueop : valueopts_parsed_)
        {
            if (valueop.second.empty())
            {
                mprintfW("Required value option %s is missing!\n", valueop.first.c_str());
                return 1;
            }
        }

        return 0;
    }

    inline bool ArgumentParser::getBoolOpt(const std::string& flag)
    {
        if (flag.empty())
        {
            mprintfW("Got an empty flag!\n");
            return false;
        }
        return boolopts_parsed_.find(flag) != boolopts_parsed_.end();
    }

    inline const std::string& ArgumentParser::getValueOpt(const std::string& flag)
    {
        if (flag.empty())
        {
            mprintfW("Got an empty flag!\n");
            return empty_str_;
        }
        if (valueopts_parsed_.find(flag) == valueopts_parsed_.end())
            return empty_str_;
        return valueopts_parsed_[flag];
    }

    inline void ArgumentParser::printPreset()
    {
        if (!boolopts_preset_.empty())
            printf("Preset Boolean Options:\n");
        for (auto& boolop : boolopts_preset_)
        {
            std::string flag_part;
            if (boolop[0].empty() || boolop[1].empty())
                flag_part = boolop[0] + boolop[1];
            else
                flag_part = boolop[0] + ", " + boolop[1];
            flag_part.resize(max_flag_size_, ' ');
            printf("    %s    %s\n", flag_part.c_str(), boolop[2].empty() ? "" : ("[Description] " + boolop[2]).c_str());
        }

        if (!valueopts_preset_.empty())
            printf("Preset Value Options:\n");
        for (auto& valueop : valueopts_preset_)
        {
            std::string flag_part;
            if (valueop[0].empty() || valueop[1].empty())
                flag_part = valueop[0] + valueop[1];
            else
                flag_part = valueop[0] + ", " + valueop[1];
            flag_part.resize(max_flag_size_, ' ');
            printf("    %s    %s %s\n", flag_part.c_str(), valueop[2].empty() ? "" : ("[Description] " + valueop[2]).c_str(), valueop[3].empty() ? "(Required)" : ("(Default: " + valueop[3] + ")").c_str());
        }
        printf("\n");
    }

    inline void ArgumentParser::printParsed()
    {
        if (!boolopts_preset_.empty())
            printf("Parsed Boolean Options:\n");
        for (auto& boolop : boolopts_preset_)
        {
            std::string flag_part;
            if (boolop[0].empty() || boolop[1].empty())
                flag_part = boolop[0] + boolop[1];
            else
                flag_part = boolop[0] + ", " + boolop[1];
            flag_part.resize(max_flag_size_, ' ');
            std::string flag = boolop[0].empty() ? boolop[1] : boolop[0];
            printf("    %s    Value: %s\n", flag_part.c_str(), boolopts_parsed_.find(flag) != boolopts_parsed_.end() ? "true" : "false");
        }

        if (!valueopts_preset_.empty())
            printf("Parsed Value Options:\n");
        for (auto& valueop : valueopts_preset_)
        {
            std::string flag_part;
            if (valueop[0].empty() || valueop[1].empty())
                flag_part = valueop[0] + valueop[1];
            else
                flag_part = valueop[0] + ", " + valueop[1];
            flag_part.resize(max_flag_size_, ' ');
            std::string flag = valueop[0].empty() ? valueop[1] : valueop[0];
            printf("    %s    Value: %s\n", flag_part.c_str(), valueopts_parsed_[flag].c_str());
        }
        printf("\n");
    }

    inline bool ArgumentParser::checkPresetsAreValid(const std::vector<BooleanOption>& boolopts_preset, const std::vector<ValueOption>& valueopts_preset)
    {
        auto func_shortflag_check = [](const std::string& flag) {
            if (flag.size() != 2 || flag[0] != '-' || !std::isalpha(flag[1]))
                return false;
            return true;
        };

        auto func_longflag_check = [](const std::string& flag) {
            if (flag.size() < 4 || flag[0] != '-' || flag[1] != '-' || !std::isalpha(flag[2]))
                return false;
            for (size_t i = 3; i < flag.size(); ++i)
            {
                char c = flag[i];
                if (!std::isalnum(c) && c != '-' && c != '_')
                    return false;
            }
            return true;
        };
        std::unordered_set<std::string> bool_flags;
        bool_flags.reserve(boolopts_preset.size() * 2);
        for (auto& boolop : boolopts_preset)
        {
            if (boolop[0].empty() && boolop[1].empty())
            {
                mprintfE("Both short flag and long flag are empty()!\n");
                return false;
            }

            if (!boolop[0].empty())
            {
                if (!func_shortflag_check(boolop[0]))
                {
                    mprintfE("Invalid short flag:%s!\n", boolop[0].c_str());
                    return false;
                }
                if (bool_flags.find(boolop[0]) != bool_flags.end())
                {
                    mprintfE("Found duplicate boolean option flag:%s!\n", boolop[0].c_str());
                    return false;
                }
                bool_flags.emplace(boolop[0]);
            }

            if (!boolop[1].empty())
            {
                if (!func_longflag_check(boolop[1]))
                {
                    mprintfE("Invalid long flag:%s!\n", boolop[1].c_str());
                    return false;
                }
                if (bool_flags.find(boolop[1]) != bool_flags.end())
                {
                    mprintfE("Found duplicate boolean option flag:%s!\n", boolop[1].c_str());
                    return false;
                }
                bool_flags.emplace(boolop[1]);
            }
        }

        std::unordered_set<std::string> value_flags;
        value_flags.reserve(valueopts_preset.size() * 2);
        for (auto& value_op : valueopts_preset)
        {
            if (value_op[0].empty() && value_op[1].empty())
            {
                mprintfE("Both short flag and long flag are empty()!\n");
                return false;
            }

            if (!value_op[0].empty())
            {
                if (!func_shortflag_check(value_op[0]))
                {
                    mprintfE("Invalid short flag:%s!\n", value_op[0].c_str());
                    return false;
                }
                if (bool_flags.find(value_op[0]) != bool_flags.end())
                {
                    mprintfE("The same operation:%s is not allowed in both boolopts_preset and valueopts_preset!\n", value_op[0].c_str());
                    return false;
                }
                if (value_flags.find(value_op[0]) != value_flags.end())
                {
                    mprintfE("Found duplicate value option flag:%s!\n", value_op[0].c_str());
                    return false;
                }
                value_flags.emplace(value_op[0]);
            }

            if (!value_op[1].empty())
            {
                if (!func_longflag_check(value_op[1]))
                {
                    mprintfE("Invalid long flag:%s!\n", value_op[1].c_str());
                    return false;
                }
                if (bool_flags.find(value_op[1]) != bool_flags.end())
                {
                    mprintfE("The same operation:%s is not allowed in both boolopts_preset and valueopts_preset!\n", value_op[1].c_str());
                    return false;
                }
                if (value_flags.find(value_op[1]) != value_flags.end())
                {
                    mprintfE("Found duplicate value option flag:%s!\n", value_op[1].c_str());
                    return false;
                }
                value_flags.emplace(value_op[1]);
            }
        }
        return true;
    }

}  // namespace mio

#ifdef MINEUTILS_TEST_MODULES
namespace _miocheck
{
    void inline func1(int)
    {
    }
    inline void printTest()
    {
        std::unordered_multimap<int, float> m1 = {{0, 0.1}, {0, 1.1}};
        std::list<float> list1 = {5, 6, 7, 8, 8, 7, 6};
        std::vector<double> vec = {1.1, 2.2, 3.3};
        std::priority_queue<double> qe2(vec.begin(), vec.end());
        std::vector<std::vector<double>> vecvec({vec, vec});
        std::forward_list<int> fl({1, 2, 3});
        std::initializer_list<int> initl({1, 2, 3});

        printf("Author check! Expected output: {0:0.1, 0:1.1} {5, 6, 7, 8, 8, 7, 6} {6.1, 5.3, 5.1} {3.3, 2.2, 1.1} {{1.1, 2.2, 3.3}, {1.1, 2.2, 3.3}} {1, 2, 3} {1, 2, 3}\n");
        mio::print("              Actual output:", m1, list1, std::stack<double>({5.1, 5.3, 6.1}), qe2, vecvec, fl, initl);
        printf("Author check! Expected output: void (int) \n");
        mio::print(func1);
    }

    inline void parseArgsTest()
    {
        std::vector<char*> argv_vec;
        argv_vec.resize(11);
        argv_vec[0] = (char*)"demo";
        argv_vec[1] = (char*)"-a";
        argv_vec[2] = (char*)"nothing";
        argv_vec[3] = (char*)"-cdD";
        argv_vec[4] = (char*)"4";

        argv_vec[5] = (char*)"-A";
        argv_vec[6] = (char*)"1";
        argv_vec[7] = (char*)"--BB";
        argv_vec[8] = (char*)"2";
        argv_vec[9] = (char*)"--CC";
        argv_vec[10] = (char*)"3";
        mio::ArgumentParser parser;
        std::vector<std::array<std::string, 3>> bool_opts;
        int ret0 = parser.parse(argv_vec.size(), argv_vec.data(),
                                {{"-a", "--aa", "bool switch1"}, {"-b", "--bb", "bool switch2"}, {"-c", "--cc", "bool switch3"}, {"-d", "--dd", "bool switch4"}},
                                {{"-A", "--AA", "value1", "111"}, {"-B", "--BB", "value02", "222"}, {"", "--CC", "value003", "333"}, {"-D", "--DD", "value4", ""}});

        if (!(ret0 == 0))
            mprintfE(R"(Failed when check: parser.parse)"
                     "\n");

        bool ret1 = parser.getBoolOpt("-a");
        if (!ret1)
            mprintfE(R"(Failed when check: parser.getBoolOpt("-a"):%d)"
                     "\n",
                     ret1);
        ret1 = parser.getBoolOpt("--aa");
        if (!ret1)
            mprintfE(R"(Failed when check: ArgumentParser::getBoolOpt("--aa"):%d)"
                     "\n",
                     ret1);
        ret1 = parser.getBoolOpt("-b");
        if (ret1)
            mprintfE(R"(Failed when check: parser.getBoolOpt("-"):%d)"
                     "\n",
                     ret1);
        ret1 = parser.getBoolOpt("--bb");
        if (ret1)
            mprintfE(R"(Failed when check: parser.getBoolOpt("--bb"):%d)"
                     "\n",
                     ret1);
        ret1 = parser.getBoolOpt("");
        if (ret1)
            mprintfE(R"(Failed when check: parser.getBoolOpt(""):%d)"
                     "\n",
                     ret1);

        ret1 = parser.getBoolOpt("--cc");
        if (!ret1)
            mprintfE(R"(Failed when check: parser.getBoolOpt("--cc"):%d)"
                     "\n",
                     ret1);
        ret1 = parser.getBoolOpt("-d");
        if (!ret1)
            mprintfE(R"(Failed when check: parser.getBoolOpt("-d"):%d)"
                     "\n",
                     ret1);

        std::string ret2 = parser.getValueOpt("-A");
        if (!(ret2 == "1"))
            mprintfE(R"(Failed when check: parser.getValueOpt("-A"):%s)"
                     "\n",
                     ret2.c_str());

        ret2 = parser.getValueOpt("--BB");
        if (!(ret2 == "2"))
            mprintfE(R"(Failed when check: parser.getValueOpt("--BB"):%s)"
                     "\n",
                     ret2.c_str());

        ret2 = parser.getValueOpt("--CC");
        if (!(ret2 == "3"))
            mprintfE(R"(Failed when check: parser.getValueOpt("--CC"):%s)"
                     "\n",
                     ret2.c_str());
        ret2 = parser.getValueOpt("-D");
        if (!(ret2 == "4"))
            mprintfE(R"(Failed when check: parser.getValueOpt("-D"):%s)"
                     "\n",
                     ret2.c_str());

        printf("Author check:\n");
        parser.printPreset();
        parser.printParsed();
        printf("Author check:\n");
        ret0 = parser.parse(argv_vec.size(), argv_vec.data(), {}, {{"-A ", "--AA", "value1", "111"}});
        if (ret0 != 0)
            mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "--AA", "value1", "111"} }))"
                     "\n");
        ret0 = parser.parse(argv_vec.size(), argv_vec.data(), {}, {{"- ", "--AA", "value1", "111"}});
        if (ret0 != 0)
            mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"- ", "--AA", "value1", "111"} }))"
                     "\n");
        ret0 = parser.parse(argv_vec.size(), argv_vec.data(), {}, {{"-A", "--A A", "value1", "111"}});
        if (ret0 != 0)
            mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "--A A", "value1", "111"} }))"
                     "\n");
        ret0 = parser.parse(argv_vec.size(), argv_vec.data(), {}, {{"-A", "-AA", "value1", "111"}});
        if (ret0 != 0)
            mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "-AA", "value1", "111"} }))"
                     "\n");
    }

    inline void check()
    {
        printf("\n--------------------check mio start--------------------\n");
        printTest();
        parseArgsTest();
        printf("---------------------check mio end---------------------\n\n");
    }

}  // namespace _miocheck
#endif
}  // namespace mineutils

#endif  // !IO_HPP_MINEUTILS
