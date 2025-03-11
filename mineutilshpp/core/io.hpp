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


        //main函数的参数解析工具，先parse，再调用其他接口
        class ArgumentParser
        {
        public:
            ArgumentParser();

            /*  解析main函数接收的参数
                @param boolopts_preset: 预设的布尔选项参数，格式为{{"-shortflag", "--longflag", "description"}, ...}，短标志和长标志至少需要给出一个
                @param valueopts_preset: 预设的值选项参数，格式为{{"-shortflag", "--longflag", "description", "default value"}, ...}，短标志和长标志至少需要给出一个，默认值可以为空
                @return 0代表成功，其他代表失败   */
            int parse(int argc, char* argv[], std::vector<BooleanOption> boolopts_preset, std::vector<ValueOption> valueopts_preset);

            //获取解析的参数中是否指定了的布尔选项flag，注意flag必须带有"-"，即"-b"或"--bool"形式
            bool getParsedBoolOpt(const std::string& flag);

            //获取解析的参数中指定的值选项flag的值，生命周期为销毁或重新parse之前。如果未指定则获得预设的默认值。注意flag必须带有"-"，即"-a"或"--arg"形式
            const char* getParsedValueOpt(const std::string& flag);

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
            int checkPresetsAreValid(const std::vector<BooleanOption>& boolopts_preset, const std::vector<ValueOption>& valueopts_preset);

            std::vector<BooleanOption> boolopts_preset_;
            std::vector<ValueOption> valueopts_preset_;
            std::unordered_set<std::string> boolopts_parsed_;
            std::unordered_map<std::string, std::string> valueopts_parsed_;
            size_t max_flag_size_;
            size_t max_desc_size_;
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
                if (boolop_keys.find(argv[i]) != boolop_keys.end())
                {
                    *boolop_keys[argv[i]] = 1;
                }
                else if (valueop_keys.find(argv[i]) != valueop_keys.end())
                {
                    //如果value option是最后一个argv，或value option的下一个argv是另一个opthion，init失败
                    if (i < argc - 1 && boolop_keys.find(argv[i + 1]) == boolop_keys.end() && valueop_keys.find(argv[i + 1]) == valueop_keys.end())
                        *valueop_keys[argv[i]] = argv[i + 1];
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

        inline bool ArgumentParser::getParsedBoolOpt(const std::string& flag)
        {
            if (flag.empty())
            {
                mprintfW("Got an empty flag!\n");
                return false;
            }
            return this->boolopts_parsed_.find(flag) != this->boolopts_parsed_.end();
        }

        inline const char* ArgumentParser::getParsedValueOpt(const std::string& flag)
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
                printf("    %s    %s %s\n", flag_part.c_str(), valueop[2].empty() ? "" : ("[Description] " + valueop[2]).c_str(), valueop[3].empty() ? "(Required)" : ("(Default: " + valueop[3]+")").c_str());
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

        inline int ArgumentParser::checkPresetsAreValid(const std::vector<BooleanOption>& boolopts_preset, const std::vector<ValueOption>& valueopts_preset)
        {
            std::unordered_set<std::string> bool_flags;
            bool_flags.reserve(boolopts_preset.size() * 2);
            for (auto& boolop : boolopts_preset)
            {
                if (boolop[0].size() < 2 && boolop[1].size() < 3)
                {
                    mprintfE("{%s, %s} is invalid!\n", boolop[0].c_str(), boolop[1].c_str());
                    return false;
                }
                if (!boolop[0].empty())
                {
                    if (boolop[0][0] != '-' or boolop[0][1] == '-')
                    {
                        mprintfE("%s is invalid short option!\n", boolop[0].c_str());
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
                    if (boolop[1].substr(0, 2) != "--" or boolop[1][2] == '-')
                    {
                        mprintfE("%s is invalid long option!\n", boolop[1].c_str());
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
                if (value_op[0].size() < 2 && value_op[1].size() < 3)
                {
                    mprintfE("The option {%s, %s} is invalid!\n", value_op[0].c_str(), value_op[1].c_str());
                    return false;
                }
                if (!value_op[0].empty())
                {
                    if (value_op[0][0] != '-' or value_op[0][1] == '-')
                    {
                        mprintfE("%s is invalid short option!\n", value_op[0].c_str());
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
                    if (value_op[1].substr(0, 2) != "--" or value_op[1][2] == '-')
                    {
                        mprintfE("%s is invalid long option!\n", value_op[1].c_str());
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
            mio::print("              Actual output:", m1, list1, std::stack<double>({5.1, 5.3, 6.1}), qe2, vecvec, fl, initl);
            printf("User check! Expected output: void (int) \n");
            mio::print(func1);
        }


        inline void parseArgsTest()
        {
            std::vector<char*> argv_vec;
            argv_vec.resize(11);
            argv_vec[0] = (char*)"demo";
            argv_vec[1] = (char*)"-b1";
            argv_vec[2] = (char*)"--BBX";
            argv_vec[3] = (char*)"-b2";
            argv_vec[4] = (char*)"--BB3";

            argv_vec[5] = (char*)"-a1";
            argv_vec[6] = (char*)"1";
            argv_vec[7] = (char*)"--AA2";
            argv_vec[8] = (char*)"2";
            argv_vec[9] = (char*)"--AA3";
            argv_vec[10] = (char*)"3";
            mio::ArgumentParser parser;
            std::vector<std::array<std::string, 3>> bool_opts;
            int ret0 = parser.parse(argv_vec.size(), argv_vec.data(),
                { {"-b1", "--BB1", "bool switch1"}, {"-b2", "", "bool switch2"}, {"", "--BB3", "bool switch3"}, {"-b4", "--BB4", "bool switch4"} },
                { {"-a1", "--AA1", "value1", "111"}, {"-a2", "--AA2", "value02", "222"}, {"-a3", "--AA3", "value003", "333"}, {"-a4", "--AA0004", "value4", ""} });

            if (!(ret0 == 0)) mprintfE(R"(Failed when check: parser.parse)""\n");


            bool ret1 = parser.getParsedBoolOpt("-b1");
            if (!ret1) mprintfE(R"(Failed when check: parser.getParsedBoolOpt("-b1"):%d)""\n", ret1);
            ret1 = parser.getParsedBoolOpt("--BB1");
            if (!ret1) mprintfE(R"(Failed when check: ArgumentParser::getParsedBoolOpt("-b1"):%d)""\n", ret1);
            ret1 = parser.getParsedBoolOpt("-b2");
            if (!ret1) mprintfE(R"(Failed when check: parser.getParsedBoolOpt("-b2"):%d)""\n", ret1);
            ret1 = parser.getParsedBoolOpt("");
            if (ret1) mprintfE(R"(Failed when check: parser.getParsedBoolOpt(""):%d)""\n", ret1);

            ret1 = parser.getParsedBoolOpt("--BBX");
            if (ret1) mprintfE(R"(Failed when check: parser.getParsedBoolOpt("--BBX"):%d)""\n", ret1);

            ret1 = parser.getParsedBoolOpt("--BB3");
            if (!ret1) mprintfE(R"(Failed when check: parser.getParsedBoolOpt("--BB3"):%d)""\n", ret1);

            std::string ret2 = parser.getParsedValueOpt("-a1");
            if (!(ret2 == "1")) mprintfE(R"(Failed when check: parser.getParsedValueOpt("-a1"):%d)""\n", ret2.c_str());

            ret2 = parser.getParsedValueOpt("--AA2");
            if (!(ret2 == "2")) mprintfE(R"(Failed when check: parser.getParsedValueOpt("--AA2"):%d)""\n", ret2.c_str());

            ret2 = parser.getParsedValueOpt("-a3");
            if (!(ret2 == "3")) mprintfE(R"(Failed when check: parser.getParsedValueOpt("-a3"):%d)""\n", ret2.c_str());

            printf("User check:\n");
            parser.printPreset();
            parser.printParsed();
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
