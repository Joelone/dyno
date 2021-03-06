// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef DYNO_DETAIL_DSL_HPP
#define DYNO_DETAIL_DSL_HPP

#include <boost/hana/core/tag_of.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/string.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <type_traits>
#include <utility>


namespace dyno {

// Right-hand-side of a clause in a concept that signifies a function with the
// given signature.
template <typename Signature>
constexpr boost::hana::basic_type<Signature> function{};

// Placeholder type representing the type of ref-unqualified `*this` when
// defining a clause in a concept.
struct T;

namespace detail {
  template <typename Name, typename ...Args>
  struct delayed_call {
    boost::hana::tuple<Args...> args;

    // All the constructors are private so that only `dyno::string` can
    // construct an instance of this. The intent is that we can only
    // manipulate temporaries of this type.
  private:
    template <char ...c> friend struct string;

    template <typename ...T>
    constexpr delayed_call(T&& ...t) : args{std::forward<T>(t)...} { }
    delayed_call(delayed_call const&) = default;
    delayed_call(delayed_call&&) = default;
  };

  template <char ...c>
  struct string : boost::hana::string<c...> {
    template <typename Function>
    constexpr boost::hana::pair<string, Function>
    operator=(Function f) const {
      static_assert(std::is_empty<Function>{},
        "Only stateless function objects can be used to define vtables");
      return {{}, f};
    }

    template <typename ...Args>
    constexpr auto operator()(Args&& ...args) const {
      return detail::delayed_call<string, Args&&...>{std::forward<Args>(args)...};
    }

    using hana_tag = typename boost::hana::tag_of<boost::hana::string<c...>>::type;
  };
} // end namespace detail

inline namespace literals {
  // Creates a compile-time string that can be used as the left-hand-side when
  // defining clauses or filling concept maps.
  template <typename CharT, CharT ...c>
  constexpr auto operator""_s() { return detail::string<c...>{}; }
} // end namespace literals

} // end namespace dyno

#endif // DYNO_DETAIL_DSL_HPP
