#ifndef UTILITY_H_NSE
#define UTILITY_H_NSE

//! \brief utility functions

#include <ndb/fwd.hpp>
#include <utility>
#include <ndb/cx_error.hpp>
#include <ndb/expression/code.hpp>

namespace ndb
{
////////////////////////////////////////////////////////////////////////////////
////////////////////////             GENERIC            ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    // compile time str _len
    constexpr size_t cx_str_len(const char* str)
    {
        return *str ? 1 + cx_str_len(str + 1) : 0;
    }

    // is_tpl_of
    template<template<class...> class, class>
    struct is_tpl_of : std::false_type {};

    template<template<class...> class T, class... Ts>
    struct is_tpl_of<T, T<Ts...>> : std::true_type {};

    // index_of
    template<class T, class Container>
    struct index_of;

    template<class T, template<class...> class Container>
    struct index_of<T, Container<>>
    {
        constexpr static auto value = ncx_error(index_of, cx_err_type_not_found, T, Container);
    };

    template<class T, template<class...> class Container, class... Ts>
    struct index_of<T, Container<T, Ts...>>
    {
        constexpr static auto value = 0;
    };

    template<class T, template<class...> class Container, class T_test, class... Ts>
    struct index_of<T, Container<T_test, Ts...>>
    {
        constexpr static auto value = 1 + index_of<T, Container<Ts...>>::value;
    };

////////////////////////////////////////////////////////////////////////////////
////////////////////////              NDB               ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    template<class T>
    static constexpr bool is_table = std::is_base_of<ndb::table_base, T>::value;

    template<class T>
    static constexpr bool is_field =  std::is_base_of<ndb::field_base, T>::value;

    template<class T>
    static constexpr bool is_field_entity = std::is_base_of<ndb::table_base, typename T::value_type>::value;

    template<class T>
    static constexpr bool is_field_entity_vector = is_field_entity<T> && (T{}.detail_.size == 0);

    template<class T>
    static constexpr bool is_option = std::is_base_of<ndb::option_base, T>::value;

    template<class T>
    static constexpr bool is_expression = std::is_base_of<ndb::expression_base, T>::value;

    template<class Field>
    constexpr unsigned char field_id = index_of<std::decay_t<Field>, typename Field::table::Detail_::entity>::value;

    template<class Table>
    constexpr unsigned char table_id = index_of<std::decay_t<Table>, typename Table::model::Detail_::entity>::value;

    template<class Database>
    constexpr unsigned char database_id = index_of<std::decay_t<Database>, typename Database::group::Detail_::entity>::value;

/////////////////////////////////////////////////////////////////////////////////
////////////////////////           EXPRESSION            ////////////////////////
/////////////////////////////////////////////////////////////////////////////////
    template<class Expr>
    constexpr bool expr_is_value = Expr::type == expr_type_code::value;

    template<class Expr>
    constexpr bool expr_is_field = Expr::type == expr_type_code::field;

    template<class Expr>
    constexpr bool expr_is_table = Expr::type == expr_type_code::table;

    template<class Expr>
    constexpr bool expr_is_scalar = expr_is_field<Expr> || expr_is_value<Expr> || expr_is_table<Expr>;

    template<class Expr, expr_clause_code Clause>
    constexpr bool expr_has_clause = static_cast<bool>(Expr::clause() & Clause);

////////////////////////////////////////////////////////////////////////////////
////////////////////////            FOR_EACH            ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        // call f for each type
        template<class... Ts, std::size_t... Ns, class F>
        void for_each_impl(std::index_sequence<Ns...>, F&& f)
        {
            using expand = int[];
            (void)expand{1, (std::forward<F>(f)(std::integral_constant<std::size_t, Ns>{}, Ts{}), 0)...};
        }

        // call f for each table or field
        template<class Entity, std::size_t... Ns, class F>
        void for_each_entity_impl(std::index_sequence<Ns...>&&, F&& f)
        {
            using expand = int[];
            (void)expand{1, ((void)std::forward<F>(f)(std::integral_constant<std::size_t, Ns>{}, typename Entity::template item_at<Ns>{}), 0)...};
        }

        // call f for each arg
        template<std::size_t... N, class F, class... Ts>
        void for_each_impl(std::index_sequence<N...>, F&& f, Ts&&... args)
        {
            using expand = int[];
            (void)expand{((void)std::forward<F>(f)(std::integral_constant<std::size_t, N>{}, std::forward<Ts>(args)), 0)...};
        }
    } // detail

    // for each on pack type
    template<class... Ts, class F>
    void for_each(F&& f)
    {
        detail::for_each_impl<Ts...>(std::index_sequence_for<Ts...>{}, std::forward<F>(f));
    }

    // for_each on model entity
    template<class DB_Entity, class F>
    void for_each_entity(DB_Entity&& e, F&& f)
    {
        using Entity = typename std::decay_t<DB_Entity>::Detail_::entity;
        using Ns = std::make_index_sequence<Entity::count()>;
        detail::for_each_entity_impl<Entity>(Ns{}, std::forward<F>(f));
    }

    // for each on model entity
    template<class DB_Entity, class F>
    void for_each_entity(F&& f)
    {
        for_each_entity(DB_Entity{}, std::forward<F>(f));
    }

    // for each on arguments
    template<class F, class... Ts>
    void for_each(F&& f, Ts&&... args)
    {
        detail::for_each_impl(std::index_sequence_for<Ts...>{}, std::forward<F>(f), std::forward<Ts>(args)...);
    }
} // ndb

#endif // UTILITY_H_NSE
