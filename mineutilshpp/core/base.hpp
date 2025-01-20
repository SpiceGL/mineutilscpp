/*  mineutils库的版本信息及完整实现需要的基本工具
 *  项目名称: mineutilscpp
 *  GitHub 地址: https://github.com/SpiceGL/mineutilscpp    */
#pragma once
#ifndef BASE_HPP_MINEUTILS
#define BASE_HPP_MINEUTILS

#include<atomic>
#include<stdio.h>
#include<string>
#include<string.h>
#include<typeinfo>
#include<type_traits>
#include<unordered_map>
#ifdef __GNUC__ 
#include<cxxabi.h>
#endif

#define MINEUTILS_MAJOR_VERSION "2"   //主版本号，对应不向下兼容的API或文件改动
#define MINEUTILS_MINOR_VERSION "0"   //次版本号，对应不影响现有API使用的新功能增加
#define MINEUTILS_PATCH_VERSION "0"   //修订版本号，对应不改变API的BUG修复或效能优化
#define MINEUTILS_DATE_VERSION "20250120-release"   //日期版本号，对应文档和注释级别的改动和测试阶段

#ifdef __GNUC__ 
#define MINE_FUNCSIG __PRETTY_FUNCTION__
#define mlikely(condition) __builtin_expect(!!(condition), 1)
#define munlikely(condition) __builtin_expect(!!(condition), 0)
#elif defined(_MSC_VER)
#define MINE_FUNCSIG __FUNCSIG__    //依赖编译器实现的函数名信息
#define mlikely(condition) (condition) 
#define munlikely(condition) (condition)
#else
#define MINE_FUNCSIG __func__
#define mlikely(condition) (condition) 
#define munlikely(condition) (condition)
#endif 

//命名空间::类名::函数名格式的const char*字符串
#define MINE_FUNCNAME mineutils::mbase::_splitFuncName(MINE_FUNCSIG, __func__)

//按printf的格式调用，打印带函数名的普通提示信息
#define mprintfI(fmt_chars, ...) mineutils::mbase::_printfI(fmt_chars, MINE_FUNCNAME, ##__VA_ARGS__)
//按printf的格式调用，打印带函数名和位置的警告信息
#define mprintfW(fmt_chars, ...) mineutils::mbase::_printfW(fmt_chars, MINE_FUNCNAME, __FILE__, __LINE__, ##__VA_ARGS__)
//按printf的格式调用，打印带函数名和位置的错误信息
#define mprintfE(fmt_chars, ...) mineutils::mbase::_printfE(fmt_chars, MINE_FUNCNAME, __FILE__, __LINE__, ##__VA_ARGS__)


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mbase
    {
        //获取mineutils库的版本
        const char* getVersion();

        //打印mineutils库的版本
        void printVersion(const char* project_name);
    }









        


    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mbase
    {
#define _mprintfDeprecatedWarning(msg) printf((mbase::_getDeprecatedWarningStr() + msg).c_str(), __FILE__, __LINE__)
        //在不支持thread_local的编译器中，仅相当于普通变量，没有线程内只有一份实例的作用
#if defined(__GNUC__) && (__GNUC__ < 5 && __GNUC_MINOR__ < 8)  //for qnx660
#define _MINE_THREAD_LOCAL_IF_HAVE
#else 
#define _MINE_THREAD_LOCAL_IF_HAVE thread_local   
#endif 

#ifdef __GNUC__ 
#define _mdeprecated(msg) __attribute__((deprecated(msg)))
#define _MINE_EXPORT __attribute__ ((visibility ("default")))
#elif defined(_MSC_VER)
#define _mdeprecated(msg) __declspec(deprecated(msg))
#define _MINE_EXPORT __declspec(dllexport)
#else
#define _mdeprecated(msg)
#define _MINE_EXPORT
#endif 


        inline const std::string& _getDeprecatedWarningStr()
        {
            static std::string str = "[WARNING][%s: line %d]";
            return str;
        }

        inline volatile char* _keepVersionString()
        {
            static volatile char MINEUTILS_VERSION[64] = "using mineutils version: " MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
            return MINEUTILS_VERSION;
        }
        static volatile char* _tmp_keepVersionString = mbase::_keepVersionString();

        inline const char* getVersion()
        {
            return "mineutils-" MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
        }

        inline void printVersion(const char* project_name)
        {
            const char ver[] = "mineutils-" MINEUTILS_MAJOR_VERSION "." MINEUTILS_MINOR_VERSION "." MINEUTILS_PATCH_VERSION "-" MINEUTILS_DATE_VERSION;
            std::string sep_line;
            sep_line.resize(sizeof(ver) + strlen(project_name) + 6, '-');
            printf("%s\n%s using %s\n%s\n", sep_line.c_str(), project_name, ver, sep_line.c_str());
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
            _MINE_THREAD_LOCAL_IF_HAVE std::unordered_map<const char*, std::string> func_name_map;
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

        inline const std::string& _getFmtI()
        {
            static std::string warning_message(R"([INFO][%s] )");
            return warning_message;
        }
        inline const std::string& _getFmtW()
        {
            static std::string warning_message(R"([WARNING][%s][%s: line %d] )");
            return warning_message;
        }
        inline const std::string& _getFmtE()
        {
            static std::string error_message(R"([ERROR][%s][%s: line %d] )");
            return error_message;
        }

        template<class... Ts>
        inline void _printfI(const char* fmt_chars, const char* funcname, Ts ...args)
        {
            printf((mbase::_getFmtI() + fmt_chars).c_str(), funcname, args...);
        }

        template<class... Ts>
        inline void _printfW(const char* fmt_chars, const char* funcname, const char* filename, int line, Ts ...args)
        {
            printf((mbase::_getFmtW() + fmt_chars).c_str(), funcname, filename, line, args...);
        }

        template<class... Ts>
        inline void _printfE(const char* fmt_chars, const char* funcname, const char* filename, int line, Ts ...args)
        {
            printf((mbase::_getFmtE() + fmt_chars).c_str(), funcname, filename, line, args...);
        }

        inline std::pair<unsigned int, unsigned int> _normRange(std::pair<unsigned int, unsigned int> range, unsigned int len)
        {
            if (range.first >= len)
                range.first = len;
            if (range.second >= len)
                range.second = len;
            return range;
        }

        class _MINE_EXPORT _ImmutableSpinLock
        {
        public:
            _ImmutableSpinLock() = default;

            void lock()
            {
                while (this->lock_flag_.test_and_set(std::memory_order_acquire)) {}
            }
            void unlock()
            {
                this->lock_flag_.clear(std::memory_order_release);
            }

            _ImmutableSpinLock(const _ImmutableSpinLock& tmp_lock) = delete;
            _ImmutableSpinLock& operator=(const _ImmutableSpinLock& tmp_lock) = delete;

        private:
            std::atomic_flag lock_flag_ = ATOMIC_FLAG_INIT;
        };
    }

}

#endif // !BASE_HPP_MINEUTILS