#pragma once

#include <utility>
#include <type_traits>

namespace util {

    template<typename...>
    struct type_list {
        using identity = type_list<>;
        using reverse = type_list<>;
        using rest = type_list<>;
        template<typename U>
        using append = type_list<U>;

        template<template<typename...>typename A>
        using apply = A<>;

        static constexpr auto size = 0;
    };
    template<typename First, typename ...Rest>
    struct type_list<First, Rest...> {
        using identity = type_list<First, Rest...>;

        using first = First;
        using rest = type_list<Rest...>;

        using reverse = typename rest::reverse::template append<First>;

        using except_last = typename reverse::rest::reverse;
        using last = typename reverse::first;

        template<typename T>
        using append = type_list<First, Rest..., T>;

        template<template<typename...>typename A>
        using apply = A<First, Rest...>;

        static constexpr auto size = sizeof...(Rest) + 1;
    };
    template<typename T>
    struct type_list<T> {
        using identity = type_list<T>;

        using first = T;
        using rest = type_list<>;

        using reverse = identity;

        using except_last = rest;
        using last = first;

        template<typename U>
        using append = type_list<T, U>;

        template<template<typename...>typename A>
        using apply = A<T>;

        static constexpr auto size = 1;
    };

    template<typename T, typename...>
    using alias_t = T;

}