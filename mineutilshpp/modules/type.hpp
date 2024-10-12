//mineutils库的类型相关工具
#pragma once
#ifndef TYPE_HPP_MINEUTILS
#define TYPE_HPP_MINEUTILS

#include<functional>
#include<iostream>
#include<tuple>
#include<type_traits>


#include"base.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mtype
    {
        /*  检查Type1、Type2、Types...是否为相同类型，不忽略const、引用等符号
            用法：SameTypesChecker<Type1, Type2, Types...>::value, 类型为constexpr bool   */
        template<class Type1, class Type2, class ...Types>
        struct SameTypesChecker;

        /*  检查T是否为Type、Types...中的一个，不忽略const、引用等符号
            用法：InTypesChecker<T, Type, Types...>::value, 类型为constexpr bool   */
        template<class T, class Type, class ...Types>
        struct InTypesChecker;


        /*  检查T是否拥有类似于标准的STL容器的begin()和end()接口，可用于模板推导
            - T类型具有begin()和end()接口
            - T类型的begin()和end()接口返回的迭代器类型为std::remove_reference<U>::type::iterator或std::remove_reference<U>::type::const_iterator
            - T类型的begin()返回的迭代器可用被*解引用
            - T类型的begin()返回的迭代器支持++操作符，且前缀的++操作符返回迭代器的引用，后缀的++操作符返回迭代器的拷贝
            用法：StdBeginEndChecker<T>::value, 类型为constexpr bool   */
        template<class T>
        struct StdBeginEndChecker;


        /*  检查T是否支持std::cout <<，可用于模板推导
            - T类型正确重载了operator<<
            - 对于T类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            用法：StdCoutChecker<T>::value, 类型为constexpr bool   */
        template<class T>
        struct StdCoutChecker;


        /*  检查T、Ts中的每一个类型是否都支持std::cout <<，可用于模板推导
            - 每一个T、Ts类型的对象都可被std::cout <<接收
            - 对于每一个T、Ts类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            用法：StdCoutEachChecker<T, Ts...>::value, 类型为constexpr bool   */
        template<class T, class... Ts>
        struct StdCoutEachChecker;


        /*  检查DstT是否可以由T、Ts...类型对象中的每一个单独构造，可用于模板推导
            - 检查规则参照std::is_constructible
            - T、Ts...中的非引用类型，会被作为右值进行检查
            用法：ConstructibleFromEachChecker<DstT, T, Ts...>::value，类型为constexpr bool   */
        template<class DstT, class T, class... Ts>
        struct ConstructibleFromEachChecker;


        /*  对Fn进行检查，获得Fn是否为有效函数以及返回值、参数数量等信息，可用于模板推导
            - Fn接收重载函数时，需要显式指定函数类型，如FuncChecker<decltype((void(*)(int&))func1)>
            用法： 
            - FuncChecker<Fn>::value，判断Fn是否为有效函数，类型为constexpr bool
            - typename FuncChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn不是有效的函数类型时ReturnType不存在
            - FuncChecker<Fn>::num_args，类型为constexpr size_t，获得Fn的参数数量，Fn不是有效的函数类型时num_args不存在   */
        template <class Fn>
        struct FuncChecker;


        /*  检查Fn和Args...参数是否能使用std::bind绑定，可用于模板推导
           - Fn接收重载函数时，需要显式指定函数类型，如StdBindChecker<decltype((void(*)(int&))func1), Args...>
           - 参数要求与std::bind相同
           用法：
           - StdBindChecker<Fn>::value，判断Fn是否为有效函数类型，类型为constexpr bool
           - typename StdBindChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn和Args...不匹配时ReturnType不存在   */
        template<class Fn, class... Args>
        struct StdBindChecker;
    }








    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mtype
    {
        /*  检查Type1、Type2、Types...是否为相同类型
           用法：SameTypesChecker<Type1, Type2, Types...>::value, 类型为constexpr bool   */
        template<class Type1, class Type2, class ...Types>
        struct SameTypesChecker
        {
        private:
            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value&& SameTypesChecker<U1, Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class ...>
            static std::false_type check(...);

            SameTypesChecker() = delete;

        public:
            static constexpr bool value = decltype(check<Type1, Type2, Types...>(0))::value;
        };
        template<class Type1, class Type2, class ...Types>
        constexpr bool SameTypesChecker<Type1, Type2, Types...>::value;


        /*  检查T是否为Type、Types...中的一个
           用法：InTypesChecker<T, Type, Types...>::value, 类型为constexpr bool   */
        template<class T, class Type, class ...Types>
        struct InTypesChecker
        {
        private:
            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value || InTypesChecker<U1, Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class ...>
            static std::false_type check(...);

            InTypesChecker() = delete;

        public:
            static constexpr bool value = decltype(check<T, Type, Types...>(0))::value;
        };
        template<class T, class Type, class ...Types>
        constexpr bool InTypesChecker<T, Type, Types...>::value;


        /*  检查T是否拥有类似于标准的STL容器的begin()和end()接口，可用于模板推导
            - T类型具有begin()和end()接口
            - T类型的begin()和end()接口返回的迭代器类型为std::remove_reference<U>::type::iterator或std::remove_reference<U>::type::const_iterator
            - T类型的begin()返回的迭代器可用被*解引用
            - T类型的begin()返回的迭代器支持++操作符，且前缀的++操作符返回迭代器的引用，后缀的++操作符返回迭代器的拷贝
            用法：StdBeginEndChecker<T>::value, 类型为constexpr bool   */
        template<class T>
        struct StdBeginEndChecker
        {
        private:
            //之所以用U而不是T，是因为为T的话，模板类型就被类的模板类型确定了，函数调用不再具有SFINAE特性
            template<class U, typename std::enable_if<!std::is_const<U>::value && std::is_same<decltype(std::declval<U>().begin()), typename std::remove_reference<U>::type::iterator>::value && std::is_same<decltype(std::declval<U>().end()), typename std::remove_reference<U>::type::iterator>::value, int>::type = 0>
            static std::true_type checkExist(int);

            template<class U, typename std::enable_if<std::is_const<U>::value && std::is_same<decltype(std::declval<U>().begin()), typename std::remove_reference<U>::type::const_iterator>::value && std::is_same<decltype(std::declval<U>().end()), typename std::remove_reference<U>::type::const_iterator>::value, int>::type = 0>
            static std::true_type checkExist(int);

            template<class...>
            static std::false_type checkExist(...);

            template<class U, typename = decltype(*(std::declval<typename std::remove_reference<U>::type::iterator>())) >
            static std::true_type checkDeref(int);

            template<class...>
            static std::false_type checkDeref(...);

            template<class U, typename std::enable_if<!std::is_const<U>::value && std::is_same<decltype(++std::declval<U>().begin()), typename std::remove_reference<U>::type::iterator&>::value && std::is_same<decltype(std::declval<U>().begin()++), typename std::remove_reference<U>::type::iterator>::value, int>::type = 0>
            static std::true_type checkIncrementOperator(int);

            template<class U, typename std::enable_if<std::is_const<U>::value && std::is_same<decltype(++std::declval<U>().begin()), typename std::remove_reference<U>::type::const_iterator&>::value && std::is_same<decltype(std::declval<U>().begin()++), typename std::remove_reference<U>::type::const_iterator>::value, int>::type = 0>
            static std::true_type checkIncrementOperator(int);

            template<class...>
            static std::false_type checkIncrementOperator(...);

            StdBeginEndChecker() = delete;

        public:
            static constexpr bool value = decltype(checkExist<T>(0))::value && decltype(checkDeref<T>(0))::value && decltype(checkIncrementOperator<T>(0))::value;
        };
        template<class T>
        constexpr bool StdBeginEndChecker<T>::value;


        /*  检查T是否支持std::cout <<，可用于模板推导
            - T类型正确重载了operator<<
            - 对于T类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            用法：StdCoutChecker<T>::value, 类型为constexpr bool   */
        template<class T>
        struct StdCoutChecker
        {
        private:
            //之所以用U而不是T，是因为为T的话，模板类型就被类的模板类型确定了，函数调用不再具有SFINAE特性
            template<class U, typename std::enable_if<std::is_same<decltype(std::cout << std::declval<U>()), std::ostream&>::value, int>::type = 0>
            static std::true_type check(int);

            template<typename...>
            static std::false_type check(...);

            StdCoutChecker() = delete;

        public:
            static constexpr bool value = decltype(check<T>(0))::value;
        };
        template<class T>
        constexpr bool StdCoutChecker<T>::value;


        /*  检查T、Ts中的每一个类型是否都支持std::cout <<，可用于模板推导
            - 每一个T、Ts类型的对象都可被std::cout <<接收
            - 对于每一个T、Ts类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            用法：StdCoutEachChecker<T, Ts...>::value, 类型为constexpr bool   */
        template<class T, class... Ts>
        struct StdCoutEachChecker
        {
        private:
            template<class U, class... Us, typename std::enable_if<StdCoutChecker<U>::value && StdCoutEachChecker<Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U, class... Us, typename std::enable_if<StdCoutChecker<U>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class...>
            static std::false_type check(...);

            StdCoutEachChecker() = delete;

        public:
            static constexpr bool value = decltype(check<T, Ts...>(0))::value;
        };
        template<class T, class... Ts>
        constexpr bool StdCoutEachChecker<T, Ts...>::value;



        /*  检查DstT是否可以由T、Ts...类型对象中的每一个单独构造，可用于模板推导
            - 检查规则参照std::is_constructible
            - T、Ts...中的非引用类型，会被作为右值进行检查
            - 构造过程中的隐式转换也是被支持的
            用法：ConstructibleFromEachChecker<DstT, T, Ts...>::value，类型为constexpr bool   */
        template<class DstT, class T, class... Ts>
        struct ConstructibleFromEachChecker
        {
        private:
            template<class DstU, class U, class... Us, typename std::enable_if<std::is_constructible<DstU, U>::value && ConstructibleFromEachChecker<DstU, Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class DstU, class U, class... Us, typename std::enable_if<std::is_constructible<DstU, U>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class...>
            static std::false_type check(...);

            ConstructibleFromEachChecker() = delete;

        public:
            static constexpr bool value = decltype(check<DstT, T, Ts...>(0))::value;
        };
        template<class DstT, class T, class... Ts>
        constexpr bool ConstructibleFromEachChecker<DstT, T, Ts...>::value;


        template <class Fn>
        struct _FunctionCheckerBase
        {
        public:
            template <class Ret, class... Arguments>
            static std::true_type checkValidity(Ret(*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkValidity(Ret(ObjType::*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkValidity(Ret(ObjType::*)(Arguments...) const);

            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkValidity(Ret(ObjType::*)(Arguments...) volatile);

            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkValidity(Ret(ObjType::*)(Arguments...) const volatile);

            template <class Functor, typename std::enable_if<decltype(checkValidity(&Functor::operator()))::value, int>::type = 0 >
            static std::true_type checkValidity(Functor);

            template<class...>
            static std::false_type checkValidity(...);

            template <class Ret, class... Arguments>
            static Ret checkRet(Ret(*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static Ret checkRet(Ret(ObjType::*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static Ret checkRet(Ret(ObjType::*)(Arguments...) const);

            template <class Ret, class ObjType, class... Arguments>
            static Ret checkRet(Ret(ObjType::*)(Arguments...) volatile);

            template <class Ret, class ObjType, class... Arguments>
            static Ret checkRet(Ret(ObjType::*)(Arguments...) const volatile);

            template <class Functor, class Ret = decltype(checkRet(&Functor::operator()))>
            static Ret checkRet(Functor);

            template <class Ret, class... Arguments, size_t N = sizeof...(Arguments)>
            static std::integral_constant<size_t, N> checkNumArgs(Ret(*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments, size_t N = sizeof...(Arguments)>
            static std::integral_constant<size_t, N> checkNumArgs(Ret(ObjType::*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments, size_t N = sizeof...(Arguments)>
            static std::integral_constant<size_t, N> checkNumArgs(Ret(ObjType::*)(Arguments...) const);

            template <class Ret, class ObjType, class... Arguments, size_t N = sizeof...(Arguments)>
            static std::integral_constant<size_t, N> checkNumArgs(Ret(ObjType::*)(Arguments...) volatile);

            template <class Ret, class ObjType, class... Arguments, size_t N = sizeof...(Arguments)>
            static std::integral_constant<size_t, N> checkNumArgs(Ret(ObjType::*)(Arguments...) const volatile);

            template <class Functor, size_t N = decltype(checkNumArgs(&Functor::operator()))::value>
            static std::integral_constant<size_t, N> checkNumArgs(Functor);

            _FunctionCheckerBase() = delete;
        };


        template <bool Checker, class Fn>
        struct _FuncCheckerHelper
        {
        private:
            _FuncCheckerHelper() = delete;
        public:
            static constexpr bool value = decltype(_FunctionCheckerBase<Fn>::checkValidity(std::declval<Fn>()))::value;
        };
        template <bool Checker, class Fn>
        constexpr bool _FuncCheckerHelper<Checker, Fn>::value;


        template <class Fn>
        struct _FuncCheckerHelper<true, Fn>
        {
        private:
            _FuncCheckerHelper() = delete;
        public:
            static constexpr bool value = decltype(_FunctionCheckerBase<Fn>::checkValidity(std::declval<Fn>()))::value;
            using ReturnType = decltype(_FunctionCheckerBase<Fn>::checkRet(std::declval<Fn>()));
            static constexpr size_t num_args = decltype(_FunctionCheckerBase<Fn>::checkNumArgs(std::declval<Fn>()))::value;
        };
        template <class Fn>
        constexpr bool _FuncCheckerHelper<true, Fn>::value;
        template <class Fn>
        constexpr size_t _FuncCheckerHelper<true, Fn>::num_args;


        /*  对Fn进行检查，获得Fn是否为有效函数以及返回值、参数数量等信息，可用于模板推导
            - Fn接收重载函数时，需要显式指定函数类型，如FuncChecker<decltype((void(*)(int&))func1)>
            用法：
            - FuncChecker<Fn>::value，判断Fn是否为有效函数，类型为constexpr bool
            - typename FuncChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn不是有效的函数类型时ReturnType不存在
            - FuncChecker<Fn>::num_args，类型为constexpr size_t，获得Fn的参数数量，Fn不是有效的函数类型时num_args不存在   */
        template <class Fn>
        struct FuncChecker : public _FuncCheckerHelper<decltype(_FunctionCheckerBase<Fn>::checkValidity(std::declval<Fn>()))::value, Fn>
        {

        };

        template<class Fn, class... Args>
        struct _StdBindCheckerBase
        {
        public:
            template<class Func, class... Arguments, typename std::enable_if<!std::is_member_pointer<Func>::value && (FuncChecker<Func>::num_args == sizeof...(Arguments)), int>::type = 0>
            static auto checkRet(int) -> decltype(std::bind(std::forward<Func>(std::declval<Func>()), std::forward<Arguments>(std::declval<Arguments>())...)());

            template<class Func, class... Arguments, typename std::enable_if<std::is_member_pointer<Func>::value && (FuncChecker<Func>::num_args + 1 == sizeof...(Arguments)), int>::type = 0>
            static auto checkRet(int) -> decltype(std::bind(std::forward<Func>(std::declval<Func>()), std::forward<Arguments>(std::declval<Arguments>())...)());

            template<class Func, class... Arguments, typename std::enable_if<!std::is_member_pointer<Func>::value && (FuncChecker<Func>::num_args == sizeof...(Arguments)), int>::type = 0>
            static auto checkValue(int) -> decltype(std::bind(std::forward<Func>(std::declval<Func>()), std::forward<Arguments>(std::declval<Arguments>())...)(), std::true_type());

            template<class Func, class... Arguments, typename std::enable_if<std::is_member_pointer<Func>::value && (FuncChecker<Func>::num_args + 1 == sizeof...(Arguments)), int>::type = 0>
            static auto checkValue(int) -> decltype(std::bind(std::forward<Func>(std::declval<Func>()), std::forward<Arguments>(std::declval<Arguments>())...)(), std::true_type());

            template<class...>
            static std::false_type checkValue(...);
        };


        template <bool Checker, class Fn, class... Args>
        struct _StdBindCheckerHelper
        {
        private:
            _StdBindCheckerHelper() = delete;
        public:
            static constexpr bool value = decltype(_StdBindCheckerBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value;
        };
        template <bool Checker, class Fn, class... Args>
        constexpr bool _StdBindCheckerHelper<Checker, Fn, Args...>::value;


        template <class Fn, class... Args>
        struct _StdBindCheckerHelper<true, Fn, Args...>
        {
        private:
            _StdBindCheckerHelper() = delete;
        public:
            static constexpr bool value = decltype(_StdBindCheckerBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value;
            using ReturnType = decltype(_StdBindCheckerBase<Fn, Args...>::template checkRet<Fn, Args...>(0));
        };
        template <class Fn, class... Args>
        constexpr bool _StdBindCheckerHelper<true, Fn, Args...>::value;


        /*  检查Fn和Args...参数是否能使用std::bind绑定，可用于模板推导
            - Fn接收重载函数时，需要显式指定函数类型，如StdBindChecker<decltype((void(*)(int&))func1), Args...>
            - 参数要求与std::bind相同
            用法：
            - StdBindChecker<Fn>::value，判断Fn是否为有效函数类型，类型为constexpr bool
            - typename StdBindChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn和Args...不匹配时ReturnType不存在   */
        template<class Fn, class... Args>
        struct StdBindChecker : public _StdBindCheckerHelper<decltype(_StdBindCheckerBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value, Fn, Args...>
        {

        };



        /*  用于判断类型是不是相同类型
            -忽略引用&修饰
            -比较非指针类型时忽略const修饰
            -比较指针或C风格数组类型时忽略作用于对该指针的const修饰   */
        template<class T1, class T2, class ...Types>
        MINE_DEPRECATED(R"(Deprecated. Please replace with struct "SameTypesChecker"(in type.hpp))") constexpr bool isSameType();


        //用于判断T是否属于后面的多种类型
        template<class T, class Tother, class... Tothers>
        MINE_DEPRECATED(R"(Deprecated. Please replace with struct "SameTypesChecker"(in type.hpp))") constexpr bool isInTypes();

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
        inline constexpr bool isSameType(T1& arg1, T2& arg2, Types & ...args)
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
        inline MINE_DEPRECATED(R"(Deprecated. Please replace with struct "InTypesChecker"(in type.hpp))") constexpr bool isInTypes()
        {
            return mtype::_isInTypes<T, Tother, Tothers...>(std::integral_constant<bool, (sizeof...(Tothers) > 0)>());
        }
    }

#ifdef MINEUTILS_TEST_MODULES
#include<vector>
    namespace _mtypecheck
    {
        inline void SameTypesCheckerTest()
        {
            bool ret0 = 0;
            ret0 = mtype::SameTypesChecker<int, unsigned int>::value;
            printf("%s SameTypesChecker<int, unsigned int>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::SameTypesChecker<int, unsigned int, int, int>::value;
            printf("%s SameTypesChecker<int, unsigned int, int, int>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::SameTypesChecker<int, int, int, unsigned int>::value;
            printf("%s SameTypesChecker<int, int, int, unsigned int>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::SameTypesChecker<int, int>::value;
            printf("%s SameTypesChecker<int, int>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::SameTypesChecker<int, int, int, int>::value;
            printf("%s SameTypesChecker<int, int, int, int>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);
        }

        inline void InTypesCheckerTest()
        {
            bool ret0 = 0;
            ret0 = mtype::InTypesChecker<int, unsigned int>::value;
            printf("%s InTypesChecker<int, unsigned int>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::InTypesChecker<int, unsigned int, float, char>::value;
            printf("%s InTypesChecker<int, unsigned int, float, char>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::InTypesChecker<int, int>::value;
            printf("%s InTypesChecker<int, int>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::InTypesChecker<int, unsigned int, int, float>::value;
            printf("%s InTypesChecker<int, unsigned int, int, float>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);
        }

        inline void StdBeginEndCheckerTest()
        {
            bool ret0 = 0;
            ret0 = mtype::StdBeginEndChecker<int>::value;
            printf("%s StdBeginEndChecker<int>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBeginEndChecker<std::vector<int>>::value;
            printf("%s StdBeginEndChecker<std::vector<int>>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBeginEndChecker<const std::vector<int>>::value;
            printf("%s StdBeginEndChecker<const std::vector<int>>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);
        }

        inline void StdCoutCheckerTest()
        {
            bool ret0 = 0;
            ret0 = mtype::StdCoutChecker<int>::value;
            printf("%s StdCoutChecker<int>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdCoutChecker<std::vector<int>>::value;
            printf("%s StdCoutChecker<std::vector<int>>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdCoutEachChecker<int>::value;
            printf("%s StdCoutEachChecker<int>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdCoutEachChecker<std::vector<int>>::value;
            printf("%s StdCoutEachChecker<std::vector<int>>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdCoutEachChecker<int, float, char>::value;
            printf("%s StdCoutEachChecker<int, float, char>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdCoutEachChecker<int, std::vector<int>, char>::value;
            printf("%s StdCoutEachChecker<int, std::vector<int>, char>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);
        }

        inline void ConstructibleFromEachCheckerTest()
        {
            bool ret0 = 0;
            ret0 = mtype::ConstructibleFromEachChecker<void, char>::value;
            printf("%s ConstructibleFromEachChecker<void, char>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::ConstructibleFromEachChecker<std::string&, const char*>::value;
            printf("%s ConstructibleFromEachChecker<std::string&, const char*>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::ConstructibleFromEachChecker<std::vector<int>, int>::value;
            printf("%s ConstructibleFromEachChecker<std::vector<int>, int>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<float>>::value;
            printf("%s ConstructibleFromEachChecker<std::vector<int>, std::vector<float>>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::ConstructibleFromEachChecker<int, char, float>::value;
            printf("%s ConstructibleFromEachChecker<int, char, float>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, int, float>::value;
            printf("%s ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, int, float>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, std::vector<float>, float>::value;
            printf("%s ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, std::vector<float>, floatt>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);
        }

        inline int _myFunc1(int a, int b)
        {
            return a + b;
        }

        class _MyClass1
        {
        public:
            int func1(int a)
            {
                return a + 1;
            }

            int func2(int a) const
            {
                return a + 1;
            }

            int func3(int a) const volatile
            {
                return a + 1;
            }

            static int staticFunc1(int a)
            {
                return a + 1;
            }
        };

        inline void FuncCheckerTest()
        {
            bool ret0 = 0;
            int num_args = 0;
            ret0 = mtype::FuncChecker<int>::value;
            printf("%s FuncChecker<decltype(_myFunc1)>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::FuncChecker<decltype(_myFunc1)>::value;
            using type0 = mtype::FuncChecker<decltype(_myFunc1)>::ReturnType;
            num_args = mtype::FuncChecker<decltype(_myFunc1)>::num_args;
            printf("%s FuncChecker<decltype(_myFunc1)>::value:%d\n", ret0 == true ? "Passed." : "Failed!", ret0);
            printf("User Check! FuncChecker<decltype(_myFunc1)>::RetType(int):%s, num_args(2):%d.\n", mtypename(type0), num_args);

            ret0 = mtype::FuncChecker<decltype(&_MyClass1::func1)>::value;
            using type1 = mtype::FuncChecker<decltype(&_MyClass1::func1)>::ReturnType;
            num_args = mtype::FuncChecker<decltype(&_MyClass1::func1)>::num_args;
            printf("%s FuncChecker<decltype(&_MyClass1::func1)>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);
            printf("User Check! FuncChecker<decltype(&_MyClass1::func1)>::ReturnType(int):%s, num_args(1):%d.\n", mtypename(type1), num_args);

            ret0 = mtype::FuncChecker<decltype(&_MyClass1::func3)>::value;
            using type1 = mtype::FuncChecker<decltype(&_MyClass1::func3)>::ReturnType;
            num_args = mtype::FuncChecker<decltype(&_MyClass1::func3)>::num_args;
            printf("%s FuncChecker<decltype(&_MyClass1::func3)>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);
            printf("User Check! FuncChecker<decltype(&_MyClass1::func3)>::ReturnType(int):%s, num_args(1):%d.\n", mtypename(type1), num_args);

            auto lambda1 = [](int a, int b) {return a + b; };
            ret0 = mtype::FuncChecker<decltype(lambda1)>::value;
            using type2 = mtype::FuncChecker<decltype(lambda1)>::ReturnType;
            num_args = mtype::FuncChecker<decltype(lambda1)>::num_args;
            printf("%s FuncChecker<decltype(lambda1)>::value:%d.\n", ret0 == true ? "Passed." : "Failed!", ret0);
            printf("User Check! FuncChecker<decltype(&_MyClass1::func1)>::ReturnType(int):%s, num_args(2):%d.\n", mtypename(type2), num_args);
        }

        inline void StdBindCheckerTest()
        {
            bool ret0 = 0;
            ret0 = mtype::StdBindChecker<int, int>::value;
            printf("%s StdBindChecker<int, int>::value:%d.\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(_myFunc1), int, float>::value;
            using type0 = mtype::StdBindChecker<decltype(_myFunc1), int, float>::ReturnType;
            printf("%s StdBindChecker<decltype(_myFunc1), int, float>::value:%d\n", ret0 == true ? "Passed." : "Failed!", ret0);
            printf("User Check! StdBindChecker<decltype(_myFunc1), int, float>::ReturnType(int):%s.\n", mtypename(type0));

            ret0 = mtype::StdBindChecker<decltype(_myFunc1), int>::value;
            printf("%s StdBindChecker<decltype(_myFunc1), int>::value:%d\n", ret0 == false ? "Passed." : "Failed!", ret0);
   
            ret0 = mtype::StdBindChecker<decltype(_myFunc1), int, int, int>::value;
            printf("%s StdBindChecker<decltype(_myFunc1), int, int, int>::value:%d\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::func1), int>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::func1), int>::value:%d\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::func1), std::vector<int>>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::func1), std::vector<int>>::value:%d\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::func1), const _MyClass1*, int>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::func1), const _MyClass1*, int>::value:%d\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::func2), const _MyClass1*, int>::value;
            using type1 = mtype::StdBindChecker<decltype(&_MyClass1::func2), const _MyClass1*, int>::ReturnType;
            printf("%s StdBindChecker<decltype(&_MyClass1::func2), const _MyClass1*, int>::value:%d\n", ret0 == true ? "Passed." : "Failed!", ret0);
            printf("User Check! StdBindChecker<decltype(&_MyClass1::func2), const _MyClass1*, int>::ReturnType(int):%s.\n", mtypename(type1));

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::func2), const _MyClass1&, int>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::func2), const _MyClass1&, int>::value:%d\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::func3), const volatile _MyClass1&, int>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::func3), const volatile _MyClass1&, int>::value:%d\n", ret0 == true ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::staticFunc1), _MyClass1*, int>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::staticFunc1), _MyClass1*, int>::value:%d\n", ret0 == false ? "Passed." : "Failed!", ret0);

            ret0 = mtype::StdBindChecker<decltype(&_MyClass1::staticFunc1), int>::value;
            printf("%s StdBindChecker<decltype(&_MyClass1::staticFunc1), int>::value:%d\n", ret0 == true ? "Passed." : "Failed!", ret0);
            _MyClass1 obj1;
        }

        inline void check()
        {
            printf("\n--------------------check mtype start--------------------\n");
            SameTypesCheckerTest();
            printf("\n");
            InTypesCheckerTest();
            printf("\n");
            StdBeginEndCheckerTest();
            printf("\n");
            StdCoutCheckerTest();
            printf("\n");
            ConstructibleFromEachCheckerTest();
            printf("\n");
            FuncCheckerTest();
            printf("\n");
            StdBindCheckerTest();
            printf("--------------------check mtype end--------------------\n\n");
        }
    }
#endif
}

#endif // !TYPE_HPP_MINEUTILS