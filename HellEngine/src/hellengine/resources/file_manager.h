#pragma once

// Internal
#include <hellengine/core/core.h>

namespace hellengine
{
	namespace resources
	{

		struct File
		{
			std::string relative_file_path = "";
			std::string absolute_file_path = "";
			std::string relative_directory_path = "";
			std::string absolute_directory_path = "";
			std::string name = "";
			std::string stem = "";
			std::string extension = "";
			std::string content = "";
			u64 size = 0;

			File() = default;
			~File() = default;

			HE_API const std::string& GetRelativePath() const { return relative_file_path; }
			HE_API const std::string& GetAbsolutePath() const { return absolute_file_path; }
			HE_API const std::string& GetRelativeDirectory() const { return relative_directory_path; }
			HE_API const std::string& GetAbsoluteDirectory() const { return absolute_directory_path; }
			HE_API const std::string& GetName() const { return name; }
			HE_API const std::string& GetExtension() const { return extension; }
			HE_API const std::string& GetContent() const { return content; }
			HE_API u64 GetSize() const { return size; }
		};

		class FileManager
		{
		public:
			HE_API static File ReadFile(const char* path);

		private:
			FileManager() = delete;
		};

	} // namespace resources

} // namespace hellengine