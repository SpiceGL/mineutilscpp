//mineutils库的终端输入输出相关
#pragma once
#ifndef IO_HPP_MINEUTILS
#define IO_HPP_MINEUTILS

#include<array>
#include<deque>
#include<exception>
#include<forward_list>
#include<initializer_list>
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
#endif
#include<typeinfo>

#include"base.hpp"
#include"type.hpp"
#include"str.hpp"


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
        template<class T, class... Args>
        void print(const T& arg, const Args&... args);


        //qnx的gcc4.7.3对std::array的列表初始化支持不好，因此创建BooleanOption类用于方便地列表初始化
        class BooleanOption
        {
        public:
            BooleanOption(std::string shortflag, std::string longflag, std::string description);

            std::string& operator[](size_t id);
            const std::string& operator[](size_t id) const;
        private:
            std::array<std::string, 3> data_;
        };

        //qnx的gcc4.7.3对std::array的列表初始化支持不好，因此创建ValueOption类用于方便地列表初始化
        class ValueOption
        {
        public:
            ValueOption(std::string shortflag, std::string longflag, std::string description, std::string default_value);

            std::string& operator[](size_t id);
            const std::string& operator[](size_t id) const;
        private:
            std::array<std::string, 4> data_;
        };


        /*  main函数的参数解析工具
            - 先调用parse，再调用其他方法
            - 短标志以单横线 - 起始，接单字母，如 -s
            - 长标志以双横线 -- 起始，如 --flag
            - 支持短标志合并，但值选项只能作为合并的最后一位，如 -v value -b -B 合并为 -bBv value  */
        class ArgumentParser
        {
        public:
            ArgumentParser();

            /*  解析main函数接收的参数
                @param boolopts_preset: 预设的布尔选项参数，格式为{{"-s", "--longflag", "description"}, ...}，短标志和长标志至少需要给出一个
                @param valueopts_preset: 预设的值选项参数，格式为{{"-s", "--longflag", "description", "default value"}, ...}，短标志和长标志至少需要给出一个，默认值可以为空
                @return 0代表成功，其他代表失败   */
            int parse(int argc, char* argv[], std::vector<BooleanOption> boolopts_preset, std::vector<ValueOption> valueopts_preset);

            //获取布尔选项flag的解析结果，注意flag必须带有"-"，即"-b"或"--bool"形式
            bool getBoolOpt(const std::string& flag);

            //获取值选项flag的解析结果，生命周期为ArgumentParser对象销毁或重新parse之前。注意flag必须带有"-"，即"-a"或"--arg"形式
            const char* getValueOpt(const std::string& flag);

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
                    -shortflag  --longflag    Parsed boolean value: true
                    -shortflag  --longflag    Parsed boolean value: false
                    ...
                Parsed Value Options:
                    -shortflag  --longflag    Parsed value: parsed value
                    -shortflag  --longflag    No value parsed!
                    ...                                                             */
            void printParsed();

            //禁止拷贝和移动
            ArgumentParser(const ArgumentParser& tmp) = delete;
            ArgumentParser& operator=(const ArgumentParser& tmp) = delete;

        private:
            bool checkPresetsAreValid(const std::vector<BooleanOption>& boolopts_preset, const std::vector<ValueOption>& valueopts_preset);

            std::vector<BooleanOption> boolopts_preset_;
            std::vector<ValueOption> valueopts_preset_;
            std::unordered_set<std::string> boolopts_parsed_;
            std::unordered_map<std::string, std::string> valueopts_parsed_;
            size_t max_flag_size_;
        public:
            //获取解析的参数中是否指定了的布尔选项flag，注意flag必须带有"-"，即"-b"或"--bool"形式
            _mdeprecated("Deprecated! Please use getBoolOpt instead.") bool getParsedBoolOpt(const std::string& flag);

            //获取值选项flag的解析结果值，生命周期为ArgumentParser对象销毁或重新parse之前。注意flag必须带有"-"，即"-a"或"--arg"形式
            _mdeprecated("Deprecated! Please use getValueOpt instead.") const char* getParsedValueOpt(const std::string& flag);
        };
    }

}


namespace mineutils
{
    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mio
    {
        template<class T, class... Args>
        void _recurPrint(const T& arg, const Args&... args);

        void _recurPrint();


        _MINE_EXPORT inline std::mutex& _immutableGetPrintlock()
        {
            static std::mutex lk;
            return lk;
        }

        template<class T, class... Args>
        inline void print(const T& arg, const Args&... args)
        {
            std::lock_guard<std::mutex> lk(mio::_immutableGetPrintlock());
            //std::cout << std::fixed;
            mio::_recurPrint(arg, args...);
        }

        template<class T, class... Args>
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
            this->data_[0] = std::move(shortflag);
            this->data_[1] = std::move(longflag);
            this->data_[2] = std::move(description);
        }

        inline std::string& BooleanOption::operator[](size_t id)
        {
            return this->data_[id];
        }

        inline const std::string& BooleanOption::operator[](size_t id) const
        {
            return this->data_[id];
        }

        inline ValueOption::ValueOption(std::string shortflag, std::string longflag, std::string description, std::string default_value)
        {
            this->data_[0] = std::move(shortflag);
            this->data_[1] = std::move(longflag);
            this->data_[2] = std::move(description);
            this->data_[3] = std::move(default_value);
        }

        inline std::string& ValueOption::operator[](size_t id)
        {
            return this->data_[id];
        }

        inline const std::string& ValueOption::operator[](size_t id) const
        {
            return this->data_[id];
        }

        inline ArgumentParser::ArgumentParser()
        {
            this->max_flag_size_ = 0;
        }

        inline int ArgumentParser::parse(int argc, char* argv[], std::vector<BooleanOption> boolopts_preset, std::vector<ValueOption> valueopts_preset)
        {
            if (!this->checkPresetsAreValid(boolopts_preset, valueopts_preset))
                return -1;
            this->max_flag_size_ = 0;
            this->boolopts_preset_ = std::move(boolopts_preset);
            this->valueopts_preset_ = std::move(valueopts_preset);

            std::vector<int> boolop_values;   //存放布尔开关的值
            boolop_values.reserve(this->boolopts_preset_.size());
            std::unordered_map<std::string, int*> boolop_keys;   //存放布尔开关的键
            boolop_keys.reserve(this->boolopts_preset_.size() * 2);

            size_t now_vsize;
            for (auto& boolop : this->boolopts_preset_)
            {
                boolop_values.push_back(0);
                boolop_keys[boolop[0]] = &boolop_values.back();
                boolop_keys[boolop[1]] = &boolop_values.back();

                if (boolop[0].empty() || boolop[1].empty())
                    now_vsize = boolop[0].size() + boolop[1].size();
                else now_vsize = boolop[0].size() + boolop[1].size() + 2;
                this->max_flag_size_ = now_vsize > this->max_flag_size_ ? now_vsize : this->max_flag_size_;
            }

            std::vector<std::string> valueop_values;
            valueop_values.reserve(this->valueopts_preset_.size());
            std::unordered_map<std::string, std::string*> valueop_keys;
            valueop_keys.reserve(this->valueopts_preset_.size() * 2);

            for (auto& valueop : this->valueopts_preset_)
            {
                valueop_values.push_back(valueop[3]);
                valueop_keys[valueop[0]] = &valueop_values.back();
                valueop_keys[valueop[1]] = &valueop_values.back();

                if (valueop[0].empty() || valueop[1].empty())
                    now_vsize = valueop[0].size() + valueop[1].size();
                else now_vsize = valueop[0].size() + valueop[1].size() + 2;

                this->max_flag_size_ = now_vsize > this->max_flag_size_ ? now_vsize : this->max_flag_size_;
            }

            if (argc < 1)
            {
                mprintfW("Wrong value of argc:%d!\n", argc);
                return 1;
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
                    //如果value option是最后一个argv，或value option的下一个argv是另一个opthion，init失败
                    if (i < argc - 1 && boolop_keys.find(argv[i + 1]) == boolop_keys.end() && valueop_keys.find(argv[i + 1]) == valueop_keys.end())
                        *valueop_keys[flag_maybe] = argv[i + 1];
                    else return -1;
                }
            }
            this->boolopts_parsed_.clear();
            this->valueopts_parsed_.clear();
            for (auto& boolop_key : boolop_keys)
            {
                if (*boolop_key.second)
                    this->boolopts_parsed_.emplace(boolop_key.first);
            }
            for (auto& valueop_key : valueop_keys)
            {
                this->valueopts_parsed_[valueop_key.first] = *valueop_key.second;
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
            return this->boolopts_parsed_.find(flag) != this->boolopts_parsed_.end();
        }

        inline bool ArgumentParser::getParsedBoolOpt(const std::string& flag)
        {
            return this->getBoolOpt(flag);
        }

        //inline const char* ArgumentParser::getPresetValueOpt(const std::string& flag)
        //{
        //    for (const auto& v_preset : this->valueopts_preset_)
        //    {
        //        if (v_preset[0] == flag || v_preset[1] == flag)
        //            return v_preset[3].c_str();
        //    }
        //    return "";
        //}

        inline const char* ArgumentParser::getValueOpt(const std::string& flag)
        {
            if (flag.empty())
            {
                mprintfW("Got an empty flag!\n");
                return "";
            }
            if (this->valueopts_parsed_.find(flag) != this->valueopts_parsed_.end())
                return this->valueopts_parsed_[flag].c_str();
            return "";
        }

        inline const char* ArgumentParser::getParsedValueOpt(const std::string& flag)
        {
            return this->getValueOpt(flag);
        }

        inline void ArgumentParser::printPreset()
        {
            if (!this->boolopts_preset_.empty())
                //printf("Preset Boolean Options:\n");
                printf("Preset Boolean Options:\n");
            for (auto& boolop : this->boolopts_preset_)
            {
                std::string flag_part;
                if (boolop[0].empty() || boolop[1].empty())
                    flag_part = boolop[0] + boolop[1];
                else flag_part = boolop[0] + ", " + boolop[1];
                flag_part.resize(this->max_flag_size_, ' ');
                printf("    %s    %s\n", flag_part.c_str(), boolop[2].empty() ? "" : ("[Description] " + boolop[2]).c_str());
            }

            if (!this->valueopts_preset_.empty())
                printf("Preset Value Options:\n");
            for (auto& valueop : this->valueopts_preset_)
            {
                std::string flag_part;
                if (valueop[0].empty() || valueop[1].empty())
                    flag_part = valueop[0] + valueop[1];
                else flag_part = valueop[0] + ", " + valueop[1];
                flag_part.resize(this->max_flag_size_, ' ');
                printf("    %s    %s %s\n", flag_part.c_str(), valueop[2].empty() ? "" : ("[Description] " + valueop[2]).c_str(), valueop[3].empty() ? "(Required)" : ("(Default: " + valueop[3] + ")").c_str());
            }
            printf("\n");
        }

        inline void ArgumentParser::printParsed()
        {
            if (!this->boolopts_preset_.empty())
                printf("Parsed Boolean Options:\n");
            for (auto& boolop : this->boolopts_preset_)
            {
                std::string flag_part;
                if (boolop[0].empty() || boolop[1].empty())
                    flag_part = boolop[0] + boolop[1];
                else flag_part = boolop[0] + ", " + boolop[1];
                flag_part.resize(this->max_flag_size_, ' ');
                std::string flag = boolop[0].empty() ? boolop[1] : boolop[0];
                printf("    %s    Value: %s\n", flag_part.c_str(), this->boolopts_parsed_.find(flag) != this->boolopts_parsed_.end() ? "true" : "false");
            }

            if (!this->valueopts_preset_.empty())
                printf("Parsed Value Options:\n");
            for (auto& valueop : this->valueopts_preset_)
            {
                std::string flag_part;
                if (valueop[0].empty() || valueop[1].empty())
                    flag_part = valueop[0] + valueop[1];
                else flag_part = valueop[0] + ", " + valueop[1];
                flag_part.resize(this->max_flag_size_, ' ');
                std::string flag = valueop[0].empty() ? valueop[1] : valueop[0];
                printf("    %s    Value: %s\n", flag_part.c_str(), this->valueopts_parsed_[flag].c_str());
            }
            printf("\n");
        }

        inline bool ArgumentParser::checkPresetsAreValid(const std::vector<BooleanOption>& boolopts_preset, const std::vector<ValueOption>& valueopts_preset)
        {
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
                    if (boolop[0].size() != 2 || boolop[0][0] != '-' || boolop[0][1] == '-' || mstr::split(boolop[0])[0] != boolop[0])
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
                    if (boolop[1].size() < 3 || boolop[1].substr(0, 2) != "--" || boolop[1][2] == '-' || mstr::split(boolop[1])[0] != boolop[1])
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
                    if (value_op[0].size() != 2 || value_op[0][0] != '-' || value_op[0][1] == '-' || mstr::split(value_op[0])[0] != value_op[0])
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
                    if (value_op[1].size() < 3 || value_op[1].substr(0, 2) != "--" || value_op[1][2] == '-' || mstr::split(value_op[1])[0] != value_op[1])
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
    }


#ifdef MINEUTILS_TEST_MODULES
    namespace _miocheck
    {
        void inline func1(int) {}
        inline void printTest()
        {
            std::unordered_multimap<int, float> m1 = { {0, 0.1}, {0, 1.1} };
            std::list<float> list1 = { 5, 6, 7,8,8,7,6 };
            std::vector<double> vec = { 1.1, 2.2, 3.3 };
            std::priority_queue<double> qe2(vec.begin(), vec.end());
            std::vector<std::vector<double>> vecvec({ vec, vec });
            std::forward_list<int> fl({ 1,2,3 });
            std::initializer_list<int> initl({ 1, 2, 3 });

            printf("User check! Expected output: {0:0.1, 0:1.1} {5, 6, 7, 8, 8, 7, 6} {6.1, 5.3, 5.1} {3.3, 2.2, 1.1} {{1.1, 2.2, 3.3}, {1.1, 2.2, 3.3}} {1, 2, 3} {1, 2, 3}\n");
            mio::print("              Actual output:", m1, list1, std::stack<double>({ 5.1, 5.3, 6.1 }), qe2, vecvec, fl, initl);
            printf("User check! Expected output: void (int) \n");
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
                { {"-a", "--aa", "bool switch1"}, {"-b", "--bb", "bool switch2"}, {"-c", "--cc", "bool switch3"}, {"-d", "--dd", "bool switch4"} },
                { {"-A", "--AA", "value1", "111"}, {"-B", "--BB", "value02", "222"}, {"", "--CC", "value003", "333"}, {"-D", "--DD", "value4", ""} });

            if (!(ret0 == 0)) mprintfE(R"(Failed when check: parser.parse)""\n");


            bool ret1 = parser.getBoolOpt("-a");
            if (!ret1) mprintfE(R"(Failed when check: parser.getBoolOpt("-a"):%d)""\n", ret1);
            ret1 = parser.getBoolOpt("--aa");
            if (!ret1) mprintfE(R"(Failed when check: ArgumentParser::getBoolOpt("--aa"):%d)""\n", ret1);
            ret1 = parser.getBoolOpt("-b");
            if (ret1) mprintfE(R"(Failed when check: parser.getBoolOpt("-"):%d)""\n", ret1);
            ret1 = parser.getBoolOpt("--bb");
            if (ret1) mprintfE(R"(Failed when check: parser.getBoolOpt("--bb"):%d)""\n", ret1);
            ret1 = parser.getBoolOpt("");
            if (ret1) mprintfE(R"(Failed when check: parser.getBoolOpt(""):%d)""\n", ret1);

            ret1 = parser.getBoolOpt("--cc");
            if (!ret1) mprintfE(R"(Failed when check: parser.getBoolOpt("--cc"):%d)""\n", ret1);
            ret1 = parser.getBoolOpt("-d");
            if (!ret1) mprintfE(R"(Failed when check: parser.getBoolOpt("-d"):%d)""\n", ret1);

            std::string ret2 = parser.getValueOpt("-A");
            if (!(ret2 == "1")) mprintfE(R"(Failed when check: parser.getValueOpt("-A"):%s)""\n", ret2.c_str());

            ret2 = parser.getValueOpt("--BB");
            if (!(ret2 == "2")) mprintfE(R"(Failed when check: parser.getValueOpt("--BB"):%s)""\n", ret2.c_str());

            ret2 = parser.getValueOpt("--CC");
            if (!(ret2 == "3")) mprintfE(R"(Failed when check: parser.getValueOpt("--CC"):%s)""\n", ret2.c_str());
            ret2 = parser.getValueOpt("-D");
            if (!(ret2 == "4")) mprintfE(R"(Failed when check: parser.getValueOpt("-D"):%s)""\n", ret2.c_str());

            printf("User check:\n");
            parser.printPreset();
            parser.printParsed();
            printf("User check:\n");
            ret0 = parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "--AA", "value1", "111"} });
            if (ret0 == 0) mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "--AA", "value1", "111"} }))""\n");
            ret0 = parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"- ", "--AA", "value1", "111"} });
            if (ret0 == 0) mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"- ", "--AA", "value1", "111"} }))""\n");
            ret0 = parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A", "--A A", "value1", "111"} });
            if (ret0 == 0) mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "--A A", "value1", "111"} }))""\n");
            ret0 = parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A", "-AA", "value1", "111"} });
            if (ret0 == 0) mprintfE(R"(Failed when check: parser.parse(argv_vec.size(), argv_vec.data(), { }, { {"-A ", "-AA", "value1", "111"} }))""\n");
        }

        inline void check()
        {
            printf("\n--------------------check mio start--------------------\n");
            printTest();
            parseArgsTest();
            printf("---------------------check mio end---------------------\n\n");
        }
    }
#endif
}


#endif // !IO_HPP_MINEUTILS
