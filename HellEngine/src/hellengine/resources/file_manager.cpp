#include "hepch.h"
#include "file_manager.h"

namespace hellengine
{
	namespace resources
	{

		File FileManager::ReadFile(const char* path)
		{

			File file;

			std::ifstream file_stream;
			file_stream.open(path);
			if (!file_stream.is_open())
			{
				HE_CORE_ERROR("Failed to open file: {0}", path);
				return File();
			}

			file.relative_file_path = path;
			file.absolute_file_path = std::filesystem::absolute(path).string();
			file.absolute_directory_path = std::filesystem::absolute(path).parent_path().string();
			file.relative_directory_path = std::filesystem::path(path).parent_path().string();
			file.name = std::filesystem::path(path).filename().string();
			file.extension = std::filesystem::path(path).extension().string();

			std::stringstream buffer;
			buffer << file_stream.rdbuf();
			file.content = buffer.str();

			file.size = std::filesystem::file_size(path);

			file.stem = std::filesystem::path(path).stem().string();

			file_stream.close();

			return file;
		}

		File FileManager::ReadFile(const std::string& path)
		{
			return ReadFile(path.c_str());
		}

	} // namespace resources

} // namespace hellengine