#pragma once

// Internal
#include <hellengine/core/core.h>

namespace hellengine
{

	namespace resources
	{

		constexpr u32 FILE_MAX_PATH = 255;

		class File;
		class FileManager
		{
		public:
			HE_API static File ReadFile(const char* path);
			HE_API static File ReadFile(const std::string& path);

			HE_API static File OpenFile(const char* path);
			HE_API static File OpenFile(const std::string& path);

		private:
			FileManager() = delete;
		};

		class File
		{
		public:
			File() = default;
			~File() = default;

			HE_API const std::string& GetRelativePath() const { return relative_file_path; }
			HE_API const std::string& GetAbsolutePath() const { return absolute_file_path; }
			HE_API const std::string& GetRelativeDirectory() const { return relative_directory_path; }
			HE_API const std::string& GetAbsoluteDirectory() const { return absolute_directory_path; }
			HE_API const std::string& GetName() const { return name; }
			HE_API const std::string& GetStem() const { return stem; }
			HE_API const std::string& GetExtension() const { return extension; }
			HE_API const std::string& GetContent() const { return content; }
			HE_API u64 GetSize() const { return size; }

		private:
			friend class FileManager;

			std::string relative_file_path = "";
			std::string absolute_file_path = "";
			std::string relative_directory_path = "";
			std::string absolute_directory_path = "";
			std::string name = "";
			std::string stem = "";
			std::string extension = "";
			std::string content = "";
			u64 size = 0;
		};

	} // namespace resources

} // namespace hellengine