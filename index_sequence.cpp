#include <tuple>

template <typename T1, typename T2>
struct tuple_cat_helper;

template <typename... T1, typename... T2>
struct tuple_cat_helper<std::tuple<T1...>, std::tuple<T2...>> {
    using type = std::tuple<T1..., T2...>;
};

template <typename T1, typename T2>
using tuple_cat_t = tuple_cat_helper::type;

template <typename Tuple, std::size_t... Ints>
auto select_tuple_from_tuple(Tuple&& tuple, std::index_sequence<Ints...>)
{
    return std::make_tuple(std::get<Ints>(std::forward<Tuple>(tuple))...);
}

template <typename Tuple>
auto remove_last(Tuple&& tuple)
{
    constexpr size_t size = std::tuple_size_v<Tuple>;
    using indices = std::make_index_sequence<size - 1>;
    return select_tuple_from_tuple(std::forward<Tuple>(tuple), indices{});
}

template <std::size_t N, typename Seq>
struct offset_sequence;

template <std::size_t N, std::size_t... Ints>
struct offset_sequence<N, std::index_sequence<Ints...>> {
    using type = std::index_sequence<Ints + N...>;
};

template <std::size_t N, typename Seq>
using offset_sequence_t = typename offset_sequence<N, Seq>::type;


template <typename Tuple>
auto remove_first(Tuple&& tuple) {
    constexpr auto size = std::tuple_size_v<Tuple>;
    using indices = offset_sequence<1, std::make_index_sequence<size - 1>>;
    return select_tuple_from_tuple(std::forward<Tuple>(tuple), indices{});
}

template <std::size_t N, typename Tuple>
auto remove_N(Tuple&& tuple)
{
    constexpr auto size = std::tuple_size_v<Tuple>;
    using first_half = std::make_index_sequence<N>;
    using second_half = offset_sequence<N + 1, std::make_index_sequence<size - N - 1>>;
    
    return std::tuple_cat(
        select_tuple_from_tuple(std::forward<Tuple>(tuple), first_half{});
        select_tuple_from_tuple(std::forward<Tuple>(tuple), second_half{});
    );
}


