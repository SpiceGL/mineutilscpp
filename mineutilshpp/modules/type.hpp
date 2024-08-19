//mineutils库的类型相关工具
#pragma once
#ifndef TYPE_HPP_MINEUTILS
#define TYPE_HPP_MINEUTILS

#include<iostream>
#include<type_traits>
#include<tuple>

#include"base.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mtype
    {
        /*  用于判断类型是不是相同类型
            -忽略引用&修饰
            -比较非指针类型时忽略const修饰
            -比较指针或C风格数组类型时忽略作用于对该指针的const修饰   */
        template<class T1, class T2, class ...Types>
        constexpr bool isSameType();


        //用于判断T是否属于后面的多种类型
        template<class T, class Tother, class... Tothers>
        constexpr bool isInTypes();


        /*  检查T是否拥有类似于标准的STL容器的begin()和end()接口，可用于模板推导
            - T具有begin()和end()接口
            - T的begin()和end()接口返回的迭代器类型为T::iterator或T::const_iterator
            - T的begin()返回的迭代器可用被*解引用
            - T的begin()返回的迭代器支持++操作符，且前缀的++操作符返回迭代器的引用，后缀的++操作符返回迭代器的拷贝
            用法：StdContainerLikeChecker<T>::value   */
        template<typename T>
        struct StdBeginEndChecker
        {
        private:
            //之所以用U而不是T，是因为为T的话，模板类型就被类的模板类型确定了，函数调用不再具有SFINAE特性
            template<typename U, typename std::enable_if<!std::is_const<U>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(std::declval<U>().begin()), typename U::iterator>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(std::declval<U>().end()), typename U::iterator>::value, int>::type = 0>
            static std::true_type checkExist(int);

            template<typename U, typename std::enable_if<std::is_const<U>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(std::declval<U>().begin()), typename U::const_iterator>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(std::declval<U>().end()), typename U::const_iterator>::value, int>::type = 0>
            static std::true_type checkExist(int);

            template<typename...>
            static std::false_type checkExist(...);

            template<typename U, typename = decltype(*(std::declval<typename U::iterator>())) >
            static std::true_type checkDeref(int);

            template<typename...>
            static std::false_type checkDeref(...);

            template<typename U, typename std::enable_if<!std::is_const<U>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(++std::declval<U>().begin()), typename U::iterator&>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(std::declval<U>().begin()++), typename U::iterator>::value, int>::type = 0>
            static std::true_type checkIncrementOperator(int);

            template<typename U, typename std::enable_if<std::is_const<U>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(++std::declval<U>().begin()), typename U::const_iterator&>::value, int>::type = 0, typename std::enable_if<std::is_same<decltype(std::declval<U>().begin()++), typename U::const_iterator>::value, int>::type = 0>
            static std::true_type checkIncrementOperator(int);

            template<typename...>
            static std::false_type checkIncrementOperator(...);

            StdBeginEndChecker() = delete;

            using DecayT = typename std::remove_reference<T>::type;

        public:
            static constexpr bool value = decltype(checkExist<DecayT>(0))::value && decltype(checkDeref<DecayT>(0))::value && decltype(checkIncrementOperator<DecayT>(0))::value;
        };


        /*  检查T是否支持std::cout <<，可用于模板推导
            - T可被std::cout <<接收
            - std::cout << T对象的返回值类型为std::ostream&
            用法：StdCoutChecker<T>::value   */
        template<typename T>
        struct StdCoutChecker
        {
        private:
            //之所以用U而不是T，是因为为T的话，模板类型就被类的模板类型确定了，函数调用不再具有SFINAE特性
            template<typename U, typename std::enable_if<std::is_same<decltype(std::cout << std::declval<U>()), std::ostream&>::value, int>::type = 0>
            static std::true_type check(int);

            template<typename...>
            static std::false_type check(...);

            StdCoutChecker() = delete;

            using DecayT = typename std::remove_reference<T>::type;

        public:
            static constexpr bool value = decltype(check<DecayT>(0))::value;
        };

    }






    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mtype
    {
        template<class T>
        constexpr bool StdBeginEndChecker<T>::value;

        template<class T>
        constexpr bool StdCoutChecker<T>::value;


        template<class T1, class T2>
        inline constexpr bool _isSameType(std::false_type bool_tag)
        {
            return std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::value;
        }

        template<class T1, class T2, class ...Types>
        inline constexpr bool _isSameType(std::true_type bool_tag)
        {
            return std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::value and mtype::isSameType<T1, Types...>();
        }


        template<class T1, class T2, class ...Types>
        inline constexpr bool isSameType()
        {
            return mtype::_isSameType<T1, T2, Types...>(std::integral_constant<bool, (sizeof...(Types) > 0)>());
        }

        //用于判断输入参数是不是相同类型
        template<class T1, class T2, class ...Types>
        inline MINE_DEPRECATED("Deprecated. Please replace with the other fuction \"isSameType\"(in type.hpp)") constexpr bool isSameType(T1& arg1, T2& arg2, Types & ...args)
        {
            return mtype::isSameType<T1, T2, Types...>();
        }


        template<class T, class Tother>
        inline constexpr bool _isInTypes(std::false_type bool_tag)
        {
            return mtype::isSameType<T, Tother>();
        }

        template<class T, class Tother, class... Tothers>
        inline constexpr bool _isInTypes(std::true_type bool_tag)
        {
            return mtype::isSameType<T, Tother>() or isInTypes<T, Tothers...>();
        }

        //用于判断T是否属于后面的多种类型
        template<class T, class Tother, class... Tothers>
        inline constexpr bool isInTypes()
        {
            return mtype::_isInTypes<T, Tother, Tothers...>(std::integral_constant<bool, (sizeof...(Tothers) > 0)>());
        }

    }
}

#endif // !TYPE_HPP_MINEUTILS