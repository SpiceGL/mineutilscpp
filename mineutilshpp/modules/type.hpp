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
            -忽略引用&修饰
            -比较非指针类型时忽略const修饰
            -比较指针或C风格数组类型时无法忽略const修饰   */
        template<class T1, class T2, class ...Ts>
        constexpr bool isSameType();


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


        template<class T1, class T2, class ...Ts>
        inline constexpr bool isSameType()
        {
            return mtype::_isSameType<T1, T2, Ts...>(std::get<(sizeof...(Ts) > 0)>(mbase::BOOL_CASE_TAGS));
        }

        //用于判断输入参数是不是相同类型
        template<class T1, class T2, class ...Ts>
        inline MINE_DEPRECATED("Deprecated. Please replace with the other fuction \"isSameType\"(in type.hpp).") constexpr bool isSameType(T1& arg1, T2& arg2, Ts & ...args)
        {
            return mtype::isSameType<T1, T2, Ts...>();
        }

        //template<class T1, class T2, class ...Ts>
        //constexpr bool isSameType(T1* const arg1, T2* const arg2, Ts* const ...args)
        //{
        //    return mtype::_isSameType<T1, T2, Ts...>(std::get<(sizeof...(Ts) > 0)>(mbase::BOOL_CASE_TAGS));
        //}


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