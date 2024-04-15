//mineutils库的版本信息及完整实现需要的基本工具
#pragma once
#ifndef BASE_HPP_MINEUTILS
#define BASE_HPP_MINEUTILS

#include<stdio.h>
#include<string>
#include<tuple>

#define MINEUTILS_MAJOR_VERSION "1"   //主版本号，对应不向下兼容的API或文件改动
#define MINEUTILS_MINOR_VERSION "2"   //次版本号，对应不影响现有API使用的新功能增加
#define MINEUTILS_PATCH_VERSION "0"   //修订版本号，对应不改变接口的BUG修复或效能优化
#define MINEUTILS_DATE_VERSION "20240422-Release"   //日期版本号，对应文档和注释级别的改动和测试阶段

#ifdef __GNUC__ 
#define MINE_FUNCSIG __PRETTY_FUNCTION__
#define MINE_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
#elif defined(_MSC_VER)
#define MINE_FUNCSIG __FUNCSIG__
#define MINE_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define MINE_FUNCSIG __func__
#define MINE_DEPRECATED(msg)
#endif 


namespace mineutils
{
    namespace mbase
    {
        static volatile char MINEUTILS_VERSION[64] = "mineutils version: " MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;

        //获取mineutils库的版本
        inline std::string getVersion()
        {
            char version[64];
            snprintf(version, 64, "mineutilshpp version is %s.%s.%s-%s", MINEUTILS_MAJOR_VERSION, MINEUTILS_MINOR_VERSION, MINEUTILS_PATCH_VERSION, MINEUTILS_DATE_VERSION);
            return version;
        }


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
        static std::tuple<mbase::CaseTag0&, mbase::CaseTag1&>& BOOL_CASE_TAGS = mbase::_creatBoolCaseTags();
    }
}

#endif // !BASE_HPP_MINEUTILS