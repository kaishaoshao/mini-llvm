#pragma once

#include <utility>

namespace mini_llvm {

namespace detail {

template <typename C>
concept BackInsertable = requires (C &container) {
    container.push_back(std::declval<typename C::value_type>());
};

template <typename C>
concept Insertable = requires (C &container) {
    container.insert(std::declval<typename C::value_type>());
};

template <typename C>
concept Addable = BackInsertable<C> || Insertable<C>;

template <typename C>
    requires BackInsertable<C>
void add(C &container, typename C::value_type value) {
    container.push_back(std::move(value));
}

template <typename C>
    requires Insertable<C>
void add(C &container, typename C::value_type value) {
    container.insert(std::move(value));
}

} // namespace detail

namespace assignment_ops {

template <typename C>
    requires detail::Addable<C>
C &operator+=(C &container, typename C::value_type value) {
    detail::add(container, std::move(value));
    return container;
}

template <typename C>
    requires detail::Addable<C>
C operator+=(C &&container, typename C::value_type value) {
    detail::add(container, std::move(value));
    return std::move(container);
}

template <typename C>
    requires detail::Addable<C>
C &operator,(C &container, typename C::value_type value) {
    return container += std::move(value);
}

template <typename C>
    requires detail::Addable<C>
C operator,(C &&container, typename C::value_type value) {
    return std::move(container) += std::move(value);
}

template <typename C>
    requires detail::Addable<C>
C operator+(C container, typename C::value_type value) {
    return std::move(container) += std::move(value);
}

} // namespace assignment_ops

} // namespace mini_llvm
