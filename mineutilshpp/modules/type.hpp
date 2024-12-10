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
        //获取类型的名字字符串
        template<class T>
        const char* getTypeName();


        /*  将T类型转换为自身的右值引用类型，用于区别于std::add_rvalue_reference
            用法：RvalueRefMaker<T>::Type  */
        template<class T>
        struct RvalueRefMaker;


        /*  检查Type1、Type2、Types...是否为相同类型，不忽略const、引用等符号
            用法：SameTypesChecker<Type1, Type2, Types...>::value, 类型为constexpr bool   */
        template<class Type1, class Type2, class ...Types>
        struct SameTypesChecker;


        /*  检查T是否为Type、Types...中的一个，不忽略const、引用等符号
            用法：InTypesChecker<T, Type, Types...>::value, 类型为constexpr bool   */
        template<class T, class Type, class ...Types>
        struct InTypesChecker;


        /*  检查T是否拥有类似于标准的STL容器的begin()和end()接口，支持模板SFINAE特性
            - T类型具有begin()和end()接口
            - T类型的begin()和end()接口返回的迭代器类型为std::remove_reference<U>::type::iterator或std::remove_reference<U>::type::const_iterator
            - T类型的begin()返回的迭代器可用被*解引用
            - T类型的begin()返回的迭代器支持++操作符，且前缀的++操作符返回迭代器的引用，后缀的++操作符返回迭代器的拷贝
            用法：StdBeginEndChecker<T>::value, 类型为constexpr bool   */
        template<class T>
        struct StdBeginEndChecker;


        /*  检查T是否支持std::cout <<，支持模板SFINAE特性
            - T类型正确重载了operator<<(std::ostream&, const T&)
            - 对于T类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            - 对T类型查找operator<<的范围为std命名空间、T所在命名空间及其关联命名空间，以及全局命名空间
            用法：StdCoutChecker<T>::value, 类型为constexpr bool   */
        template<class T>
        struct StdCoutChecker;


        /*  检查T、Ts中的每一个类型是否都支持std::cout <<，支持模板SFINAE特性
            - 每一个T、Ts类型的对象都正确重载了operator<<(std::ostream&, const T&)
            - 对于每一个T、Ts类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            - 对每一个T、Ts类型查找operator<<的范围为std命名空间、当前T所在命名空间及其关联命名空间，以及全局命名空间
            用法：StdCoutEachChecker<T, Ts...>::value, 类型为constexpr bool   */
        template<class T, class... Ts>
        struct StdCoutEachChecker;


        /*  检查DstT是否可以由T、Ts...类型对象中的每一个单独构造或引用绑定，支持模板SFINAE特性
            - 检查规则参照std::is_constructible<DstT, T>
            - T、Ts...中的非引用类型，会被作为右值类型进行检查
            - DstT作为非引用类型时，检查DstT是否可以由T、Ts...中每一个的对象构造
            - DstT作为引用类型时，检查DstT是否可以绑定到每一个T、Ts...对象上
            - 构造过程中的隐式转换也是被支持的
            用法：ConstructibleFromEachChecker<DstT, T, Ts...>::value，类型为constexpr bool   */
        template<class DstT, class T, class... Ts>
        struct ConstructibleFromEachChecker;


        /*  检查T、Ts...类型中的每一个是否都支持使用自身的左值进行构造或引用绑定，支持模板SFINAE特性
            - 对于T、Ts...中的非引用类型，检查其能否由自身的左值对象构造
            - 对于T、Ts...中的引用类型，检查其能否绑定到自身的左值对象上
            用法：EachLvalueConstructibleChecker<T, Ts...>::value，类型为constexpr bool   */
        template<class T, class... Ts>
        struct EachLvalueConstructibleChecker;


        /*  检查T、Ts...类型中的每一个是否都支持使用自身的右值进行构造或引用绑定，支持模板SFINAE特性
            - 对于T、Ts...中的非引用类型，检查其能否由自身的右值对象构造
            - 对于T、Ts...中的引用类型，检查其能否绑定到自身的右值对象上
            用法：EachRvalueConstructibleChecker<T, Ts...>::value，类型为constexpr bool   */
        template<class T, class... Ts>
        struct EachRvalueConstructibleChecker;


        /*  对Fn进行检查，判断Fn是否为可解析的函数、仿函数或成员函数，并获取返回值、参数数量等信息，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针和仿函数类型
            - Fn接收有多个重载的函数和成员函数时，需要显式指定函数类型，如FuncChecker<decltype((void(*)(int&))func1)>
            - Fn是仿函数时，如果仿函数重载了多个operator()，那么FuncChecker<Fn>::value为false
            - Fn是仿函数时，对operator()的检查不会考虑其const、volatile等限定
            用法：
            - FuncChecker<Fn>::value，判断Fn是否为有效函数，类型为constexpr bool
            - FuncChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn不是有效的函数类型时ReturnType不存在
            - FuncChecker<Fn>::ArgsTupleType，获取Fn的参数列表类型组成的std::tuple类型，Fn不是有效的函数类型时ArgsTupleType不存在
            - FuncChecker<Fn>::num_args，类型为constexpr size_t，获得Fn的参数数量，Fn不是有效的函数类型时num_args不存在   */
        template <class Fn>
        struct FuncChecker;


        /*  检查Fn和Args...参数是否能使用std::bind绑定，且具有更严格的限制，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针、仿函数类型以及它们的reference_warpper包装
            - Fn接收重载函数时，需要显式指定函数类型，如StdBindChecker<decltype((void(*)(int&))func1), int&>
            - Fn是仿函数时，自身的去引用类型必须支持使用自身的左值和右值对象进行构造，且不是volatile类型
            - Fn是具有多个operator()重载的仿函数时，参数类型与Fn的一个operator()匹配即可
            - const、volatile限定的成员函数只能由const、volatile对象调用，对于仿函数也是一样
            - 要求Args...中非C数组类型的去引用类型均支持使用自身的左值和右值对象进行构造
            - 要求Fn的参数类型不能为右值引用
            - 其他要求参考std::bind规则
            用法：
            - StdBindChecker<Fn>::value，判断Fn是否为有效函数类型，类型为constexpr bool
            - StdBindChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn和Args...不匹配时ReturnType不存在   */
        template<class Fn, class... Args>
        struct StdBindChecker;
    }








    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mtype
    {
        template<class T>
        inline const char* getTypeName()
        {
#ifdef __GNUC__ 
            return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
            return typeid(T).name();
#endif
        }

        /*  将T类型转换为自身的右值引用类型，用于区别于std::add_rvalue_reference
            用法：RvalueRefMaker<T>::Type  */
        template<class T>
        struct RvalueRefMaker
        {
        public:
            using Type = typename std::add_rvalue_reference<typename std::remove_reference<T>::type>::type;
        };


        /*  检查Type1、Type2、Types...是否为相同类型，不忽略const、引用等符号
            用法：SameTypesChecker<Type1, Type2, Types...>::value, 类型为constexpr bool   */
        template<class Type1, class Type2, class ...Types>
        struct SameTypesChecker
        {
        private:
            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value&& mtype::SameTypesChecker<U1, Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class ...>
            static std::false_type check(...);

            SameTypesChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::SameTypesChecker<Type1, Type2, Types...>::template check<Type1, Type2, Types...>(0))::value;
        };
        template<class Type1, class Type2, class ...Types>
        constexpr bool mtype::SameTypesChecker<Type1, Type2, Types...>::value;


        /*  检查T是否为Type、Types...中的一个，不忽略const、引用等符号
            用法：InTypesChecker<T, Type, Types...>::value, 类型为constexpr bool   */
        template<class T, class Type, class ...Types>
        struct InTypesChecker
        {
        private:
            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value || mtype::InTypesChecker<U1, Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U1, class U2, class ...Us, typename std::enable_if<std::is_same<U1, U2>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class ...>
            static std::false_type check(...);

            InTypesChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::InTypesChecker<T, Type, Types...>::template check<T, Type, Types...>(0))::value;
        };
        template<class T, class Type, class ...Types>
        constexpr bool mtype::InTypesChecker<T, Type, Types...>::value;


        /*  检查T是否拥有类似于标准的STL容器的begin()和end()接口，支持模板SFINAE特性
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
            template<class U, typename std::enable_if<!std::is_const<U>::value&& std::is_same<decltype(std::declval<U>().begin()), typename std::remove_reference<U>::type::iterator>::value&& std::is_same<decltype(std::declval<U>().end()), typename std::remove_reference<U>::type::iterator>::value, int>::type = 0>
            static std::true_type checkExist(int);

            template<class U, typename std::enable_if<std::is_const<U>::value&& std::is_same<decltype(std::declval<U>().begin()), typename std::remove_reference<U>::type::const_iterator>::value&& std::is_same<decltype(std::declval<U>().end()), typename std::remove_reference<U>::type::const_iterator>::value, int>::type = 0>
            static std::true_type checkExist(int);

            template<class...>
            static std::false_type checkExist(...);

            template<class U, typename = decltype(*(std::declval<typename std::remove_reference<U>::type::iterator>())) >
            static std::true_type checkDeref(int);

            template<class...>
            static std::false_type checkDeref(...);

            template<class U, typename std::enable_if<!std::is_const<U>::value&& std::is_same<decltype(++std::declval<U>().begin()), typename std::remove_reference<U>::type::iterator&>::value&& std::is_same<decltype(std::declval<U>().begin()++), typename std::remove_reference<U>::type::iterator>::value, int>::type = 0>
            static std::true_type checkIncrementOperator(int);

            template<class U, typename std::enable_if<std::is_const<U>::value&& std::is_same<decltype(++std::declval<U>().begin()), typename std::remove_reference<U>::type::const_iterator&>::value&& std::is_same<decltype(std::declval<U>().begin()++), typename std::remove_reference<U>::type::const_iterator>::value, int>::type = 0>
            static std::true_type checkIncrementOperator(int);

            template<class...>
            static std::false_type checkIncrementOperator(...);

            StdBeginEndChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::StdBeginEndChecker<T>::template checkExist<T>(0))::value && decltype(mtype::StdBeginEndChecker<T>::template checkDeref<T>(0))::value && decltype(mtype::StdBeginEndChecker<T>::template checkIncrementOperator<T>(0))::value;
        };
        template<class T>
        constexpr bool mtype::StdBeginEndChecker<T>::value;

        template <typename... T>
        struct _make_void { using type = void; };

        template <typename... T>
        using _void_t = typename _make_void<T...>::type;


        /*  检查T是否支持std::cout <<，支持模板SFINAE特性
            - T类型正确重载了operator<<(std::ostream&, const T&)
            - 对于T类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            - 对T类型查找operator<<的范围为std命名空间、T所在命名空间及其关联命名空间，以及全局命名空间
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
            static constexpr bool value = decltype(mtype::StdCoutChecker<T>::template check<T>(0))::value;
        };
        template<class T>
        constexpr bool mtype::StdCoutChecker<T>::value;


        /*  检查T、Ts中的每一个类型是否都支持std::cout <<，支持模板SFINAE特性
            - 每一个T、Ts类型的对象都正确重载了operator<<(std::ostream&, const T&)
            - 对于每一个T、Ts类型的对象obj，std::cout << obj的返回值类型为std::ostream&
            - 对每一个T、Ts类型查找operator<<的范围为std命名空间、当前T所在命名空间及其关联命名空间，以及全局命名空间
            用法：StdCoutEachChecker<T, Ts...>::value, 类型为constexpr bool   */
        template<class T, class... Ts>
        struct StdCoutEachChecker
        {
        private:
            template<class U, class... Us, typename std::enable_if<mtype::StdCoutChecker<U>::value&& mtype::StdCoutEachChecker<Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U, class... Us, typename std::enable_if<mtype::StdCoutChecker<U>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class...>
            static std::false_type check(...);

            StdCoutEachChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::StdCoutEachChecker<T, Ts...>::template check<T, Ts...>(0))::value;
        };
        template<class T, class... Ts>
        constexpr bool mtype::StdCoutEachChecker<T, Ts...>::value;



        /*  检查DstT是否可以由T、Ts...类型对象中的每一个单独构造或引用绑定，支持模板SFINAE特性
            - 检查规则参照std::is_constructible<DstT, T>
            - T、Ts...中的非引用类型，会被作为右值类型进行检查
            - DstT作为非引用类型时，检查DstT是否可以由T、Ts...中每一个的对象构造
            - DstT作为引用类型时，检查DstT是否可以绑定到每一个T、Ts...对象上
            - 构造过程中的隐式转换也是被支持的
            用法：ConstructibleFromEachChecker<DstT, T, Ts...>::value，类型为constexpr bool   */
        template<class DstT, class T, class... Ts>
        struct ConstructibleFromEachChecker
        {
        private:
            template<class DstU, class U, class... Us, typename std::enable_if<std::is_constructible<DstU, U>::value&& mtype::ConstructibleFromEachChecker<DstU, Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class DstU, class U, class... Us, typename std::enable_if<std::is_constructible<DstU, U>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class...>
            static std::false_type check(...);

            ConstructibleFromEachChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::ConstructibleFromEachChecker<DstT, T, Ts...>::template check<DstT, T, Ts...>(0))::value;
        };
        template<class DstT, class T, class... Ts>
        constexpr bool mtype::ConstructibleFromEachChecker<DstT, T, Ts...>::value;

        /*  检查T、Ts...类型中的每一个是否都支持使用自身的左值进行构造或引用绑定，支持模板SFINAE特性
            - 对于T、Ts...中的非引用类型，检查其能否由自身的左值对象构造
            - 对于T、Ts...中的引用类型，检查其能否绑定到自身的左值对象上
            用法：EachLvalueConstructibleChecker<T, Ts...>::value，类型为constexpr bool   */
        template<class T, class... Ts>
        struct EachLvalueConstructibleChecker
        {
        private:
            template<class U, class... Us, typename std::enable_if<std::is_constructible<U, typename std::add_lvalue_reference<U>::type>::value&& mtype::EachLvalueConstructibleChecker<Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U, class... Us, typename std::enable_if<std::is_constructible<U, typename std::add_lvalue_reference<U>::type>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class...>
            static std::false_type check(...);

            EachLvalueConstructibleChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::EachLvalueConstructibleChecker<T, Ts...>::template check<T, Ts...>(0))::value;
        };
        template<class T, class... Ts>
        constexpr bool mtype::EachLvalueConstructibleChecker<T, Ts...>::value;


        /*  检查T、Ts...类型中的每一个是否都支持使用自身的右值进行构造或引用绑定，支持模板SFINAE特性
            - 对于T、Ts...中的非引用类型，检查其能否由自身的右值对象构造
            - 对于T、Ts...中的引用类型，检查其能否绑定到自身的右值对象上
            用法：EachRvalueConstructibleChecker<T, Ts...>::value，类型为constexpr bool   */
        template<class T, class... Ts>
        struct EachRvalueConstructibleChecker
        {
        private:
            template<class U, class... Us, typename std::enable_if<std::is_constructible<U, typename mtype::RvalueRefMaker<U>::Type>::value&& mtype::EachRvalueConstructibleChecker<Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U, class... Us, typename std::enable_if<std::is_constructible<U, typename mtype::RvalueRefMaker<U>::Type>::value && (sizeof...(Us) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class...>
            static std::false_type check(...);

            EachRvalueConstructibleChecker() = delete;

        public:
            static constexpr bool value = decltype(mtype::EachRvalueConstructibleChecker<T, Ts...>::template check<T, Ts...>(0))::value;
        };
        template<class T, class... Ts>
        constexpr bool mtype::EachRvalueConstructibleChecker<T, Ts...>::value;


        struct _FunctionCheckerBase
        {
        public:
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

            template <class Functor, class CallType = decltype(&std::remove_reference<Functor>::type::operator()), class Ret = decltype(mtype::_FunctionCheckerBase::checkRet(&std::remove_reference<Functor>::type::operator()))>
            static Ret checkRet(Functor&&);

            template <class Ret, class... Arguments>
            static std::tuple<Arguments...> checkArguments(Ret(*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static std::tuple<Arguments...> checkArguments(Ret(ObjType::*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static std::tuple<Arguments...> checkArguments(Ret(ObjType::*)(Arguments...) const);

            template <class Ret, class ObjType, class... Arguments>
            static std::tuple<Arguments...> checkArguments(Ret(ObjType::*)(Arguments...) volatile);

            template <class Ret, class ObjType, class... Arguments>
            static std::tuple<Arguments...> checkArguments(Ret(ObjType::*)(Arguments...) const volatile);

            template <class Functor, class ArgumentsTuple = decltype(mtype::_FunctionCheckerBase::checkArguments(&std::remove_reference<Functor>::type::operator()))>
            static ArgumentsTuple checkArguments(Functor&&);

            //要避免void作为形参传递的可能，因此不宜用Fn&&传递对象
            //template<class Fn, class Ret = decltype(mtype::_FunctionCheckerBase::checkRet(std::declval<typename std::remove_reference<Fn>::type>()))>
            //static std::true_type checkValidity(Fn&&);
            
            template<class Fn, class Ret = decltype(mtype::_FunctionCheckerBase::checkRet(std::declval<typename std::remove_reference<Fn>::type>()))>
            static std::true_type checkValidity(int);

            template<class...>
            static std::false_type checkValidity(...);
        };


        template <bool Checker, class Fn>
        struct _FunctionCheckerHelper
        {
        public:
            static constexpr bool value = false;
        };
        template <bool Checker, class Fn>
        constexpr bool mtype::_FunctionCheckerHelper<Checker, Fn>::value;


        template <class Fn>
        struct _FunctionCheckerHelper<true, Fn>
        {
        public:
            static constexpr bool value = true;
            using ReturnType = decltype(mtype::_FunctionCheckerBase::checkRet(std::declval<Fn>()));
            using ArgsTupleType = decltype(mtype::_FunctionCheckerBase::checkArguments(std::declval<Fn>()));
            static constexpr size_t num_args = std::tuple_size<ArgsTupleType>::value;
        };
        template <class Fn>
        constexpr bool mtype::_FunctionCheckerHelper<true, Fn>::value;
        template <class Fn>
        constexpr size_t mtype::_FunctionCheckerHelper<true, Fn>::num_args;


        /*  对Fn进行检查，判断Fn是否为可解析的函数、仿函数或成员函数，并获取返回值、参数数量等信息，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针和仿函数类型
            - Fn接收有多个重载的函数和成员函数时，需要显式指定函数类型，如FuncChecker<decltype((void(*)(int&))func1)>
            - Fn是仿函数时，如果仿函数重载了多个operator()，那么FuncChecker<Fn>::value为false
            - Fn是仿函数时，对operator()的检查不会考虑其const、volatile等限定
            用法：
            - FuncChecker<Fn>::value，判断Fn是否为有效函数，类型为constexpr bool
            - FuncChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn不是有效的函数类型时ReturnType不存在
            - FuncChecker<Fn>::ArgsTupleType，获取Fn的参数列表类型组成的std::tuple类型，Fn不是有效的函数类型时ArgsTupleType不存在
            - FuncChecker<Fn>::num_args，类型为constexpr size_t，获得Fn的参数数量，Fn不是有效的函数类型时num_args不存在   */
        template <class Fn>
        struct FuncChecker : public mtype::_FunctionCheckerHelper<decltype(mtype::_FunctionCheckerBase::template checkValidity<Fn>(0))::value, Fn>
        {
        private:
            FuncChecker() = delete;
        };

        //std::bind在qnx的gcc4.7.3上，无法绑定const仿函数和它的非const operator()，在其他编译器上却可以，因此统一按照严格的限定处理
        template <class Fn, class... Args>
        struct _FunctorBindChecker
        {
            //此处仍然存在风险：在gcc4.7.3这个C++特性支持不完全的编译器上，如果用户自行实现了类似std::reference_wrapper的模板类，即user_reference_wrapper<Func>，其中Func的cv限定符和实际调用的operator()不匹配会产生编译错误而非触发SFINAE
            template <class Func, class... Arguments, typename std::enable_if<std::is_class<typename std::remove_reference<Func>::type>::value, int>::type = 0, class Ret = decltype(std::declval<Func>()(std::declval<typename std::add_lvalue_reference<Arguments>::type>()...))>
            static std::true_type check(int);

            //似乎在gcc4.7.3上，const ObjPtr*指向非const的Func函数会直接导致编译错误而非SFINAE，而开启-fpermissive后又会判断通过，均不符合原意
            //template <class Func, class ObjPtr, class... Arguments, typename std::enable_if<std::is_member_function_pointer<Func>::value, int>::type = 0, class Ret = decltype((std::declval<ObjPtr>()->*std::declval<Func>())(std::declval<Arguments>()...)) >
            //static std::true_type check(int);

            template <class...>
            static std::false_type check(...);

            //同样因为gcc4.7.3上std::reference_wrapper<Func>包装的Func进行上面的check，Func的cv属性不匹配会直接报错而不是触发SFINAE
            template<class Func>
            static Func checkFn(std::reference_wrapper<Func>);

            template<class Func>
            static Func checkFn(Func&&);

            //在当前的使用情况中，Func不可能作为void被传进来
            using Func = decltype(mtype::_FunctorBindChecker<Fn, Args...>::template checkFn(std::declval<Fn>()));

            static constexpr bool value = decltype(mtype::_FunctorBindChecker<Func, Args...>::template check<Func, Args...>(0))::value;
        };
        template <typename Fn, typename... Args>
        constexpr bool mtype::_FunctorBindChecker<Fn, Args...>::value;

        //会去除Args...的引用后再检查
        template<class... Args>
        struct _EachConstructibleByLRvalueChecker
        {
        private:
            template<class Argument, class... Arguments, typename std::enable_if<std::is_array<typename std::remove_reference<Argument>::type>::value, int>::type = 0, typename std::enable_if<mtype::_EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class Argument, class... Arguments, typename std::enable_if<!std::is_array<typename std::remove_reference<Argument>::type>::value, int>::type = 0, typename std::enable_if<mtype::ConstructibleFromEachChecker<typename std::remove_reference<Argument>::type, typename std::add_lvalue_reference<Argument>::type, typename mtype::RvalueRefMaker<Argument>::Type>::value&& mtype::_EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
            static std::true_type check(int);

            //template<class Argument, class... Arguments, typename std::enable_if<mtype::ConstructibleFromEachChecker<typename std::remove_reference<Argument>::type, typename std::add_lvalue_reference<Argument>::type, typename mtype::RvalueRefMaker<Argument>::Type>::value&& mtype::_EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
            //static std::true_type check(int);

            template<class... Arguments, typename std::enable_if<(sizeof...(Arguments) == 0), int>::type = 0>
            static std::true_type check(int);

            template<class Argument, class... Arguments>
            static std::false_type check(...);

        public:
            static constexpr bool value = decltype(mtype::_EachConstructibleByLRvalueChecker<Args...>::template check<Args...>(0))::value;
        };
        template <class... Args>
        constexpr bool mtype::_EachConstructibleByLRvalueChecker<Args...>::value;


        template<class Fn>
        struct _MemFuncCvChecker
        {
        public:
            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkOnlyConst(Ret(ObjType::*)(Arguments...) const);

            template <class...>
            static std::false_type checkOnlyConst(...);

            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkOnlyVolatile(Ret(ObjType::*)(Arguments...) volatile);

            template <class...>
            static std::false_type checkOnlyVolatile(...);

            template <class Ret, class ObjType, class... Arguments>
            static std::true_type checkConstVolatile(Ret(ObjType::*)(Arguments...) const volatile);

            template <class...>
            static std::false_type checkConstVolatile(...);

            template <class Ret, class ObjType, class... Arguments>
            static ObjType checkObjType(Ret(ObjType::*)(Arguments...));

            template <class Ret, class ObjType, class... Arguments>
            static ObjType checkObjType(Ret(ObjType::*)(Arguments...) const);

            template <class Ret, class ObjType, class... Arguments>
            static ObjType checkObjType(Ret(ObjType::*)(Arguments...) volatile);

            template <class Ret, class ObjType, class... Arguments>
            static ObjType checkObjType(Ret(ObjType::*)(Arguments...) const volatile);

        public:
            //目前在使用时Fn不可能为void
            static constexpr bool is_only_const = decltype(mtype::_MemFuncCvChecker<Fn>::template checkOnlyConst(std::declval<Fn>()))::value;
            static constexpr bool is_only_volatile = decltype(mtype::_MemFuncCvChecker<Fn>::template checkOnlyVolatile(std::declval<Fn>()))::value;
            static constexpr bool is_const_volatile = decltype(mtype::_MemFuncCvChecker<Fn>::template checkConstVolatile(std::declval<Fn>()))::value;

            static constexpr bool is_const = mtype::_MemFuncCvChecker<Fn>::is_only_const || mtype::_MemFuncCvChecker<Fn>::is_const_volatile;
            static constexpr bool is_volatile = mtype::_MemFuncCvChecker<Fn>::is_only_volatile || mtype::_MemFuncCvChecker<Fn>::is_const_volatile;
            static constexpr bool is_no_cv = !mtype::_MemFuncCvChecker<Fn>::is_const && !mtype::_MemFuncCvChecker<Fn>::is_volatile;
        };
        template<class Fn>
        constexpr bool mtype::_MemFuncCvChecker<Fn>::is_only_const;
        template<class Fn>
        constexpr bool mtype::_MemFuncCvChecker<Fn>::is_only_volatile;
        template<class Fn>
        constexpr bool mtype::_MemFuncCvChecker<Fn>::is_const_volatile;
        template<class Fn>
        constexpr bool mtype::_MemFuncCvChecker<Fn>::is_const;
        template<class Fn>
        constexpr bool mtype::_MemFuncCvChecker<Fn>::is_volatile;
        template<class Fn>
        constexpr bool mtype::_MemFuncCvChecker<Fn>::is_no_cv;

        //qnx的gcc4.7.3无法正确处理std::bind绑定非cv限定的成员函数和cv限定的对象，以及std::declval<ObjT>()->*std::declval<Func>()的情况，因此添加这个检查
        template<class MemFn, class ObjMaybePtr, class...Args>
        struct _MemFnObjCvMatchChecker
        {
            template<class Obj>
            static Obj checkObj(std::reference_wrapper<Obj>);
            template<class Obj>
            static Obj checkObj(const Obj&);
        
            using ObjNoStdRef = decltype(mtype::_MemFnObjCvMatchChecker<MemFn, ObjMaybePtr, Args...>::template checkObj(std::declval<ObjMaybePtr>()));
            using Obj = typename std::remove_reference<typename std::remove_pointer<ObjNoStdRef>::type>::type;
            using DecayObj = typename std::remove_cv<Obj>::type;
            //static constexpr bool only_const_match = mtype::_MemFuncCvChecker<MemFn>::is_only_const && !std::is_volatile<Obj>::value;
            //static constexpr bool only_volatile_match = mtype::_MemFuncCvChecker<MemFn>::is_only_volatile && !std::is_const<Obj>::value;
            //static constexpr bool const_volatile_match = mtype::_MemFuncCvChecker<MemFn>::is_const_volatile;
            //static constexpr bool no_cv_match = mtype::_MemFuncCvChecker<MemFn>::is_no_cv && !std::is_const<Obj>::value && !std::is_volatile<Obj>::value;

        public:
            //在当前的使用情况中，MemFn不可能作为void被传进来
            static constexpr bool value = (mtype::_MemFuncCvChecker<MemFn>::is_const_volatile ||
                mtype::_MemFuncCvChecker<MemFn>::is_only_const && !std::is_volatile<Obj>::value ||
                mtype::_MemFuncCvChecker<MemFn>::is_only_volatile && !std::is_const<Obj>::value ||
                mtype::_MemFuncCvChecker<MemFn>::is_no_cv && !std::is_const<Obj>::value && !std::is_volatile<Obj>::value) &&
                (std::is_same<decltype(mtype::_MemFuncCvChecker<MemFn>::template checkObjType(std::declval<MemFn>())), DecayObj>::value ||
                    std::is_base_of<decltype(mtype::_MemFuncCvChecker<MemFn>::template checkObjType(std::declval<MemFn>())), DecayObj>::value);
        };
        template<class MemFn, class ObjMaybePtr, class...Args>
        constexpr bool mtype::_MemFnObjCvMatchChecker<MemFn, ObjMaybePtr, Args...>::value;


        template<class Fn, class... Args>
        struct _StdBindCheckerBase
        {
        public:
            template<class Func, class... Arguments, class DecayFunc = typename std::remove_reference<Func>::type, typename std::enable_if<!std::is_class<DecayFunc>::value && !std::is_member_function_pointer<DecayFunc>::value && (mtype::FuncChecker<Func>::num_args == sizeof...(Arguments)) && mtype::_EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
            static std::true_type checkConditionsUnsupportSFINAE(int);

            //检查Functor和std::ref包装的函数
            template<class Func, class... Arguments, class DecayFunc = typename std::remove_reference<Func>::type, typename std::enable_if <std::is_class<DecayFunc>::value && !std::is_volatile<DecayFunc>::value, int>::type = 0, typename std::enable_if<mtype::_EachConstructibleByLRvalueChecker<Func, Arguments...>::value && mtype::_FunctorBindChecker<Func, Arguments...>::value, int> ::type = 0 >
            static std::true_type checkConditionsUnsupportSFINAE(int);  

            template<class Func, class... Arguments, typename std::enable_if<std::is_member_function_pointer<typename std::remove_reference<Func>::type>::value && (FuncChecker<Func>::num_args + 1 == sizeof...(Arguments)) && mtype::_EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0, typename std::enable_if<mtype::_MemFnObjCvMatchChecker<Func, Arguments...>::value, int>::type = 0>
            static std::true_type checkConditionsUnsupportSFINAE(int);

            template<class ...>
            static std::false_type checkConditionsUnsupportSFINAE(...);


            template<class Func, class... Arguments, typename std::enable_if<decltype(mtype::_StdBindCheckerBase<Func, Arguments...>::template checkConditionsUnsupportSFINAE<Func, Arguments...>(0))::value, int>::type = 0>
            static auto checkRet(int) -> decltype(std::bind(std::forward<Func>(std::declval<Func>()), std::forward<Arguments>(std::declval<Arguments>())...)());

            //template<class Func, class... Arguments, class DecayFunc = decltype(std::declval<Func>().get())>
            //static auto checkValue(int) -> decltype(mtype::_StdBindCheckerBase<DecayFunc, Arguments...>::template checkRet<DecayFunc, Arguments...>(0), std::true_type());

            template<class Func, class... Arguments>
            static auto checkValue(int) -> decltype(mtype::_StdBindCheckerBase<Func, Arguments...>::template checkRet<Func, Arguments...>(0), std::true_type());

            template<class...>
            static std::false_type checkValue(...);
        };

        template <bool Checker, class Fn, class... Args>
        struct _StdBindCheckerHelper
        {
        public:
            static constexpr bool value = decltype(mtype::_StdBindCheckerBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value;
        };
        template <bool Checker, class Fn, class... Args>
        constexpr bool mtype::_StdBindCheckerHelper<Checker, Fn, Args...>::value;


        template <class Fn, class... Args>
        struct _StdBindCheckerHelper<true, Fn, Args...>
        {
        public:
            static constexpr bool value = decltype(mtype::_StdBindCheckerBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value;
            using ReturnType = decltype(mtype::_StdBindCheckerBase<Fn, Args...>::template checkRet<Fn, Args...>(0));
        };
        template <class Fn, class... Args>
        constexpr bool mtype::_StdBindCheckerHelper<true, Fn, Args...>::value;


        /*  检查Fn和Args...参数是否能使用std::bind绑定，且具有更严格的限制，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针、仿函数类型以及它们的reference_warpper包装
            - Fn接收重载函数时，需要显式指定函数类型，如StdBindChecker<decltype((void(*)(int&))func1), int&>
            - Fn是仿函数时，自身的去引用类型必须支持使用自身的左值和右值对象进行构造，且不是volatile类型
            - Fn是具有多个operator()重载的仿函数时，参数类型与Fn的一个operator()匹配即可
            - const、volatile限定的成员函数只能由const、volatile对象调用，对于仿函数也是一样
            - 要求Args...中非C数组类型的去引用类型均支持使用自身的左值和右值对象进行构造
            - 要求Fn的参数类型不能为右值引用
            - 其他要求参考std::bind规则
            用法：
            - StdBindChecker<Fn>::value，判断Fn是否为有效函数类型，类型为constexpr bool
            - StdBindChecker<Fn>::ReturnType，获取Fn的返回值类型，Fn和Args...不匹配时ReturnType不存在   */
        template<class Fn, class... Args>
        struct StdBindChecker : public mtype::_StdBindCheckerHelper<decltype(mtype::_StdBindCheckerBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value, Fn, Args...>
        {
        private:
            StdBindChecker() = delete;
        };


        /*  用于判断类型是不是相同类型
            -忽略引用&修饰
            -比较非指针类型时忽略const修饰
            -比较指针或C风格数组类型时忽略作用于对该指针的const修饰   */
        template<class T1, class T2, class ...Types>
        mdeprecated(R"(Deprecated. Please replace with struct "SameTypesChecker"(in type.hpp))") constexpr bool isSameType();


        //用于判断T是否属于后面的多种类型
        template<class T, class Tother, class... Tothers>
        mdeprecated(R"(Deprecated. Please replace with struct "SameTypesChecker"(in type.hpp))") constexpr bool isInTypes();

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
        mdeprecated(R"(Deprecated. Please replace with struct "InTypesChecker"(in type.hpp))") inline constexpr bool isInTypes()
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
            static_assert(mtype::SameTypesChecker<int, unsigned int>::value == false, "assert failed!");
            static_assert(mtype::SameTypesChecker<int, unsigned int, int, int>::value == false, "assert failed!");
            static_assert(mtype::SameTypesChecker<int, int, int, unsigned int>::value == false, "assert failed!");
            static_assert(mtype::SameTypesChecker<int, int>::value == true, "assert failed!");
            static_assert(mtype::SameTypesChecker<int, int, int, int>::value == true, "assert failed!");
        }

        inline void InTypesCheckerTest()
        {
            static_assert(mtype::InTypesChecker<int, unsigned int>::value == false, "assert failed!");
            static_assert(mtype::InTypesChecker<int, unsigned int, float, char>::value == false, "assert failed!");
            static_assert(mtype::InTypesChecker<int, int>::value == true, "assert failed!");
            static_assert(mtype::InTypesChecker<int, unsigned int, int, float>::value == true, "assert failed!");
        }

        inline void StdBeginEndCheckerTest()
        {
            static_assert(mtype::StdBeginEndChecker<int>::value == false, "assert failed!");
            static_assert(mtype::StdBeginEndChecker<std::vector<int>>::value == true, "assert failed!");
            static_assert(mtype::StdBeginEndChecker<const std::vector<int>>::value == true, "assert failed!");
        }

        inline void StdCoutCheckerTest()
        {
            static_assert(mtype::StdCoutChecker<int>::value == true, "assert failed!");
            static_assert(mtype::StdCoutChecker<std::vector<int>>::value == false, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<int>::value == true, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<std::vector<int>>::value == false, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<int, float, char>::value == true, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<int, std::vector<int>, char>::value == false, "assert failed!");
        }

        inline void ConstructibleFromEachCheckerTest()
        {
            static_assert(mtype::ConstructibleFromEachChecker<void, char>::value == false, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::string&, const char*>::value == false, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, int>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<float>>::value == false, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<int, char, float>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, int, float>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, std::vector<float>, float>::value == false, "assert failed!");
        }

        class _MyClass1
        {
        public:
            _MyClass1(const _MyClass1& tmp) {}
        };

        class _MyClass2
        {
        public:
            _MyClass2() {};
            _MyClass2(const _MyClass2& tmp) {}
            _MyClass2(_MyClass2&& tmp) = delete;

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


        inline void EachLRvalueConstructibleChecker()
        {
            static_assert(mtype::EachLvalueConstructibleChecker<_MyClass1>::value == true, "assert failed!");
            static_assert(mtype::EachLvalueConstructibleChecker<_MyClass1, _MyClass2>::value == true, "assert failed!");
            static_assert(mtype::EachRvalueConstructibleChecker<_MyClass1>::value == true, "assert failed!");
            static_assert(mtype::EachRvalueConstructibleChecker<_MyClass1, _MyClass2>::value == false, "assert failed!");
        }

        inline int _myFunc1(const int& a, int b)
        {
            return a + b;
        }

        inline void _myFunc2() {}

        class _MyClass3
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

        class _Functor1
        {
        public:
            void operator()() const {}
            char operator()(char a) { printf("char a\n"); return 0; };
            short operator()(short a) { printf("short a\n"); return 0; };
            short operator()(std::string& a) volatile { printf("string a\n"); return 0; };

            void func(int a) {}
            void funcV(int a) volatile{}
        };
        class _Functor1Son : public _Functor1
        {

        };

        class _Functor2
        {
        public:
            void operator()() {}
        };

        inline void FunctionCheckerTest()
        {
            static_assert(mtype::FuncChecker<int>::value == false, "assert failed!");
            static_assert(mtype::FuncChecker<decltype(_myFunc1)>::value == true, "assert failed!");
            static_assert(mtype::FuncChecker<decltype(&_MyClass3::func1)>::value == true, "assert failed!");
            static_assert(mtype::FuncChecker<decltype(&_MyClass3::func3)>::value == true, "assert failed!");
            static_assert(mtype::FuncChecker<_Functor1>::value == false, "assert failed!");
            static_assert(mtype::FuncChecker<volatile _Functor1>::value == false, "assert failed!");
            static_assert(mtype::FuncChecker<volatile _Functor2>::value == true, "assert failed!");
            static_assert(mtype::FuncChecker<const void>::value == false, "assert failed!");

            int num_args = 0;
            using type0 = mtype::FuncChecker<decltype(_myFunc1)>::ReturnType;
            using tuple_type0 = mtype::FuncChecker<decltype(_myFunc1)>::ArgsTupleType;
            num_args = mtype::FuncChecker<decltype(_myFunc1)>::num_args;
            printf("User Check! FuncChecker<decltype(_myFunc1)>::RetType(int):%s, ArgsTupleType(std::tuple<int const&, int>):%s, num_args(2):%d.\n", mtype::getTypeName<type0>(), mtype::getTypeName<tuple_type0>(), num_args);

            using type1 = mtype::FuncChecker<decltype(&_MyClass3::func1)>::ReturnType;
            num_args = mtype::FuncChecker<decltype(&_MyClass3::func1)>::num_args;
            printf("User Check! FuncChecker<decltype(&_MyClass3::func1)>::ReturnType(int):%s, num_args(1):%d.\n", mtype::getTypeName<type1>(), num_args);

            using type1 = mtype::FuncChecker<decltype(&_MyClass3::func3)>::ReturnType;
            num_args = mtype::FuncChecker<decltype(&_MyClass3::func3)>::num_args;
            printf("User Check! FuncChecker<decltype(&_MyClass3::func3)>::ReturnType(int):%s, num_args(1):%d.\n", mtype::getTypeName<type1>(), num_args);
            printf("\n");
        }

        inline void StdBindCheckerTest()
        {
            static_assert(mtype::StdBindChecker<int, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(_myFunc1), int, float>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(_myFunc1), int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(_myFunc1), int, int, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(_myFunc2), void>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<void, void>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::func1), int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::func1), std::vector<int>>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::func1), const _MyClass3*, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::func2), const _MyClass3*, int>::value == true, "assert failed!");

            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::func2), _MyClass2*, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::func3), const volatile _MyClass3&, int>::value == false, "assert failed!");  //没有默认可接收volatile类型的拷贝构造函数
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::staticFunc1), _MyClass3*, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_MyClass3::staticFunc1), int>::value == true, "assert failed!");

            static_assert(mtype::StdBindChecker<_Functor1, short&&>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<std::reference_wrapper<_Functor1>, short&&>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<std::reference_wrapper<const _Functor1>>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<std::reference_wrapper<const _Functor1>, short>::value == false, "assert failed!");

            static_assert(mtype::StdBindChecker<_Functor1, int&&>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<_Functor1, short>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<volatile _Functor1, short>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<const _Functor1, short>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_Functor1Son::func), _Functor1Son, int>::value == true, "assert failed!");

            static_assert(mtype::StdBindChecker<decltype(&_Functor1::func), std::reference_wrapper<_Functor1Son>, int>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_Functor1::func), std::reference_wrapper<_Functor1Son>&, int>::value == true, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_Functor1::func), std::reference_wrapper<volatile _Functor1Son>, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindChecker<decltype(&_Functor1::funcV), std::reference_wrapper<volatile _Functor1Son>, int>::value == true, "assert failed!");

            using type0 = mtype::StdBindChecker<decltype(_myFunc1), int, float>::ReturnType;
            printf("User Check! StdBindChecker<decltype(_myFunc1), int, float>::ReturnType(int):%s.\n", mtype::getTypeName<type0>());

            using type1 = mtype::StdBindChecker<decltype(&_MyClass3::func2), const _MyClass3*, int>::ReturnType;
            printf("User Check! StdBindChecker<decltype(&_MyClass3::func2), const _MyClass3*, int>::ReturnType(int):%s.\n", mtype::getTypeName<type1>());
            printf("\n");
        }

        inline void check()
        {
            printf("\n--------------------check mtype start--------------------\n\n");
            SameTypesCheckerTest();
            InTypesCheckerTest();
            StdBeginEndCheckerTest();
            StdCoutCheckerTest();
            ConstructibleFromEachCheckerTest();
            EachLRvalueConstructibleChecker();
            FunctionCheckerTest();
            StdBindCheckerTest();
            printf("--------------------check mtype end--------------------\n\n");
        }
    }
#endif
}

#endif // !TYPE_HPP_MINEUTILS