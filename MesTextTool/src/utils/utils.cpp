#include <iostream>
#include <windows.h>
#include <utils.hpp>
#include <chrono>

namespace utils {

	auto mstr_cvt(std::string_view str, uint32_t o_cdpg, uint32_t n_cdpg) -> std::string {
		auto&& wstr   = std::wstring(::MultiByteToWideChar(o_cdpg, 0,  str.data(), -1, 0, 0) - 1, 0);
		auto&& result = std::string (::WideCharToMultiByte(n_cdpg, 0, wstr.data(), -1, 0, 0, 0, 0) - 1, 0);
		::WideCharToMultiByte(n_cdpg, 0, wstr.data(), -1, const_cast<char*>(result.c_str()), result.size(), 0, 0);
		return result;
	}

	auto u16str_to(std::wstring_view str, uint32_t cdpg) -> std::string {
		auto&& result = std::string(::WideCharToMultiByte(cdpg, 0, str.data(), -1, 0, 0, 0, 0) - 1, 0);
		::WideCharToMultiByte(cdpg, 0, str.data(), -1, const_cast<char*>(result.c_str()), result.size(), 0, 0);
		return result;
	}

	auto utils::to_u16str(std::string_view str, uint32_t cdpg) -> std::wstring {
		auto&& result = std::wstring(::MultiByteToWideChar(cdpg, 0, str.data(), -1, 0, 0) - 1, 0);
		::MultiByteToWideChar(cdpg, 0, str.data(), -1, const_cast<wchar_t*>(result.c_str()), result.size());
		return result;
	}

	auto utils::to_u8str(std::string_view str, uint32_t cdpg) -> std::string {
		return utils::to_u8str(utils::to_u16str(str, cdpg));
	}

	auto utils::to_u8str(std::wstring_view str) -> std::string {
		auto&& result = std::string(::WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, 0, 0, 0, 0) - 1, 0);
		::WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, const_cast<char*>(result.c_str()), result.size(), 0, 0);
		return result;
	}

	auto utils::to_gbkstr(std::string_view str, uint32_t cdpg) -> std::string {
		return utils::to_gbkstr(utils::to_u16str(str, cdpg));
	}

	auto utils::to_gbkstr(std::wstring_view str) -> std::string {
		auto&& result = std::string(::WideCharToMultiByte(936, 0, str.data(), -1, 0, 0, 0, 0) - 1, 0);
		::WideCharToMultiByte(936, 0, str.data(), -1, const_cast<char*>(result.c_str()), result.size(), 0, 0);
		return result;
	}

	template<> auto utils::time_counter<std::chrono::nanoseconds, void>
	(std::function<void(void)> callbreak) -> std::chrono::nanoseconds
	{
		auto start = std::chrono::high_resolution_clock::now();
		callbreak();
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::nanoseconds result = end - start;
		return result;
	}
}