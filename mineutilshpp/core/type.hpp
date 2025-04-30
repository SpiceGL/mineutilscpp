//mineutils库的类型相关工具
#pragma once
#ifndef TYPE_HPP_MINEUTILS
#define TYPE_HPP_MINEUTILS

#include<functional>
#include<iostream>
#include<tuple>
#include<type_traits>
#include<vector>

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


        /*  检查T、Ts中的每一个类型是否都支持std::cout<<，支持模板SFINAE特性
            - 每一个T、Ts类型的对象都重载了std::ostream& operator<<(std::ostream&, const T&)
            - 对每一个T、Ts类型查找operator<<的范围为std命名空间、类型T所在命名空间及其关联命名空间，以及全局命名空间
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
            - Fn接收有多个重载的函数和成员函数时，需要显式指定函数类型，如FuncTraits<decltype((void(*)(int&))func1)>
            - Fn是仿函数时，如果仿函数重载了多个operator()，那么FuncTraits<Fn>::value为false
            - Fn是仿函数时，对operator()的检查不会考虑其const、volatile等限定
            - 由于编译器的设定，对于非引用的基本类型，ReturnType不保留顶层cv修饰（如返回const int会被忽略为int）
            用法：
            - FuncTraits<Fn>::value，判断Fn是否为有效函数，类型为constexpr bool
            - FuncTraits<Fn>::ReturnType，获取Fn的返回值类型，Fn不是有效的函数类型时ReturnType不存在
            - FuncTraits<Fn>::ArgsTupleType，获取Fn的参数列表类型组成的std::tuple类型，Fn不是有效的函数类型时ArgsTupleType不存在
            - FuncTraits<Fn>::num_args，类型为constexpr size_t，获得Fn的参数数量，Fn不是有效的函数类型时num_args不存在   */
        template <class Fn>
        struct FuncTraits;


        /*  检查Fn和Args...参数是否能使用std::bind绑定，且具有更严格的限制，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针、仿函数类型以及它们的reference_warpper包装
            - Fn接收重载函数时，需要显式指定函数类型，如StdBindTraits<decltype((void(*)(int&))func1), int&>
            - Fn是仿函数时，自身的去引用类型必须支持使用自身的左值和右值对象进行构造，且不是volatile类型
            - Fn是具有多个operator()重载的仿函数时，参数类型与Fn的一个operator()匹配即可
            - const、volatile限定的成员函数只能由const、volatile对象调用，对于仿函数也是一样
            - 要求Args...中非C数组类型的去引用类型均支持使用自身的左值和右值对象进行构造
            - 要求Fn的参数类型不能为右值引用
            - 由于编译器的设定，对于非引用的基本类型，ReturnType不保留顶层cv修饰（如返回const int会被忽略为int）
            - 其他要求参考std::bind规则
            用法：
            - StdBindTraits<Fn>::value，判断Fn是否为有效函数类型，类型为constexpr bool
            - StdBindTraits<Fn>::ReturnType，获取Fn的返回值类型，Fn和Args...不匹配时ReturnType不存在   */
        template<class Fn, class... Args>
        struct StdBindTraits;
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

        namespace _mpriv
        {
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
                static constexpr bool value = decltype(_mpriv::StdBeginEndChecker<T>::template checkExist<T>(0))::value && decltype(_mpriv::StdBeginEndChecker<T>::template checkDeref<T>(0))::value && decltype(_mpriv::StdBeginEndChecker<T>::template checkIncrementOperator<T>(0))::value;
            };
            template<class T>
            constexpr bool _mpriv::StdBeginEndChecker<T>::value;

            template <typename... T>
            struct _make_void { using type = void; };

            template <typename... T>
            using _void_t = typename _make_void<T...>::type;
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



        namespace _mpriv
        {
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
                static constexpr bool value = decltype(_mpriv::StdCoutChecker<T>::template check<T>(0))::value;
            };
            template<class T>
            constexpr bool _mpriv::StdCoutChecker<T>::value;
        }

        /*  检查T、Ts中的每一个类型是否都支持std::cout<<，支持模板SFINAE特性
            - 每一个T、Ts类型的对象都重载了std::ostream& operator<<(std::ostream&, const T&)
            - 对每一个T、Ts类型查找operator<<的范围为std命名空间、类型T所在命名空间及其关联命名空间，以及全局命名空间
            用法：StdCoutEachChecker<T, Ts...>::value, 类型为constexpr bool   */
        template<class T, class... Ts>
        struct StdCoutEachChecker
        {
        private:
            template<class U, class... Us, typename std::enable_if<_mpriv::StdCoutChecker<U>::value&& mtype::StdCoutEachChecker<Us...>::value, int>::type = 0>
            static std::true_type check(int);

            template<class U, class... Us, typename std::enable_if<_mpriv::StdCoutChecker<U>::value && (sizeof...(Us) == 0), int>::type = 0>
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


        namespace _mpriv
        {
            struct FunctionCheckerBase
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

                template <class Functor, class CallType = decltype(&std::remove_reference<Functor>::type::operator()), class Ret = decltype(_mpriv::FunctionCheckerBase::checkRet(&std::remove_reference<Functor>::type::operator()))>
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

                template <class Functor, class ArgumentsTuple = decltype(_mpriv::FunctionCheckerBase::checkArguments(&std::remove_reference<Functor>::type::operator()))>
                static ArgumentsTuple checkArguments(Functor&&);

                //要避免void作为形参传递的可能，因此不宜用Fn&&传递对象
                //template<class Fn, class Ret = decltype(_mpriv::FunctionCheckerBase::checkRet(std::declval<typename std::remove_reference<Fn>::type>()))>
                //static std::true_type checkValidity(Fn&&);

                template<class Fn, class Ret = decltype(_mpriv::FunctionCheckerBase::checkRet(std::declval<typename std::remove_reference<Fn>::type>()))>
                static std::true_type checkValidity(int);

                template<class...>
                static std::false_type checkValidity(...);
            };


            template <bool Checker, class Fn>
            struct FunctionCheckerHelper
            {
            public:
                static constexpr bool value = false;
            };
            template <bool Checker, class Fn>
            constexpr bool _mpriv::FunctionCheckerHelper<Checker, Fn>::value;


            template <class Fn>
            struct FunctionCheckerHelper<true, Fn>
            {
            public:
                static constexpr bool value = true;
                using ReturnType = decltype(_mpriv::FunctionCheckerBase::checkRet(std::declval<Fn>()));
                using ArgsTupleType = decltype(_mpriv::FunctionCheckerBase::checkArguments(std::declval<Fn>()));
                static constexpr size_t num_args = std::tuple_size<ArgsTupleType>::value;
            };
            template <class Fn>
            constexpr bool _mpriv::FunctionCheckerHelper<true, Fn>::value;
            template <class Fn>
            constexpr size_t _mpriv::FunctionCheckerHelper<true, Fn>::num_args;
        }


        /*  对Fn进行检查，判断Fn是否为可解析的函数、仿函数或成员函数，并获取返回值、参数数量等信息，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针和仿函数类型
            - Fn接收有多个重载的函数和成员函数时，需要显式指定函数类型，如FuncTraits<decltype((void(*)(int&))func1)>
            - Fn是仿函数时，如果仿函数重载了多个operator()，那么FuncTraits<Fn>::value为false
            - Fn是仿函数时，对operator()的检查不会考虑其const、volatile等限定
            - 由于编译器的设定，对于非引用的基本类型，ReturnType不保留顶层cv修饰（如返回const int会被忽略为int）
            用法：
            - FuncTraits<Fn>::value，判断Fn是否为有效函数，类型为constexpr bool
            - FuncTraits<Fn>::ReturnType，获取Fn的返回值类型，Fn不是有效的函数类型时ReturnType不存在
            - FuncTraits<Fn>::ArgsTupleType，获取Fn的参数列表类型组成的std::tuple类型，Fn不是有效的函数类型时ArgsTupleType不存在
            - FuncTraits<Fn>::num_args，类型为constexpr size_t，获得Fn的参数数量，Fn不是有效的函数类型时num_args不存在   */
        template <class Fn>
        struct FuncTraits : public _mpriv::FunctionCheckerHelper<decltype(_mpriv::FunctionCheckerBase::template checkValidity<Fn>(0))::value, Fn>
        {
        private:
            FuncTraits() = delete;
        };


        namespace _mpriv
        {
            //std::bind在qnx的gcc4.7.3上，无法绑定const仿函数和它的非const operator()，在其他编译器上却可以，因此统一按照严格的限定处理
            template <class Fn, class... Args>
            struct FunctorBindChecker
            {
                /*  此处在gcc4.7.3这个C++特性支持不完全的编译器上仍然存在风险：
                    - 如果用户自行实现了类似std::reference_wrapper的模板类，即user_reference_wrapper<Func>，其中Func的cv限定符和实际调用的operator()不匹配会产生编译错误而非触发SFINAE
                    - 如果匹配的operator()是私有或受保护的，那么会在产生编译错误而非触发SFINAE  */
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
                using Func = decltype(_mpriv::FunctorBindChecker<Fn, Args...>::template checkFn(std::declval<Fn>()));

                static constexpr bool value = decltype(_mpriv::FunctorBindChecker<Func, Args...>::template check<Func, Args...>(0))::value;
            };
            template <typename Fn, typename... Args>
            constexpr bool _mpriv::FunctorBindChecker<Fn, Args...>::value;

            //会去除Args...的引用后再检查
            template<class... Args>
            struct EachConstructibleByLRvalueChecker
            {
            private:
                template<class Argument, class... Arguments, typename std::enable_if<std::is_array<typename std::remove_reference<Argument>::type>::value, int>::type = 0, typename std::enable_if<_mpriv::EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
                static std::true_type check(int);

                template<class Argument, class... Arguments, typename std::enable_if<!std::is_array<typename std::remove_reference<Argument>::type>::value, int>::type = 0, typename std::enable_if<mtype::ConstructibleFromEachChecker<typename std::remove_reference<Argument>::type, typename std::add_lvalue_reference<Argument>::type, typename mtype::RvalueRefMaker<Argument>::Type>::value&& _mpriv::EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
                static std::true_type check(int);

                //template<class Argument, class... Arguments, typename std::enable_if<mtype::ConstructibleFromEachChecker<typename std::remove_reference<Argument>::type, typename std::add_lvalue_reference<Argument>::type, typename mtype::RvalueRefMaker<Argument>::Type>::value&& _mpriv::EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
                //static std::true_type check(int);

                template<class... Arguments, typename std::enable_if<(sizeof...(Arguments) == 0), int>::type = 0>
                static std::true_type check(int);

                template<class Argument, class... Arguments>
                static std::false_type check(...);

            public:
                static constexpr bool value = decltype(_mpriv::EachConstructibleByLRvalueChecker<Args...>::template check<Args...>(0))::value;
            };
            template <class... Args>
            constexpr bool _mpriv::EachConstructibleByLRvalueChecker<Args...>::value;


            template<class Fn>
            struct MemFuncCvChecker
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
                static constexpr bool is_only_const = decltype(_mpriv::MemFuncCvChecker<Fn>::template checkOnlyConst(std::declval<Fn>()))::value;
                static constexpr bool is_only_volatile = decltype(_mpriv::MemFuncCvChecker<Fn>::template checkOnlyVolatile(std::declval<Fn>()))::value;
                static constexpr bool is_const_volatile = decltype(_mpriv::MemFuncCvChecker<Fn>::template checkConstVolatile(std::declval<Fn>()))::value;

                static constexpr bool is_const = _mpriv::MemFuncCvChecker<Fn>::is_only_const || _mpriv::MemFuncCvChecker<Fn>::is_const_volatile;
                static constexpr bool is_volatile = _mpriv::MemFuncCvChecker<Fn>::is_only_volatile || _mpriv::MemFuncCvChecker<Fn>::is_const_volatile;
                static constexpr bool is_no_cv = !_mpriv::MemFuncCvChecker<Fn>::is_const && !_mpriv::MemFuncCvChecker<Fn>::is_volatile;
            };
            template<class Fn>
            constexpr bool _mpriv::MemFuncCvChecker<Fn>::is_only_const;
            template<class Fn>
            constexpr bool _mpriv::MemFuncCvChecker<Fn>::is_only_volatile;
            template<class Fn>
            constexpr bool _mpriv::MemFuncCvChecker<Fn>::is_const_volatile;
            template<class Fn>
            constexpr bool _mpriv::MemFuncCvChecker<Fn>::is_const;
            template<class Fn>
            constexpr bool _mpriv::MemFuncCvChecker<Fn>::is_volatile;
            template<class Fn>
            constexpr bool _mpriv::MemFuncCvChecker<Fn>::is_no_cv;

            //qnx的gcc4.7.3无法正确处理std::bind绑定非cv限定的成员函数和cv限定的对象，以及std::declval<ObjT>()->*std::declval<Func>()的情况，因此添加这个检查
            template<class MemFn, class ObjMaybePtr, class...Args>
            struct MemFnObjCvMatchChecker
            {
                template<class Obj>
                static Obj checkObj(std::reference_wrapper<Obj>);
                template<class Obj>
                static Obj checkObj(const Obj&);

                using ObjNoStdRef = decltype(_mpriv::MemFnObjCvMatchChecker<MemFn, ObjMaybePtr, Args...>::template checkObj(std::declval<ObjMaybePtr>()));
                using Obj = typename std::remove_reference<typename std::remove_pointer<ObjNoStdRef>::type>::type;
                using DecayObj = typename std::remove_cv<Obj>::type;
                //static constexpr bool only_const_match = _mpriv::MemFuncCvChecker<MemFn>::is_only_const && !std::is_volatile<Obj>::value;
                //static constexpr bool only_volatile_match = _mpriv::MemFuncCvChecker<MemFn>::is_only_volatile && !std::is_const<Obj>::value;
                //static constexpr bool const_volatile_match = _mpriv::MemFuncCvChecker<MemFn>::is_const_volatile;
                //static constexpr bool no_cv_match = _mpriv::MemFuncCvChecker<MemFn>::is_no_cv && !std::is_const<Obj>::value && !std::is_volatile<Obj>::value;

            public:
                //在当前的使用情况中，MemFn不可能作为void被传进来
                static constexpr bool value = (_mpriv::MemFuncCvChecker<MemFn>::is_const_volatile ||
                    _mpriv::MemFuncCvChecker<MemFn>::is_only_const && !std::is_volatile<Obj>::value ||
                    _mpriv::MemFuncCvChecker<MemFn>::is_only_volatile && !std::is_const<Obj>::value ||
                    _mpriv::MemFuncCvChecker<MemFn>::is_no_cv && !std::is_const<Obj>::value && !std::is_volatile<Obj>::value) &&
                    (std::is_same<decltype(_mpriv::MemFuncCvChecker<MemFn>::template checkObjType(std::declval<MemFn>())), DecayObj>::value ||
                        std::is_base_of<decltype(_mpriv::MemFuncCvChecker<MemFn>::template checkObjType(std::declval<MemFn>())), DecayObj>::value);
            };
            template<class MemFn, class ObjMaybePtr, class...Args>
            constexpr bool _mpriv::MemFnObjCvMatchChecker<MemFn, ObjMaybePtr, Args...>::value;


            template<class Fn, class... Args>
            struct StdBindTraitsBase
            {
            public:
                template<class Func, class... Arguments, class DecayFunc = typename std::remove_reference<Func>::type, typename std::enable_if<!std::is_class<DecayFunc>::value && !std::is_member_function_pointer<DecayFunc>::value && (mtype::FuncTraits<Func>::num_args == sizeof...(Arguments)) && _mpriv::EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0>
                static std::true_type checkConditionsUnsupportSFINAE(int);

                //检查Functor和std::ref包装的函数
                template<class Func, class... Arguments, class DecayFunc = typename std::remove_reference<Func>::type, typename std::enable_if <std::is_class<DecayFunc>::value && !std::is_volatile<DecayFunc>::value, int>::type = 0, typename std::enable_if<_mpriv::EachConstructibleByLRvalueChecker<Func, Arguments...>::value&& _mpriv::FunctorBindChecker<Func, Arguments...>::value, int> ::type = 0 >
                static std::true_type checkConditionsUnsupportSFINAE(int);

                template<class Func, class... Arguments, typename std::enable_if<std::is_member_function_pointer<typename std::remove_reference<Func>::type>::value && (FuncTraits<Func>::num_args + 1 == sizeof...(Arguments)) && _mpriv::EachConstructibleByLRvalueChecker<Arguments...>::value, int>::type = 0, typename std::enable_if<_mpriv::MemFnObjCvMatchChecker<Func, Arguments...>::value, int>::type = 0>
                static std::true_type checkConditionsUnsupportSFINAE(int);

                template<class ...>
                static std::false_type checkConditionsUnsupportSFINAE(...);


                template<class Func, class... Arguments, typename std::enable_if<decltype(_mpriv::StdBindTraitsBase<Func, Arguments...>::template checkConditionsUnsupportSFINAE<Func, Arguments...>(0))::value, int>::type = 0>
                static auto checkRet(int) -> decltype(std::bind(std::forward<Func>(std::declval<Func>()), std::forward<Arguments>(std::declval<Arguments>())...)());

                //template<class Func, class... Arguments, class DecayFunc = decltype(std::declval<Func>().get())>
                //static auto checkValue(int) -> decltype(_mpriv::StdBindTraitsBase<DecayFunc, Arguments...>::template checkRet<DecayFunc, Arguments...>(0), std::true_type());

                template<class Func, class... Arguments>
                static auto checkValue(int) -> decltype(_mpriv::StdBindTraitsBase<Func, Arguments...>::template checkRet<Func, Arguments...>(0), std::true_type());

                template<class...>
                static std::false_type checkValue(...);
            };

            template <bool Checker, class Fn, class... Args>
            struct StdBindTraitsHelper
            {
            public:
                static constexpr bool value = decltype(_mpriv::StdBindTraitsBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value;
            };
            template <bool Checker, class Fn, class... Args>
            constexpr bool _mpriv::StdBindTraitsHelper<Checker, Fn, Args...>::value;


            template <class Fn, class... Args>
            struct StdBindTraitsHelper<true, Fn, Args...>
            {
            public:
                static constexpr bool value = decltype(_mpriv::StdBindTraitsBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value;
                using ReturnType = decltype(_mpriv::StdBindTraitsBase<Fn, Args...>::template checkRet<Fn, Args...>(0));
            };
            template <class Fn, class... Args>
            constexpr bool _mpriv::StdBindTraitsHelper<true, Fn, Args...>::value;
        }

        /*  检查Fn和Args...参数是否能使用std::bind绑定，且具有更严格的限制，支持模板SFINAE特性
            - Fn支持函数、函数指针、成员函数指针、仿函数类型以及它们的reference_warpper包装
            - Fn接收重载函数时，需要显式指定函数类型，如StdBindTraits<decltype((void(*)(int&))func1), int&>
            - Fn是仿函数时，自身的去引用类型必须支持使用自身的左值和右值对象进行构造，且不是volatile类型
            - Fn是具有多个operator()重载的仿函数时，参数类型与Fn的一个operator()匹配即可
            - const、volatile限定的成员函数只能由const、volatile对象调用，对于仿函数也是一样
            - 要求Args...中非C数组类型的去引用类型均支持使用自身的左值和右值对象进行构造
            - 要求Fn的参数类型不能为右值引用
            - 由于编译器的设定，对于非引用的基本类型，ReturnType不保留顶层cv修饰（如返回const int会被忽略为int）
            - 其他要求参考std::bind规则
            用法：
            - StdBindTraits<Fn>::value，判断Fn是否为有效函数类型，类型为constexpr bool
            - StdBindTraits<Fn>::ReturnType，获取Fn的返回值类型，Fn和Args...不匹配时ReturnType不存在
            注意，经测试QNX的g++4.7.3对C++11特性支持不全，以下情况可能直接在模板内部编译错误而非触发SFINAE特性：
            - 仿函数作为Fn，但使用类似std::reference_wrapper的第三方引用包装传递时
            - 仿函数作为Fn，但匹配Args...的operator()为私有或受保护的成员时  */
        template<class Fn, class... Args>
        struct StdBindTraits : public _mpriv::StdBindTraitsHelper<decltype(_mpriv::StdBindTraitsBase<Fn, Args...>::template checkValue<Fn, Args...>(0))::value, Fn, Args...>
        {
        private:
            StdBindTraits() = delete;
        };



        namespace _mpriv
        {
            template<class T>
            struct AnyCVRefChecker
            {
            private:
                template<class U, typename std::enable_if<std::is_const<U>::value || std::is_volatile<U>::value || std::is_reference<U>::value, int>::type = 0>
                static std::true_type check(int);

                template<class ...>
                static std::false_type check(...);
            public:
                static constexpr bool value = decltype(_mpriv::AnyCVRefChecker<T>::template check<T>(0))::value;
            };
            template<class T>
            constexpr bool _mpriv::AnyCVRefChecker<T>::value;


            template<class T>
            struct CircularQueueElemChecker
            {
            private:
                template<class U, typename std::enable_if<!std::is_void<U>::value && !_mpriv::AnyCVRefChecker<U>::value && (std::is_assignable<U&, const U&>::value || std::is_assignable<U&, U&&>::value) && std::is_default_constructible<U>::value, int>::type = 0>
                static std::true_type check(int);

                template<class ...>
                static std::false_type check(...);
            public:
                static constexpr bool value = decltype(_mpriv::CircularQueueElemChecker<T>::template check<T>(0))::value;
            };
            template<class T>
            constexpr bool _mpriv::CircularQueueElemChecker<T>::value;


            template<class T>
            struct RectElemChecker
            {
            private:
                template<class U, typename std::enable_if<!_mpriv::AnyCVRefChecker<U>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
                static std::true_type check(int);

                template<class ...>
                static std::false_type check(...);
            public:
                static constexpr bool value = decltype(_mpriv::RectElemChecker<T>::template check<T>(0))::value;
            };
            template<class T>
            constexpr bool _mpriv::RectElemChecker<T>::value;
        }
    }


#ifdef MINEUTILS_TEST_MODULES
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

        inline void privStdBeginEndCheckerTest()
        {
            static_assert(mtype::_mpriv::StdBeginEndChecker<int>::value == false, "assert failed!");
            static_assert(mtype::_mpriv::StdBeginEndChecker<std::vector<int>>::value == true, "assert failed!");
            static_assert(mtype::_mpriv::StdBeginEndChecker<const std::vector<int>>::value == true, "assert failed!");
        }

        inline void privStdCoutCheckerTest()
        {
            static_assert(mtype::_mpriv::StdCoutChecker<int>::value == true, "assert failed!");
            static_assert(mtype::_mpriv::StdCoutChecker<std::vector<int>>::value == false, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<int>::value == true, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<std::vector<int>>::value == false, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<int, float, char>::value == true, "assert failed!");
            static_assert(mtype::StdCoutEachChecker<int, std::vector<int>, char>::value == false, "assert failed!");
        }

        inline void ConstructibleFromEachCheckerTest()
        {
            static_assert(mtype::ConstructibleFromEachChecker<std::string, std::string>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<void, char>::value == false, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::string&, const char*>::value == false, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, int>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<float>>::value == false, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<int, char, float>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, int, float>::value == true, "assert failed!");
            static_assert(mtype::ConstructibleFromEachChecker<std::vector<int>, std::vector<int>, std::vector<float>, float>::value == false, "assert failed!");
        }

        class MyClass1
        {
        public:
            MyClass1(const MyClass1& tmp) {}
        };

        class MyClass2
        {
        public:
            MyClass2() {};
            MyClass2(const MyClass2& tmp) {}
            MyClass2(MyClass2&& tmp) = delete;

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
            static_assert(mtype::EachLvalueConstructibleChecker<MyClass1>::value == true, "assert failed!");
            static_assert(mtype::EachLvalueConstructibleChecker<MyClass1, MyClass2>::value == true, "assert failed!");
            static_assert(mtype::EachRvalueConstructibleChecker<MyClass1>::value == true, "assert failed!");
            static_assert(mtype::EachRvalueConstructibleChecker<MyClass1, MyClass2>::value == false, "assert failed!");
        }

        inline int _myFunc1(const int& a, int b)
        {
            return a + b;
        }

        inline void _myFunc2() {}

        class MyClass3
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

        class Functor1
        {
        public:
            void operator()() const {}
            char operator()(char a) { printf("char a\n"); return 0; };
            short operator()(short a) { printf("short a\n"); return 0; };
            short operator()(std::string& a) volatile { printf("string a\n"); return 0; };

            void func(int a) {}
            void funcV(int a) volatile {}
        };
        class Functor1Son : public Functor1
        {

        };

        class Functor2
        {
        public:
            void operator()() {}
        };


        inline const int* const constFunc(const int* x)  //此处警告符合预期
        {
            return x;
        }

        inline const MyClass1 constFunc2(MyClass1 x)
        {
            return x;
        }

        inline void FunctionCheckerTest()
        {
            static_assert(mtype::FuncTraits<int>::value == false, "assert failed!");
            static_assert(mtype::FuncTraits<decltype(_myFunc1)>::value == true, "assert failed!");
            static_assert(mtype::FuncTraits<decltype(&MyClass3::func1)>::value == true, "assert failed!");
            static_assert(mtype::FuncTraits<decltype(&MyClass3::func3)>::value == true, "assert failed!");
            static_assert(mtype::FuncTraits<Functor1>::value == false, "assert failed!");
            static_assert(mtype::FuncTraits<volatile Functor1>::value == false, "assert failed!");
            static_assert(mtype::FuncTraits<volatile Functor2>::value == true, "assert failed!");
            static_assert(mtype::FuncTraits<const void>::value == false, "assert failed!");

            int num_args = 0;
            using type0 = mtype::FuncTraits<decltype(_myFunc1)>::ReturnType;
            using tuple_type0 = mtype::FuncTraits<decltype(_myFunc1)>::ArgsTupleType;
            num_args = mtype::FuncTraits<decltype(_myFunc1)>::num_args;
            printf("User Check! FuncTraits<decltype(_myFunc1)>::RetType(int):%s, ArgsTupleType(std::tuple<int const&, int>):%s, num_args(2):%d.\n", mtype::getTypeName<type0>(), mtype::getTypeName<tuple_type0>(), num_args);

            using type1 = mtype::FuncTraits<decltype(&MyClass3::func1)>::ReturnType;
            num_args = mtype::FuncTraits<decltype(&MyClass3::func1)>::num_args;
            printf("User Check! FuncTraits<decltype(&MyClass3::func1)>::ReturnType(int):%s, num_args(1):%d.\n", mtype::getTypeName<type1>(), num_args);

            using type1 = mtype::FuncTraits<decltype(&MyClass3::func3)>::ReturnType;
            num_args = mtype::FuncTraits<decltype(&MyClass3::func3)>::num_args;
            printf("User Check! FuncTraits<decltype(&MyClass3::func3)>::ReturnType(int):%s, num_args(1):%d.\n", mtype::getTypeName<type1>(), num_args);

            static_assert(mtype::SameTypesChecker<mtype::FuncTraits<decltype(constFunc)>::ReturnType, const int*>::value, "Check FuncTraits failed!");
            static_assert(mtype::SameTypesChecker<mtype::FuncTraits<decltype(constFunc2)>::ReturnType, const MyClass1>::value, "Check FuncTraits failed!");
        }

        class Functor3
        {
        private:
            void operator()(int a) {}
        public:
            void operator()(float a) {}
            void operator()(void* a) {}

            void func1(void* ptr) {}
        };



        inline void StdBindTraitsTest()
        {
            static_assert(mtype::StdBindTraits<int, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(_myFunc1), int, float>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(_myFunc1), int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(_myFunc1), int, int, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(_myFunc2), void>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<void, void>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::func1), int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::func1), std::vector<int>>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::func1), const MyClass3*, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::func2), const MyClass3*, int>::value == true, "assert failed!");

            static_assert(mtype::StdBindTraits<decltype(&MyClass3::func2), MyClass2*, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::func3), const volatile MyClass3&, int>::value == false, "assert failed!");  //没有默认可接收volatile类型的拷贝构造函数
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::staticFunc1), MyClass3*, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&MyClass3::staticFunc1), int>::value == true, "assert failed!");

            static_assert(mtype::StdBindTraits<Functor1, short&&>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<std::reference_wrapper<Functor1>, short&&>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<std::reference_wrapper<const Functor1>>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<std::reference_wrapper<const Functor1>, short>::value == false, "assert failed!");

            static_assert(mtype::StdBindTraits<Functor1, int&&>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<Functor1, short>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<volatile Functor1, short>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<const Functor1, short>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&Functor1Son::func), Functor1Son, int>::value == true, "assert failed!");

            static_assert(mtype::StdBindTraits<decltype(&Functor1::func), std::reference_wrapper<Functor1Son>, int>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&Functor1::func), std::reference_wrapper<Functor1Son>&, int>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&Functor1::func), std::reference_wrapper<volatile Functor1Son>, int>::value == false, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&Functor1::funcV), std::reference_wrapper<volatile Functor1Son>, int>::value == true, "assert failed!");

            static_assert(mtype::SameTypesChecker<mtype::StdBindTraits<decltype(constFunc), int*>::ReturnType, const int*>::value, "Check StdBindTraits failed!");
            static_assert(mtype::SameTypesChecker<mtype::StdBindTraits<decltype(constFunc2), MyClass1>::ReturnType, const MyClass1>::value, "Check StdBindTraits failed!");

#if !defined(__GNUC__) || (defined(__GNUC__) && _mgccMinVersion(4, 8, 1))  
            static_assert(mtype::StdBindTraits<Functor3, int>::value == false, "assert failed!");
#endif
            static_assert(mtype::StdBindTraits<Functor3, float>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<decltype(&Functor3::func1), Functor3*, int*>::value == true, "assert failed!");
            static_assert(mtype::StdBindTraits<Functor3, int*>::value == true, "assert failed!");

            Functor3 functor3;
            int a = 0;
            std::bind(&Functor3::func1, &functor3, &a)();

            using type0 = mtype::StdBindTraits<decltype(_myFunc1), int, float>::ReturnType;
            printf("User Check! StdBindTraits<decltype(_myFunc1), int, float>::ReturnType(int):%s.\n", mtype::getTypeName<type0>());

            using type1 = mtype::StdBindTraits<decltype(&MyClass3::func2), const MyClass3*, int>::ReturnType;
            printf("User Check! StdBindTraits<decltype(&MyClass3::func2), const MyClass3*, int>::ReturnType(int):%s.\n", mtype::getTypeName<type1>());
        }

        inline void check()
        {
            printf("\n--------------------check mtype start--------------------\n");
            SameTypesCheckerTest();
            InTypesCheckerTest();
            privStdBeginEndCheckerTest();
            privStdCoutCheckerTest();
            ConstructibleFromEachCheckerTest();
            EachLRvalueConstructibleChecker();
            FunctionCheckerTest();
            StdBindTraitsTest();
            printf("---------------------check mtype end---------------------\n\n");
        }
    }
#endif
}

#endif // !TYPE_HPP_MINEUTILS