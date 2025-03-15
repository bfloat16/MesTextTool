#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <file.hpp>

namespace utils {

    auto extension_check(std::string_view file, std::string_view ext) -> bool {
		auto transform = [](std::string_view str) -> std::string {
			auto first = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
				return !(ch == ' ' || ch == '\n' || ch == '\r');
				});
			auto last = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
				return !(ch == ' ' || ch == '\n' || ch == '\r');
				}).base();
			std::string result(first, last);
			for (char& c : result) { c = std::tolower(c); }
			return result;
			};
		size_t find_pos = file.find_last_of(".");
		if (find_pos == std::string_view::npos) {
			return false;
		}
		bool result = transform(file.data() + find_pos) == transform(ext);
		return result;
    }
}