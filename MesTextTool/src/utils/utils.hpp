#pragma once
#ifndef _FUNCTIONAL_
#include <functional>
#endif
#ifndef _CHRONO_
#include <chrono>
#endif

namespace utils {

	auto mstr_cvt(std::string_view str, uint32_t o_cdpg, uint32_t n_cdpg) -> std::string;

	auto u16str_to(std::wstring_view str, uint32_t cdpg) -> std::string;

	auto to_u16str(std::string_view str, uint32_t cdpg) -> std::wstring;

	auto to_u8str(std::string_view str, uint32_t cdpg) -> std::string;

	auto to_u8str(std::wstring_view str) -> std::string;

	auto to_gbkstr(std::wstring_view str) -> std::string;

	auto to_gbkstr(std::string_view str, uint32_t cdpg) -> std::string;

	template<typename R = std::chrono::nanoseconds, typename Period = void>
	auto time_counter(std::function<void(void)> callbreak) -> R;

	template<> auto utils::time_counter<std::chrono::nanoseconds, void>
	(std::function<void(void)> callbreak)->std::chrono::nanoseconds;

	template<typename R, typename Period>
	auto time_counter(std::function<void(void)> callbreak) -> R {
		if constexpr (std::is_same<Period, void>::value) {
			auto result = static_cast<std::chrono::duration<R>>(
				time_counter<std::chrono::nanoseconds, void>(callbreak)
				);
			return result.count();
		}
		else {
			auto result = static_cast<std::chrono::duration<R, Period>>(
				time_counter<std::chrono::nanoseconds, void>(callbreak)
				);
			return result.count();
		}
	}
}