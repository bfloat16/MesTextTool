#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <mes.hpp>
#include <utils.hpp>
#include <file.hpp>
#include <filesystem>
#include <string_buffer.hpp>
#include <console.hpp>

namespace mes {

	auto multi_script_helper::config::read(std::string_view path) -> config 
	{
		config result{};
		multi_script_helper::config::read(path, result);
		return result;
	}

	auto multi_script_helper::config::read(std::string_view path, config& result) -> void 
	{
		if (path.empty()) 
		{
			throw std::exception
			{ 
				"[multi_script_helper::config::read] Path is empty!" 
			};
		}

		utils::file file(path.data(), "r");
		if (!file.is_open()) 
		{
			throw std::exception
			{ 
				"[multi_script_helper::config::read] Failed to open file!" 
			};
		}

		file.seek(0, SEEK_END);
		utils::string::buffer buffer(file.tell());
		file.rewind();

		int flag{ -1 };
		while (file.gets(buffer.data(), buffer.size()) != nullptr)
		{

			buffer.recount();
			if (buffer.trim().count() == 0) 
			{
				continue;
			}

			if (buffer.contrast(config::path, 0, sizeof(config::path) - 1)) 
			{
				flag = 1;
			}
			else if (buffer.contrast(config::cdpg, 0, sizeof(config::cdpg) - 1))
			{
				flag = 2;
			}
			else if (buffer.contrast(config::tmin, 0, sizeof(config::tmin) - 1)) 
			{
				flag = 3;
			}
			else if (buffer.contrast(config::tmax, 0, sizeof(config::tmax) - 1))
			{
				flag = 4;
			}
			else if (buffer.contrast(config::bfrp, 0, sizeof(config::bfrp) - 1)) 
			{
				flag = 5;
			}
			else if (buffer.contrast(config::atrp, 0, sizeof(config::atrp) - 1)) 
			{
				flag = 6;
			}
			else if (flag == 1) 
			{
				result.InputPath.assign(buffer.data());
				if (!result.InputPath.empty()) 
				{
					flag = 0;
				}
			}
			else if (flag > 1 && flag < 5) 
			{
				int value{ NULL };
				int matched = std::sscanf(buffer.data(), "%d", &value);
				if (matched == 1) 
				{
					if (flag == 2) 
					{
						result.UseCodePage = value;
					}
					else if (flag == 3) 
					{
						result.MinLength = value;
					}
					else
					{
						result.MaxLength = value;
					}
					flag = 0;
				}
			}
			else if (flag == 5 || flag == 6) 
			{
				if (!buffer.starts_with("[") || !buffer.ends_with("]")) 
				{
					continue;
				}
				size_t offset = buffer.rfind("]:[");
				if (offset == std::string_view::npos || offset == 1)
				{
					continue;
				}
				std::string key = buffer.substr(1, offset - 1);
				if (key.empty()) { continue; }
				size_t v_count = buffer.count() - (offset + 4);
				std::string value = buffer.substr(offset + 3, v_count);
				auto pair = std::make_pair(key, value);
				if (flag == 5)
				{
					result.Before.push_back(pair);
				}
				else 
				{
					result.After.push_back(pair);
				}
			}
		}
	}

	multi_script_helper::text_formater::text_formater(config& config) : m_Config(config)
	{
		for (auto& [key, value] : config.After)
		{
			key   = { utils::mstr_cvt(key,   CP_UTF8, this->m_Config.UseCodePage) };
			value = { utils::mstr_cvt(value, CP_UTF8, this->m_Config.UseCodePage) };
		}
	}

	auto multi_script_helper::text_formater::is_first_char_forbidden(wchar_t chr) -> bool
	{
		size_t find { std::wstring_view{ L"。、？’”，！～】；：）」』 　" }.find(chr) };
		return { find != std::wstring_view::npos };
	}

	auto multi_script_helper::text_formater::is_last_char_forbidden(wchar_t chr) -> bool
	{
		size_t find { std::wstring_view{ L"（(「『【‘“…" }.find(chr) };
		return { find != std::wstring_view::npos };
	}

	auto mes::multi_script_helper::text_formater::is_talking(wchar_t beg, wchar_t end) -> bool
	{

		if (beg == L'「' && end == L'」')
		{
			return { true };
		}

		if (beg == L'『' && end == L'』') 
		{
			return { true };
		}

		if (beg == L'“' && end == L'“')
		{
			return { true };
		}

		return { false };
	}

	auto multi_script_helper::text_formater::is_half_width(wchar_t wchar) -> bool
	{
		return { static_cast<bool>(wchar >= 0x0000 && wchar <= 0x007F) };
	}

	auto multi_script_helper::text_formater::format(std::string& text) -> void
	{
		utils::string::buffer buffer{ text };
		for (const auto& [key, value] : this->m_Config.Before) {
			buffer.replace(key, value);
		}

		buffer.replace(u8"/", u8"／");
		buffer.replace(u8"{", u8"｛");
		buffer.replace(u8"}", u8"｝");

		if (buffer.starts_with("@::"))
		{
			buffer.remove("@::", 0, 1);
			buffer.replace("\\n", "\n");
			buffer.convert_encoding(CP_UTF8, this->m_Config.UseCodePage);
		}
		else if ((this->m_Config.MaxLength != -1 || this->m_Config.MinLength != -1)
			&& this->m_Config.MaxLength >= this->m_Config.MinLength)
		{
			buffer.remove(u8"\\n　").remove(u8"\\n");

			auto&& wtext = buffer.wstring(CP_UTF8);

			auto first = wtext.begin();
			auto last = wtext.end() - 1;

			bool is_talking = this->is_talking(*first, *last);

			size_t length = wtext.size();

			if (length > size_t(this->m_Config.MinLength))
			{

				float char_count = 0.0f;
				utils::wstring::buffer n_text{ length + 0x10 };

				for (size_t index{ 0 }; index < length;)
				{
					const wchar_t& wchar = wtext[index];

					if (char_count >= static_cast<float>(this->m_Config.MinLength))
					{
						if (!this->is_first_char_forbidden(wchar))
						{
							n_text.write({ is_talking ? L"\n　" : L"\n" });
							char_count = { is_talking ? 1.0f : 0.0f     };
						}
					}

					if (wchar == L'｛')
					{
						auto finish = [&]() -> bool
							{
								size_t spt = wtext.find(L'／', index);
								if (std::wstring::npos == spt)
								{
									return false;
								}

								size_t end = wtext.find(L'｝', spt);
								if (std::wstring::npos == end)
								{
									return false;
								}

								size_t count = end - spt - 1;
								std::wstring_view sub{ wtext.data() + spt + 1, count };
								{
									float length = 0.0f;
									for (const wchar_t& chr : sub)
									{
										length += this->is_half_width(chr) ? 0.5f : 1.0f;
									}

									char_count = char_count + length;
									if (char_count >= this->m_Config.MaxLength)
									{
										n_text.write(is_talking ? L"\n　" : L"\n");
										char_count = length;
									}
								}

								auto&& first{ wtext.begin() + index };
								auto&& last{ first + (end - index + 1) };
								std::wstring_view text{ first,  last };
								n_text.write(text);

								index = end + 1;
								return true;
							}();

						if (finish) { continue; };
					}

					if (this->is_half_width(wchar))
					{
						if (wchar != L' ' && index < length)
						{
							int count{ 1 };
							while (index + count < length)
							{
								const wchar_t& chr = wtext[index + count];
								if (this->is_half_width(chr) && chr != L' ')
								{
									count++;
									continue;
								}
								break;
							}
							auto&& first{ wtext.begin() + index };
							auto&& last{ first + count };
							std::wstring_view text{ first,  last };


							size_t find = text.find(L'@');
							if (find == std::wstring_view::npos)
							{
								auto length = static_cast<float>(count) / 2.0f;
								char_count = char_count + length;
								if (char_count >= this->m_Config.MaxLength)
								{
									n_text.write(is_talking ? L"\n　" : L"\n");
									char_count = length;
								}
							}

							n_text.write(text);
							index += count;
							continue;
						}
						else
						{
							char_count += 0.5f;
						}
					}
					else
					{
						char_count += 1.0f;
					}

					if (char_count >= static_cast<float>(this->m_Config.MinLength))
					{
						if (this->is_last_char_forbidden(wchar))
						{
							n_text.write({ is_talking ? L"\n　" : L"\n" });
							char_count = { is_talking ? 2.0f : 1.0f     };
						}
					}

					n_text.write(wchar);
					index++;
				}

				buffer = n_text.string_buffer(this->m_Config.UseCodePage);
			}
			else
			{
				buffer.convert_encoding(CP_UTF8, this->m_Config.UseCodePage);
			}
		}
		else 
		{
			buffer.convert_encoding(CP_UTF8, this->m_Config.UseCodePage);
		}

		for (const auto& [key, value] : this->m_Config.After) 
		{
			buffer.replace(key, value);
		}
		text = buffer.string();
	}

	auto multi_script_helper::read_text(std::string_view path, std::vector<script_helper::text>& result) -> bool {

		if (path.empty()) {
			return false;
		}

		utils::file file(path, "r");
		if (!file.is_open()) {
			return false;
		}

		file.rewind();
		result.clear();
		utils::string::buffer buffer(1024);

		int32_t offset{ -1 };
		while (file.gets(buffer.data(), buffer.size()) != nullptr)
		{

			buffer.recount();
			if (buffer.trim().count() == 0) 
			{
				continue;
			}

			if (buffer.starts_with("#0x")) 
			{
				int matched = std::sscanf(buffer.data(), "#0x%X", &offset);
				if (matched != 1) 
				{
					offset = -1;
				}
				continue;
			}

			if (offset == -1) 
			{
				continue;
			}

			if (buffer.find("//") != std::string_view::npos)
			{
				continue;
			}

			size_t pos = buffer.rfind(reinterpret_cast<const char*>(u8"◎★"));
			if (pos == std::string_view::npos) 
			{
				continue;
			}

			std::string string = buffer.substr(pos + 6);
			script_helper::text text{ .offset = offset, .string = string };
			result.push_back(text);
		}

		return true;
	}

	multi_script_helper::multi_script_helper(std::string_view ipt_dirOrFile, std::string_view opt_dir, const script_info* info, uint32_t ipt_cdpg) :
		m_IptDir(ipt_dirOrFile), m_OptDir(opt_dir), m_IptCodePage(ipt_cdpg)
	{

		this->m_Helper.set_info(info);

		if (ipt_dirOrFile.empty())
		{
			throw std::exception
			{ 
				"[multi_script_helper::ctor] ipt <DirOrFile> is empty!" 
			};
		}

		if (!std::filesystem::exists(ipt_dirOrFile))
		{
			throw std::exception
			{ 
				"[multi_script_helper::ctor] ipt <DirOrFile> is not exists!" 
			};
		}

		if (std::filesystem::is_directory(ipt_dirOrFile))
		{
			for (const auto& entry : std::filesystem::directory_iterator(ipt_dirOrFile))
			{
				const auto&& file = entry.path().string();
				if (this->m_ConfigFile.empty())
				{
					size_t find_pos = file.find_last_of("\\/");
					if (find_pos == std::string::npos)
					{
						if (file == config::name)
						{
							this->m_ConfigFile.assign(file);
							continue;
						}
					}
					std::string_view name = file.c_str() + find_pos + 1;
					if (name == config::name)
					{
						this->m_ConfigFile.assign(file);
						continue;
					}
				}
				this->m_FileList.push_back(file);
			}
			return;
		}

		size_t find_pos = ipt_dirOrFile.find_last_of("\\/");
		if (find_pos != std::string::npos && find_pos != ipt_dirOrFile.size() - 1)
		{
			this->m_IptDir.assign(ipt_dirOrFile, 0, find_pos);
			this->m_FileList.emplace_back(ipt_dirOrFile);
		}
		else
		{
			this->m_IptDir.assign(".\\");
			this->m_FileList.emplace_back(ipt_dirOrFile);
		}
	}

	auto multi_script_helper::get_err_msg() const -> const std::string&
	{
		return this->m_ErrorMessage;
	}

	auto multi_script_helper::set_ipt_cdpg(int32_t cdpg) -> multi_script_helper&
	{
		this->m_IptCodePage = cdpg;
		return *this;
	}

	auto multi_script_helper::run(success_call onSuccess, failure_call onFailure, bool _noexcept) -> multi_script_helper&
	{

		this->m_OnSuccess = onSuccess;
		this->m_OnFailure = onFailure;

		try 
		{
			if (!this->m_ConfigFile.empty())
			{
				this->import_all_text();
			}
			else
			{
				this->export_all_text();
			}
		}
		catch (const std::exception& err)
		{
			if (!_noexcept)
			{
				throw err;
			}
		}

		return *this;
	}

	auto multi_script_helper::import_all_text() -> void
	{

		auto&& CONFIG = multi_script_helper::config::read(this->m_ConfigFile);

		if (CONFIG.InputPath.empty()) 
		{
			throw std::exception
			{
				"[multi_script_helper::import_all_text] "
				"Mes path is empty！Please check."
			};
		}

		if (!std::filesystem::is_directory(CONFIG.InputPath)) 
		{
			throw std::exception
			{
				"[multi_script_helper::import_all_text] "
				"Mes path is not a directory or does not exist!"
			};
		}

		size_t pos = CONFIG.InputPath.find_last_of("\\/");
		if (pos != (CONFIG.InputPath.size() - 1)) 
		{
			CONFIG.InputPath.append("\\");
		}

		size_t success_count{};
		std::string opt_path{};
		text_formater formater{ CONFIG };
		for (const std::string& file : this->m_FileList)
		{

			if (!utils::extension_check(file, ".txt")) 
			{
				this->on_failure("It's not an txt file.", file);
				continue;
			}

			std::string mes_path{}, opt_name{};
			{
				size_t find_pos = file.find_last_of("\\/");
				if (find_pos != std::string::npos) 
				{
					mes_path.assign(file.data() + find_pos + 1);
				}
				else
				{
					mes_path.assign(file);
				}

				if (mes_path.empty()) 
				{
					this->on_failure("Name empty.", file);
					continue;
				}

				find_pos = mes_path.find_last_of(".");
				if (find_pos != std::string::npos) 
				{
					opt_name.assign(mes_path.substr(0, find_pos));
				}
				opt_name.append(".mes");
				mes_path.assign(CONFIG.InputPath + opt_name);
			}

			try {
				if (!this->m_Helper.read(mes_path, false).is_parsed())
				{
					this->on_failure(
						"Failed to read or parse mes file.",
						mes_path
					);
					continue;
				}
			}
			catch (const std::exception& err)
			{
				this->on_failure(err.what(), mes_path);
				continue;
			}

			if (opt_path.empty()) 
			{
				const char* name = this->m_Helper.get_view().info()->name;
				opt_path.assign(this->m_OptDir).append(name).append("_mes\\");
			}

			if (!std::filesystem::exists(opt_path)) {
				std::filesystem::create_directories(opt_path);
				if (!std::filesystem::exists(opt_path)) {
					throw std::exception
					{
						"[multi_script_helper::import_all_text] "
						"Failed to create output directory."
					};
				}
			}

			std::vector<script_helper::text> texts{};
			bool read = multi_script_helper::read_text(file, texts);

			if (!read)
			{
				this->on_failure("Failed to read txt file.", file);
				continue;
			}

			if (texts.empty())
			{
				this->on_failure("Texts empty, Skipped.", file);
				continue;
			}

			for (script_helper::text& text : texts)
			{
				formater.format(text.string);
			}

			bool imported = this->m_Helper.import_scene_text(texts);

			if (imported)
			{
				std::string opt_file_path{ opt_path + opt_name };
				utils::file opt_file(opt_file_path, "wb");
				if (!opt_file.is_open()) {
					this->on_failure(
						"Failed to create output file: " + opt_file_path,
						file
					);
					continue;
				}
				auto raw = this->m_Helper.get_view().raw();
				opt_file.write(raw.data, 1, raw.size);
				this->on_success(file, opt_file_path);
				success_count++;
			}
			else
			{
				this->on_failure("Import text failed.", file);
			}
		}

		if (success_count == 0)
		{
			throw std::exception
			{
				"[multi_script_helper::import_all_text] "
				"failed to all file!"
			};
		}

	}

	auto multi_script_helper::export_all_text() -> void
	{
		size_t success_count{};
		std::string opt_path{};
		utils::string::buffer buffer{}, line{};
		for (const std::string& file : this->m_FileList)
		{

			if (!utils::extension_check(file, ".mes"))
			{
				this->on_failure("It's not an mes file.", file);
				continue;
			}

			try {

				if (!this->m_Helper.read(file).is_parsed())
				{
					this->on_failure(
						"Failed to read or parse mes file.",
						file
					);
					continue;
				}
			}
			catch (const std::exception& err)
			{
				this->on_failure(err.what(), file);
				continue;
			}

			if (opt_path.empty())
			{
				const char* name = this->m_Helper.get_view().info()->name;
				opt_path.assign(this->m_OptDir).append(name).append("_text\\");
			}

			if (!std::filesystem::exists(opt_path))
			{
				std::filesystem::create_directories(opt_path);
				if (!std::filesystem::exists(opt_path))
				{
					throw std::exception
					{
						"[multi_script_helper::export_all_text] "
						"Failed to create output directory."
					};
				}
			}

			std::string opt_name{};
			{
				size_t find_pos = file.find_last_of("\\/");
				if (find_pos != std::string::npos)
				{
					opt_name.assign(file.data() + find_pos + 1);
				}
				else
				{
					opt_name.assign(file);
				}
				if (opt_name.empty())
				{
					this->on_failure("Name empty.", file);
					continue;
				}

				find_pos = opt_name.find_last_of(".");
				if (find_pos != std::string::npos)
				{
					opt_name.assign(opt_name.substr(0, find_pos));
				}
				opt_name.append(".txt");
			}

			std::string opt_file_path{ opt_path + opt_name };
			utils::file opt_file(opt_file_path, "wb");
			if (!opt_file.is_open())
			{
				this->on_failure(
					"Failed to create output file: " + opt_file_path,
					file
				);
				continue;
			}

			buffer.reset();
			const auto& texts = this->m_Helper.fetch_scene_text();
			for (size_t i = 0; i < texts.size(); i++)
			{
				const script_helper::text& text = texts[i];
				if (text.string.empty())
				{
					continue;
				}
				buffer.write_as_format("#0x%X\n", text.offset);
				line.reset().write(text.string).replace("\n", "\\n");
				line.convert_to_utf8(this->m_IptCodePage);
				buffer.write_as_format(u8"★◎  %03d  ◎★//%s\n", i + 1, line.data());
				buffer.write_as_format(u8"★◎  %03d  ◎★%s\n\n", i + 1, line.data());
			}
			opt_file.write(buffer.data(), 1, buffer.count());
			opt_file.close();

			this->on_success(file, opt_file_path);
			success_count++;
		}

		if (success_count > 0)
		{
			this->make_config_file(opt_path);
		}
		else 
		{
			throw std::exception
			{
				"[multi_script_helper::export_all_text] "
				"failed to all file!"
			};
		}
	}

	auto multi_script_helper::make_config_file(const std::string& opt_dir) -> void
	{

		utils::string::buffer buffer{};
		buffer.write_as_format("%s\n%s\n\n", config::path, this->m_IptDir);
		buffer.write_as_format("%s\n%d\n\n", config::cdpg, 936);
		buffer.write_as_format("%s\n%d\n\n", config::tmin, 22);
		buffer.write_as_format("%s\n%d\n\n", config::tmax, 24);
		buffer.write_as_format("%s\n[]:[]\n\n", config::bfrp);
		buffer.write_as_format("%s\n[]:[]\n\n", config::atrp);
		std::string opt_config_path{ opt_dir + config::name };
		utils::file opt_config_file(opt_config_path, "wb");
		if (!opt_config_file.is_open())
		{
			this->on_failure(
				"Failed to create configuration file.",
				opt_config_path
			);
			return;
		}
		opt_config_file.write(buffer.data(), 1, buffer.count());
		opt_config_file.close();
		this->on_success("", opt_config_path);
	}
}