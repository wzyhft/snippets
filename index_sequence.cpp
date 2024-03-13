#include <tuple>

template <typename T1, typename T2>
struct tuple_cat_helper;

template <typename... T1, typename... T2>
struct tuple_cat_helper<std::tuple<T1...>, std::tuple<T2...>> {
    using type = std::tuple<T1..., T2...>;
};

template <typename T1, typename T2>
using tuple_cat_t = tuple_cat_helper::type;

