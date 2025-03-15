#include <iostream>
#include <charconv>
#include <xstr.hpp>


namespace xstr {

	//template<>
	//auto xstr::to_integer<int64_t>(std::string_view str) -> std::optional<int64_t>
	//{
	//	int64_t number{};
	//	auto result = std::from_chars(str.data(), str.data() + str.size(), number);
	//	if (result.ec == std::errc()) 
	//	{
	//		return { number };
	//	}
	//	else 
	//	{
	//		return { std::nullopt };
	//	}
	//}
	//
	//template<>
	//auto xstr::to_integer<int64_t>(std::string_view str) -> std::optional<uint64_t>
	//{
	//	uint64_t number{};
	//	auto result = std::from_chars(str.data(), str.data() + str.size(), number);
	//	if (result.ec == std::errc())
	//	{
	//		return { number };
	//	}
	//	else
	//	{
	//		return { std::nullopt };
	//	}
	//}


}
