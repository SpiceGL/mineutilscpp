//mineutils库的类型相关工具
#pragma once
#ifndef TYPE_HPP_MINEUTILS
#define TYPE_HPP_MINEUTILS

#include<type_traits>
#include<tuple>

#include"base.hpp"


namespace mineutils
{
    namespace mtype
    {
        /*--------------------------------------------用户接口--------------------------------------------*/

        /*  用于判断类型是不是相同类型
            -父类和子类不是相同类型
            -比较类型时忽略const和&修饰，忽略同类型数组的长度不同
            -int和const int判断相同，但int[]和const int[]判断不同
            -涉及指针的时候忽略不了const   */
        template<class T1, class T2, class ...Ts>
        constexpr bool isSameType();

        //用于判断输入参数是不是相同类型
        template<class T1, class T2, class ...Ts>
        constexpr bool isSameType(T1& arg1, T2& arg2, Ts& ...args);


        //用于判断T是否属于后面的多种类型
        template<class T, class T1, class... Types>
        constexpr bool isInTypes();






        /*--------------------------------------------内部实现--------------------------------------------*/

        template<class T1, class T2>
        inline constexpr bool _isSameType(mbase::CaseTag0& tag)
        {
            return std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::value;
        }

        template<class T1, class T2, class ...Ts>
        inline constexpr bool _isSameType(mbase::CaseTag1& tag)
        {
            return std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::value and mtype::isSameType<T1, Ts...>();
        }

        /*  用于判断类型是不是相同类型
            -父类和子类不是相同类型
            -比较类型时忽略const和&修饰，忽略同类型数组的长度不同
            -int和const int判断相同，但int[]和const int[]判断不同
            -涉及指针的时候忽略不了const   */
        template<class T1, class T2, class ...Ts>
        inline constexpr bool isSameType()
        {
            return mtype::_isSameType<T1, T2, Ts...>(std::get<(sizeof...(Ts) > 0)>(mbase::BOOL_CASE_TAGS));
        }

        //用于判断输入参数是不是相同类型
        template<class T1, class T2, class ...Ts>
        inline constexpr bool isSameType(T1& arg1, T2& arg2, Ts& ...args)
        {
            return mtype::isSameType<T1, T2, Ts...>();
        }


        template<class T, class T1>
        inline constexpr bool _isInTypes(mbase::CaseTag0& tag)
        {
            return mtype::isSameType<T, T1>();
        }

        template<class T, class T1, class... Types>
        inline constexpr bool _isInTypes(mbase::CaseTag1& tag)
        {
            return mtype::isSameType<T, T1>() or isInTypes<T, Types...>();
        }

        //用于判断T是否属于后面的多种类型
        template<class T, class T1, class... Types>
        inline constexpr bool isInTypes()
        {
            return mtype::_isInTypes<T, T1, Types...>(std::get<(sizeof...(Types) > 0)>(mbase::BOOL_CASE_TAGS));
        }
    }
}

#endif // !TYPE_HPP_MINEUTILS