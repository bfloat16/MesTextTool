#pragma once

namespace utils {

	class file {
		FILE* fptr{};
	public:
		inline file(): fptr(nullptr) {}

		inline file(std::string_view name, std::string_view mode) :
			fptr(std::fopen(name.data(), mode.data())) {
		}

		inline ~file() {
			static_cast<void>(this->close());
		}

		inline auto seek(long offset, int origin) -> int {
			return std::fseek(this->fptr, offset, origin);
		}

		inline auto tell() -> long {
			return std::ftell(this->fptr);
		}

		inline auto rewind() -> file& {
			std::rewind(this->fptr);
			return *this;
		}

		inline auto read(void* buffer, size_t elementSize, size_t elementCount) -> size_t {
			return std::fread(buffer, elementSize, elementCount, this->fptr);
		}

		inline auto gets(char* buffer, int maxCount) -> char* {
			return std::fgets(buffer, maxCount, this->fptr);
		}

		inline auto write(void const* buffer, size_t elementSize, size_t elementCount) -> size_t {
			return std::fwrite(buffer, elementSize, elementCount, this->fptr);
		}

		inline auto close() -> int {
			int result{ errno };
			if (this->fptr) {
				result = std::fclose(this->fptr);
			}
			this->fptr = {};
			return result;
		}

		inline auto get() -> FILE* {
			return this->fptr;
		}

		inline auto is_open() -> bool {
			return this->fptr != nullptr;
		}

		inline auto open(const char* name, char const* mode) -> file& {
			if (this->fptr != nullptr) {
				this->close();
			}
			this->fptr = std::fopen(name, mode);
			return *this;
		}
	};

	auto extension_check(std::string_view file, std::string_view ext) -> bool;
}