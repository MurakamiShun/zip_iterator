#pragma once
#include <iterator>
#include <tuple>
#include <utility>
/*
 * zip_iterator C++14: https://github.com/MurakamiShun/zip_iterator
 * Copyright (c) 2022 MurakamiShun
 *
 * Released under the MIT License.
 */

#if __cplusplus < 201402L && _MSVC_LANG < 201402L
template<typename... Iterators>
#error zip_iterator needs C++14 or later.
#endif

namespace tuple_utils {
    template<typename IterList>
    struct zip_iterator;

    template<typename ZipIterator>
    struct zip_iterator_reference_tuple;

    template<typename... Iterators>
    struct zip_iterator<std::tuple<Iterators...>> {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::tuple<typename Iterators::value_type...>;
        using reference = zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>>;
        using pointer = std::tuple<typename Iterators::value_type*...>;
        using iterator_category = std::random_access_iterator_tag;
        using iterator_tuple = std::tuple<Iterators...>;
        static constexpr auto iterator_tuple_size_v = sizeof...(Iterators);
    private:
        iterator_tuple iters;
    public:
        explicit zip_iterator(iterator_tuple&);
        explicit zip_iterator(const iterator_tuple&);
        explicit zip_iterator(iterator_tuple&&);

        difference_type operator-(const zip_iterator&) const;
        zip_iterator operator-(difference_type) const;
        zip_iterator operator+(difference_type) const;

        zip_iterator& operator++();
        zip_iterator& operator--();

        bool operator==(const zip_iterator&) const;
        bool operator!=(const zip_iterator&) const;
        bool operator<(const zip_iterator&) const;

        reference operator*();
        reference operator*() const;
    };

#if __cplusplus >= 201703L
    template<typename... Iterators>
    zip_iterator(std::tuple<Iterators...>)->zip_iterator<std::tuple<Iterators...>>;
#endif

    template<typename... Iterators>
    zip_iterator<std::tuple<Iterators...>> make_zip_iterator(Iterators&&...);

    template<typename... Iterators>
    struct zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>> : std::tuple<typename Iterators::reference...> {
        using std::tuple<typename Iterators::reference...>::tuple;
        using std::tuple<typename Iterators::reference...>::operator=;
        using tuple_type = std::tuple<typename Iterators::reference...>;
        explicit zip_iterator_reference_tuple(std::tuple<typename Iterators::reference...>&& t) : std::tuple<typename Iterators::reference...>(std::move(t)) {}
        zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>>& operator=(const std::tuple<typename Iterators::value_type...>&);
    };

    template<typename... Iterators>
    void swap(zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>>, zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>>); // Customization Point

    // --- Implementations --- //

    namespace detail {
        template<typename IterTuple, std::size_t... I>
        auto make_reference_tuple_helper(IterTuple& iters, std::index_sequence<I...>) {
            return std::forward_as_tuple((*std::get<I>(iters))...);
        }
        template<typename IterTuple>
        void zip_iterator_swap_helper(IterTuple&, IterTuple&, std::index_sequence<>) {}
        template<typename IterTuple, std::size_t N, std::size_t... I>
        void zip_iterator_swap_helper(IterTuple& a, IterTuple& b, std::index_sequence<N, I...>) {
            using std::swap;
            swap(std::get<N>(a), std::get<N>(b));
            zip_iterator_swap_helper(a, b, std::index_sequence<I...>{});
        }

        template<typename F, typename T>
        void apply_each(F&&, T&&, std::index_sequence<>) {}
        template<typename F, typename T, std::size_t N, std::size_t... I>
        void apply_each(F&& f, T& t, std::index_sequence<N, I...>) {
            f(std::get<N>(t));
            apply_each(std::forward<F>(f), t, std::index_sequence<I...>{});
        }
        template<typename F, typename T, std::size_t N, std::size_t... I>
        void apply_each(F&& f, T&& t, std::index_sequence<N, I...>) {
            f(std::move(std::get<N>(t)));
            apply_each(std::forward<F>(f), std::move(t), std::index_sequence<I...>{});
        }
        template<typename F, typename Tuple>
        void apply_each(F&& f, Tuple&& t) {
            apply_each(std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size<std::remove_cv_t<std::remove_reference_t<Tuple>>>::value>{});
        }

        template<typename Ref, typename Val>
        void zip_iterator_reference_tuple_assign_helper(Ref&, const Val&, std::index_sequence<>) {}
        template<typename Ref, typename Val, std::size_t N, std::size_t... I>
        void zip_iterator_reference_tuple_assign_helper(Ref& ref, const Val& val, std::index_sequence<N, I...>) {
            std::get<N>(ref) = std::get<N>(val);
            zip_iterator_reference_tuple_assign_helper(ref, val, std::index_sequence<I...>{});
        }
    }

    template<typename... Iterators>
    zip_iterator<std::tuple<Iterators...>>::zip_iterator(iterator_tuple& it) :
        iters(it) {
    }

    template<class... Iterators>
    zip_iterator<std::tuple<Iterators...>>::zip_iterator(const iterator_tuple& it) :
        iters(it) {
    }
    template<class... Iterators>
    zip_iterator<std::tuple<Iterators...>>::zip_iterator(iterator_tuple&& it) :
        iters(std::move(it)) {
    }

    template<class... Iterators>
    typename zip_iterator<std::tuple<Iterators...>>::difference_type zip_iterator<std::tuple<Iterators...>>::operator-(const zip_iterator& z_it) const {
        return std::get<0>(iters) - std::get<0>(z_it.iters);
    }
    template<class... Iterators>
    zip_iterator<std::tuple<Iterators...>> zip_iterator<std::tuple<Iterators...>>::operator-(difference_type d) const {
        zip_iterator<std::tuple<Iterators...>> rtn(*this);
        detail::apply_each([d](auto& it) { it -= d; }, rtn.iters);
        return rtn;
    }
    template<class... Iterators>
    zip_iterator<std::tuple<Iterators...>> zip_iterator<std::tuple<Iterators...>>::operator+(difference_type d) const {
        zip_iterator<std::tuple<Iterators...>> rtn(*this);
        detail::apply_each([d](auto& it) { it += d; }, rtn.iters);
        return rtn;
    }
    template<class... Iterators>
    zip_iterator<std::tuple<Iterators...>>& zip_iterator<std::tuple<Iterators...>>::operator++() {
        detail::apply_each([](auto& it) { ++it; }, iters);
        return *this;
    }
    template<class... Iterators>
    zip_iterator<std::tuple<Iterators...>>& zip_iterator<std::tuple<Iterators...>>::operator--() {
        detail::apply_each([](auto& it) { --it; }, iters);
        return *this;
    }
    template<class... Iterators>
    bool zip_iterator<std::tuple<Iterators...>>::operator==(const zip_iterator& zip_iter) const {
        return iters == zip_iter.iters;
    }
    template<class... Iterators>
    bool zip_iterator<std::tuple<Iterators...>>::operator!=(const zip_iterator& zip_iter) const {
        return iters != zip_iter.iters;
    }
    template<class... Iterators>
    bool zip_iterator<std::tuple<Iterators...>>::operator<(const zip_iterator& zip_iter) const {
        return iters < zip_iter.iters;
    }
    template<class... Iterators>
    typename zip_iterator<std::tuple<Iterators...>>::reference zip_iterator<std::tuple<Iterators...>>::operator*() {
        return reference(detail::make_reference_tuple_helper(iters, std::make_index_sequence<sizeof...(Iterators)>{}));
    }
    template<class... Iterators>
    typename zip_iterator<std::tuple<Iterators...>>::reference zip_iterator<std::tuple<Iterators...>>::operator*() const {
        return reference(detail::make_reference_tuple_helper(iters, std::make_index_sequence<sizeof...(Iterators)>{}));
    }

    template<typename... Iterators>
    zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>>& zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>>::operator=(const std::tuple<typename Iterators::value_type...>& t) {
        detail::zip_iterator_reference_tuple_assign_helper(*this, t, std::make_index_sequence<sizeof...(Iterators)>{});
        return *this;
    }


    template<typename... Iterators>
    zip_iterator<std::tuple<Iterators...>> make_zip_iterator(Iterators&&... iters) {
        return zip_iterator<std::tuple<Iterators...>>(std::make_tuple(iters...));
    }
    template<typename... Iterators>
    void swap(zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>> a, zip_iterator_reference_tuple<zip_iterator<std::tuple<Iterators...>>> b) { // Customization Point
        using tuple_type = typename decltype(a)::tuple_type;
        auto ref_tup_a = static_cast<tuple_type>(a);
        auto ref_tup_b = static_cast<tuple_type>(b);
        detail::zip_iterator_swap_helper(ref_tup_a, ref_tup_b, std::make_index_sequence<zip_iterator<std::tuple<Iterators...>>::iterator_tuple_size_v>{});
    }
} // namespace tuple_utils
