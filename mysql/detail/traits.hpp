#pragma once
#include <type_traits>
#include <cstdint>
#include <memory>
#include <functional>
#include <array>
#include <iterator>
#include <iosfwd>
#include <string>

namespace mysql_detail {
	template <int B>
	using integer = std::integral_constant<int, B>;
	template <int B>
	constexpr inline int integer_v = integer<B>::value;

	template <bool B>
	using boolean = std::integral_constant<bool, B>;
	template <bool B>
	constexpr inline bool boolean_v = boolean<B>::value;

	template <typename T>
	using neg = boolean<!T::value>;
	template <typename T>
	constexpr inline bool neg_v = neg<T>::value;

	template <typename T, typename...>
	struct all_same : std::true_type {};
	template <typename T, typename U, typename... Args>
	struct all_same<T, U, Args...> : std::integral_constant<bool, std::is_same<T, U>::value && all_same<T, Args...>::value> {};

	template <typename T, typename...>
	struct any_same : std::false_type {};
	template <typename T, typename U, typename... Args>
	struct any_same<T, U, Args...> : std::integral_constant<bool, std::is_same<T, U>::value || any_same<T, Args...>::value> {};
	template <typename T, typename... Args>
	constexpr inline bool any_same_v = any_same<T, Args...>::value;

	template <typename... Args>
	struct all : boolean<true> {};
	template <typename T, typename... Args>
	struct all<T, Args...> : std::conditional_t<T::value, all<Args...>, boolean<false>> {};
	template <typename T, typename... Args>
	constexpr inline bool all_v = all<T, Args...>::value;

	template <typename... Args>
	struct any : boolean<false> {};
	template <typename T, typename... Args>
	struct any<T, Args...> : std::conditional_t<T::value, boolean<true>, any<Args...>> {};
	template <typename T, typename... Args>
	constexpr inline bool any_v = any<T, Args...>::value;

	enum class enable_t { _ };
	constexpr const auto enabler = enable_t::_;

	template <bool value, typename T = void>
	using disable_if_t = std::enable_if_t<!value, T>;

	template <typename... Args>
	using enable = std::enable_if_t<all<Args...>::value, enable_t>;

	template <typename... Args>
	using disable = std::enable_if_t<neg<all<Args...>>::value, enable_t>;

	template <typename... Args>
	using enable_any = std::enable_if_t<any<Args...>::value, enable_t>;

	template <typename... Args>
	using disable_any = std::enable_if_t<neg<any<Args...>>::value, enable_t>;

	template <typename T, typename CharT = char>
	using is_string_literal_array_of = boolean<std::is_array_v<T> && std::is_same_v<std::remove_all_extents_t<T>, CharT>>;
	template <typename T, typename CharT = char>
	constexpr inline bool is_string_literal_array_of_v = is_string_literal_array_of<T, CharT>::value;

	template <typename T>
	using is_string_literal_array = boolean<std::is_array_v<T> && any_same_v<std::remove_all_extents_t<T>, char, char16_t, char32_t, wchar_t>>;
	template <typename T>
	constexpr inline bool is_string_literal_array_v = is_string_literal_array<T>::value;

	template <typename T, typename CharT>
	struct is_string_of : std::false_type {};
	template <typename CharT, typename CharTargetT, typename TraitsT, typename AllocT>
	struct is_string_of<std::basic_string<CharT, TraitsT, AllocT>, CharTargetT> : std::is_same<CharT, CharTargetT> {};
	template <typename T, typename CharT>
	constexpr inline bool is_string_of_v = is_string_of<T, CharT>::value;

	template <typename T, typename CharT>
	struct is_string_view_of : std::false_type {};
	template <typename CharT, typename CharTargetT, typename TraitsT>
	struct is_string_view_of<std::basic_string_view<CharT, TraitsT>, CharTargetT> : std::is_same<CharT, CharTargetT> {};
	template <typename T, typename CharT>
	constexpr inline bool is_string_view_of_v = is_string_view_of<T, CharT>::value;

	template <typename T, template <typename...> class Templ>
	struct is_specialization_of_d : std::false_type {};
	template <typename... T, template <typename...> class Templ>
	struct is_specialization_of_d<Templ<T...>, Templ> : std::true_type {};
	template <typename T, template <typename...> class Templ>
	using is_specialization_of = is_specialization_of_d<std::remove_cv_t<T>, Templ>;
	template <typename T, template <typename...> class Templ>
	inline constexpr bool is_specialization_of_v = is_specialization_of<std::remove_cv_t<T>, Templ>::value;
	template <typename T>
	using is_string_like = boolean<is_specialization_of_v<T, std::basic_string>
		|| is_specialization_of_v<T, std::basic_string_view>
		|| is_string_literal_array_v<T>>;

	template <typename T>
	constexpr inline bool is_string_like_v = is_string_like<T>::value;

	template <typename T, typename CharT = char>
	using is_string_constructible = boolean<
		is_string_literal_array_of_v<T,
		CharT> || std::is_same_v<T, const CharT*> || std::is_same_v<T, CharT> || is_string_of_v<T, CharT> || std::is_same_v<T, std::initializer_list<CharT>>
		|| is_string_view_of_v<T, CharT>
	>;
	template <typename T, typename CharT = char>
	constexpr inline bool is_string_constructible_v = is_string_constructible<T, CharT>::value;
	template <typename T>
	using is_string_like_or_constructible = boolean<is_string_like_v<T> || is_string_constructible_v<T>>;

	template <typename T, typename Char>
	using is_c_str_of = any<std::is_same<T, const Char*>, std::is_same<T, Char const* const>, std::is_same<T, Char*>, is_string_of<T, Char>,
		is_string_literal_array_of<T, Char>>;
	template <typename T, typename Char>
	constexpr inline bool is_c_str_of_v = is_c_str_of<T, Char>::value;

	template <typename T>
	using is_c_str = is_c_str_of<T, char>;
	template <typename T>
	constexpr inline bool is_c_str_v = is_c_str<T>::value;

	template <typename T>
	using unqualified = std::remove_cv<std::remove_reference_t<T>>;
	template <typename T>
	using unqualified_t = typename unqualified<T>::type;

	template <typename... Args>
	struct return_type {
		typedef std::tuple<Args...> type;
	};

	template <typename T>
	struct return_type<T> {
		typedef T type;
	};
	template <>
	struct return_type<> {
		typedef void type;
	};
	template <typename... Args>
	using return_type_t = typename return_type<Args...>::type;

	template <typename>
	struct always_true : std::true_type {};
	struct is_invokable_tester {
		template <typename Fun, typename... Args>
		static always_true<decltype(std::declval<Fun>()(std::declval<Args>()...))> test(int);
		template <typename...>
		static std::false_type test(...);
	};
	template <typename T>
	struct is_invokable;
	template <typename Fun, typename... Args>
	struct is_invokable<Fun(Args...)> : decltype(is_invokable_tester::test<Fun, Args...>(0)) {};

	template <typename...>
	struct is_error : std::false_type {};
	template <typename T>
	struct is_error<T> : std::integral_constant<bool, std::is_same<unqualified<T>, std::error_code>::value || std::is_same<unqualified<T>, std::system_error>::value> {};
}

using namespace mysql_detail;