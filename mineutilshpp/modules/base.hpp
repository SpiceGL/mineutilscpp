//mineutils库的版本信息及完整实现需要的基本工具
#pragma once
#ifndef BASE_HPP_MINEUTILS
#define BASE_HPP_MINEUTILS

#include<sstream>
#include<stdio.h>
#include<string>
#include<tuple>
#include<type_traits>

#define MINEUTILS_MAJOR_VERSION "1"   //主版本号，对应不向下兼容的API或文件改动
#define MINEUTILS_MINOR_VERSION "6"   //次版本号，对应不影响现有API使用的新功能增加
#define MINEUTILS_PATCH_VERSION "0"   //修订版本号，对应不改变接口的BUG修复或效能优化
#define MINEUTILS_DATE_VERSION "20240729-release"   //日期版本号，对应文档和注释级别的改动和测试阶段


#ifdef __GNUC__ 
#define MINE_FUNCSIG __PRETTY_FUNCTION__
#define MINE_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define MINE_FUNCSIG __FUNCSIG__
#define MINE_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define MINE_FUNCSIG __func__
#define MINE_DEPRECATED(msg)
#endif 

#ifdef __QNX__
#define MINE_THREAD_LOCAL
#else 
#define MINE_THREAD_LOCAL thread_local
#endif // __QNX__


namespace mineutils
{
    namespace mbase
    {
        /*--------------------------------------------用户接口--------------------------------------------*/

        //获取mineutils库的版本
        std::string getVersion();

        //打印mineutils库的版本
        int printVersion(const std::string& project_name);










        


        /*--------------------------------------------内部实现--------------------------------------------*/
         
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
        MINE_DEPRECATED("Deprecated. Please replace with std::integral_constant<bool, value>()")
            static std::tuple<mbase::CaseTag0&, mbase::CaseTag1&> BOOL_CASE_TAGS = mbase::_creatBoolCaseTags();
#ifdef _MSC_VER
        class MINE_DEPRECATED("Deprecated. Please replace with std::false_type") CaseTag0;
        class MINE_DEPRECATED("Deprecated. Please replace with std::true_type") CaseTag1;
#endif // _MSC_VER

        inline volatile char* _keepVersionString()
        {
            static volatile char MINEUTILS_VERSION[64] = "mineutils version: " MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
            return MINEUTILS_VERSION;
        }
        static volatile char* _tmp_keepVersionString = _keepVersionString();

        inline std::string getVersion()
        {
            std::string MINEUTILS_VERSION = "mineutils-" MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
            return MINEUTILS_VERSION;
        }

        inline int printVersion(const std::string& project_name)
        {
            std::string MINEUTILS_VERSION = mbase::getVersion();
            std::string sep_line;           
            sep_line.resize(MINEUTILS_VERSION.size() + project_name.size() + 7, '-');
            return printf("%s\n%s using %s\n%s\n", sep_line.c_str(), project_name.c_str(), MINEUTILS_VERSION.c_str(), sep_line.c_str());
        }
        
        //static std::ostringstream _ostream;
    }
}

#endif // !BASE_HPP_MINEUTILS