/*  mineutils库的版本信息及完整实现需要的基本工具
 *  项目名称: mineutilscpp
 *  GitHub 地址: https://github.com/SpiceGL/mineutilscpp    */
#pragma once
#ifndef BASE_HPP_MINEUTILS
#define BASE_HPP_MINEUTILS

#include<sstream>
#include<stdio.h>
#include<string>
#include<string.h>
#include<tuple>
#include<typeinfo>
#include<type_traits>
#include<unordered_map>

#define MINEUTILS_MAJOR_VERSION "1"   //主版本号，对应不向下兼容的API或文件改动
#define MINEUTILS_MINOR_VERSION "16"   //次版本号，对应不影响现有API使用的新功能增加
#define MINEUTILS_PATCH_VERSION "1"   //修订版本号，对应不改变API的BUG修复或效能优化
#define MINEUTILS_DATE_VERSION "20241231-release"   //日期版本号，对应文档和注释级别的改动和测试阶段
#ifdef __GNUC__ 
#include<cxxabi.h>
#endif

#ifdef __GNUC__ 
#define MINE_FUNCSIG __PRETTY_FUNCTION__
#define mdeprecated(msg) __attribute__((deprecated(msg)))
#define mlikely(condition) __builtin_expect(!!(condition), 1)
#define munlikely(condition) __builtin_expect(!!(condition), 0)
#elif defined(_MSC_VER)
#define MINE_FUNCSIG __FUNCSIG__    //依赖编译器实现的函数名信息
#define mdeprecated(msg) __declspec(deprecated(msg))
#define mlikely(condition) (condition) 
#define munlikely(condition) (condition)
#else
#define MINE_FUNCSIG __func__
#define mdeprecated(msg)
#define mlikely(condition) (condition) 
#define munlikely(condition) (condition)
#endif 

//在不支持thread_local的编译器中，仅相当于普通变量，没有线程内只有一份实例的作用
#if defined(__GNUC__) && (__GNUC__ < 5 && __GNUC_MINOR__ < 8)  //for qnx660
#define MINE_THREAD_LOCAL_IF_HAVE
#else 
#define MINE_THREAD_LOCAL_IF_HAVE thread_local   
#endif 

//命名空间::类名::函数名格式的const char*字符串
#define MINE_FUNCNAME mineutils::mbase::_splitFuncName(MINE_FUNCSIG, __func__)


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mbase
    {
        //获取mineutils库的版本
        std::string getVersion();

        //打印mineutils库的版本
        int printVersion(const std::string& project_name);
    }









        


    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mbase
    {
        inline const std::string& _getDeprecatedWarningStr()
        {
            static std::string str = "!Warning! [%s](line %d): ";
            return str;
        }

#define _mprintfDeprecatedWarning(msg) printf((mbase::_getDeprecatedWarningStr() + msg).c_str(), __FILE__, __LINE__)

        inline volatile char* _keepVersionString()
        {
            static volatile char MINEUTILS_VERSION[64] = "using mineutils version: " MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
            return MINEUTILS_VERSION;
        }
        static volatile char* _tmp_keepVersionString = mbase::_keepVersionString();

        inline std::string getVersion()
        {
            return "mineutils-" MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
        }

        inline int printVersion(const std::string& project_name)
        {
            const char ver[] = "mineutils-" MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
            std::string sep_line;
            sep_line.resize(sizeof(ver) + project_name.size() + 6, '-');
            return printf("%s\n%s using %s\n%s\n", sep_line.c_str(), project_name.c_str(), ver, sep_line.c_str());
        }


        //template<size_t N>
        //inline std::string _splitFuncName(const char* func_sig, const char(&func_name)[N])
        //{
        //    std::string s_func_sig = func_sig;
        //    char sep[N + 1] = "";
        //    memcpy(sep, func_name, N - 1);
        //    memcpy(sep + N - 1, "(", 2);
        //    size_t name_pos = s_func_sig.find(sep);
        //    if (name_pos == std::string::npos)
        //    {
        //        sep[N - 1] = '<';
        //        name_pos = s_func_sig.find(sep);
        //    }
        //    s_func_sig.erase(name_pos + N - 1);
        //    size_t func_start_pos = s_func_sig.rfind(' ', name_pos - 1) + 1;
        //    s_func_sig.erase(0, func_start_pos);
        //    return s_func_sig;
        //}

        inline const char* _splitFuncName(const char* func_sig, const char* func_name)
        {
            MINE_THREAD_LOCAL_IF_HAVE std::unordered_map<const char*, std::string> func_name_map;
            auto it = func_name_map.find(func_sig);
            if (it != func_name_map.end())
                return it->second.c_str();

            std::string s_func_sig = func_sig;
            size_t name_pos = s_func_sig.find(func_name + std::string("("));
            if (name_pos == std::string::npos)
            {
                name_pos = s_func_sig.find(func_name + std::string("<"));
                //处理gcc上lambda函数
                if (name_pos == std::string::npos)
                {
                    s_func_sig.append("::").append(func_name);
                    func_name_map[func_sig] = std::move(s_func_sig);
                    return func_name_map[func_sig].c_str();
                }
            }
            s_func_sig.erase(name_pos + strlen(func_name));
            s_func_sig.erase(0, s_func_sig.rfind(' ', name_pos - 1) + 1);
            func_name_map[func_sig] = std::move(s_func_sig);
            return func_name_map[func_sig].c_str();
        }

        //已废弃，使用std::false_type代替
        class CaseTag0
        {
        public:
            static CaseTag0& getInstance()
            {
                static CaseTag0 tag0;
                return tag0;
            }

        private:
            CaseTag0() {}
            CaseTag0(CaseTag0& tag) = delete;
            CaseTag0& operator=(const CaseTag0& tag) = delete;
            ~CaseTag0() {}
        };

        //已废弃，使用std::true_type代替
        class CaseTag1
        {
        public:
            static CaseTag1& getInstance()
            {
                static mbase::CaseTag1 tag1;
                return tag1;
            }

        private:
            CaseTag1() {}
            CaseTag1(const CaseTag1& tag) = delete;
            CaseTag1& operator=(const CaseTag1& tag) = delete;
            ~CaseTag1() {}
        };


        inline std::tuple<mbase::CaseTag0&, mbase::CaseTag1&>& _creatBoolCaseTags()
        {
            mbase::CaseTag0& tag0 = mbase::CaseTag0::getInstance();
            mbase::CaseTag1& tag1 = mbase::CaseTag1::getInstance();
            static std::tuple<mbase::CaseTag0&, mbase::CaseTag1&> BOOL_CASE_TAGS(tag0, tag1);

            return BOOL_CASE_TAGS;
        }

        //包含两个成员的tuple，用于重载函数的选择
        mdeprecated("Deprecated. Please replace with std::integral_constant<bool, value>()")
            static std::tuple<mbase::CaseTag0&, mbase::CaseTag1&> BOOL_CASE_TAGS = mbase::_creatBoolCaseTags();
#ifdef _MSC_VER
        class mdeprecated("Deprecated. Please replace with std::false_type") CaseTag0;
        class mdeprecated("Deprecated. Please replace with std::true_type") CaseTag1;
#endif // _MSC_VER
    }

//已废弃。mtypename在处理复杂类型(如模板嵌套类)时，无法正确解析语法，因此用mtype::getTypeName替代
#ifdef __GNUC__ 
#define mtypename(type) abi::__cxa_demangle(typeid(type).name(), nullptr, nullptr, nullptr)
#else
#define mtypename(type) typeid(type).name()
#endif
}


#define MINE_THREAD_LOCAL MINE_THREAD_LOCAL_IF_HAVE  //已废弃
#define MINE_DEPRECATED mdeprecated  //已废弃

#endif // !BASE_HPP_MINEUTILS