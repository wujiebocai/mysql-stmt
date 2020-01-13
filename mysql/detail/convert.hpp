#pragma once

#include <string>

#include <detail/traits.hpp>

namespace mysql_detail {
	template<class T>
	struct convert {
		using ttype = T;
		template<class ...Args>
		inline static void conv(Args&&... args) {
		}
	};

	template<>
	struct convert<bool> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static bool conv(Args&&... args) { return (!(std::stoi(std::forward<Args>(args)...) == 0)); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static bool conv(T&& t) { return (!(static_cast<int>(std::forward<T>(t))) == 0); }
	};

	template<>
	struct convert<char> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static char conv(Args&&... args) { return static_cast<char>(std::stoi(std::forward<Args>(args)...)); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static char conv(T&& t) { return static_cast<char>(std::forward<T>(t)); }
	};

	template<>
	struct convert<unsigned char> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static unsigned char conv(Args&&... args) { return static_cast<unsigned char>(std::stoul(std::forward<Args>(args)...)); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static unsigned char conv(T&& t) { return static_cast<unsigned char>(std::forward<T>(t)); }
	};

	template<>
	struct convert<short> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static short conv(Args&&... args) { return static_cast<short>(std::stoi(std::forward<Args>(args)...)); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static short conv(T&& t) { return static_cast<short>(std::forward<T>(t)); }
	};

	template<>
	struct convert<unsigned short> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static unsigned short conv(Args&&... args) { return static_cast<unsigned short>(std::stoul(std::forward<Args>(args)...)); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static unsigned short conv(T&& t) { return static_cast<unsigned short>(std::forward<T>(t)); }
	};

	template<>
	struct convert<int> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static int conv(Args&&... args) { return std::stoi(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static int conv(T&& t) { return static_cast<int>(std::forward<T>(t)); }
	};

	template<>
	struct convert<unsigned int> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static unsigned int conv(Args&&... args) { return static_cast<unsigned int>(std::stoul(std::forward<Args>(args)...)); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static unsigned int conv(T&& t) { return static_cast<unsigned int>(std::forward<T>(t)); }
	};

	template<>
	struct convert<long> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static long conv(Args&&... args) { return std::stol(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static long conv(T&& t) { return static_cast<long>(std::forward<T>(t)); }
	};

	template<>
	struct convert<unsigned long> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static unsigned long conv(Args&&... args) { return std::stoul(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static unsigned long conv(T&& t) { return static_cast<unsigned long>(std::forward<T>(t)); }
	};

	template<>
	struct convert<long long> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static long long conv(Args&&... args) { return std::stoll(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static long long conv(T&& t) { return static_cast<long long>(std::forward<T>(t)); }
	};

	template<>
	struct convert<unsigned long long> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static unsigned long long conv(Args&&... args) { return std::stoull(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static unsigned long long conv(T&& t) { return static_cast<unsigned long long>(std::forward<T>(t)); }
	};

	template<>
	struct convert<float> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static float conv(Args&&... args) { return std::stof(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static float conv(T&& t) { return static_cast<float>(std::forward<T>(t)); }
	};

	template<>
	struct convert<double> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static double conv(Args&&... args) { return std::stod(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static double conv(T&& t) { return static_cast<double>(std::forward<T>(t)); }
	};

	template<>
	struct convert<long double> {
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static long double conv(Args&&... args) { return std::stold(std::forward<Args>(args)...); }
		template<class T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<T>>>> = mysql_detail::enabler>
		inline static long double conv(T&& t) { return static_cast<long double>(std::forward<T>(t)); }
	};

	template<class CharT, class Traits, class Allocator>
	struct convert<std::basic_string<CharT, Traits, Allocator> > {
		template<class ...Args>
		inline static std::basic_string<CharT, Traits, Allocator> conv(Args&&... args) {
			return std::basic_string<CharT, Traits, Allocator>(std::forward<Args>(args)...);
		}
	};
	
	template<>
	struct convert<std::string> {
		template<class ...Args, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<std::decay_t<Args>>>...> = mysql_detail::enabler>
		inline static std::string conv(Args&&... args) { 
			return std::to_string(std::forward<Args>(args)...); 
			//return "";
		}
		template<class ...Args, mysql_detail::enable<mysql_detail::is_string_constructible<std::decay_t<Args>>...> = mysql_detail::enabler>
		inline static std::string conv(Args&&... args) {
			return std::string(std::forward<Args>(args)...);
		}
	};

	template<class CharT, class Traits>
	struct convert<std::basic_string_view<CharT, Traits> > {
		template<class ...Args>
		inline static std::basic_string_view<CharT, Traits> conv(Args&&... args) {
			return std::basic_string_view<CharT, Traits>(convert<std::string>::conv(std::forward<Args>(args)...));
		}
	};

	template<>
	struct convert<std::initializer_list<char>> {
		template<class ...Args>
		inline static std::string conv(Args&&... args) {
			return std::string(convert<std::string>::conv(std::forward<Args>(args)...));
		}
	};

	template<>
	struct convert<const char*> {
		template<class ...Args>
		inline static const char* conv(Args&&... args) {
			return std::string(convert<std::string>::conv(std::forward<Args>(args)...)).c_str();
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T, class ...Args>
	inline decltype(auto) conv_get(Args&&... args) {
		return convert<unqualified_t<T>>::conv(std::forward<Args>(args)...);
	}
}  // namespace mysql_detail
using namespace mysql_detail;



