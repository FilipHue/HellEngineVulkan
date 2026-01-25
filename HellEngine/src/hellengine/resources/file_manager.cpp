#include "hepch.h"
#include "file_manager.h"

// Internal
#include <hellengine/core/engine/engine.h>

// External
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include <commdlg.h>

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

		File FileManager::OpenFile(const char* filter)
		{
#ifdef HE_PLATFORM_WINDOWS
			OPENFILENAMEA ofn;
			CHAR sz_file[FILE_MAX_PATH] = { 0 };

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)core::Engine::GetInstance().GetApplication().GetWindow()->GetHandle());
			ofn.lpstrFile = sz_file;
			ofn.nMaxFile = sizeof(sz_file);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			File file;
			if (GetOpenFileNameA(&ofn) == TRUE)
			{
				file = ReadFile(sz_file);
			}

			return file;
#endif
		}

		File FileManager::OpenFile(const std::string& path)
		{
			return OpenFile(path.c_str());
		}

		b8 FileManager::Exists(const char* path)
		{
			return std::filesystem::exists(path);
		}

		b8 FileManager::Exists(const std::string& path)
		{
			return Exists(path.c_str());
		}

	} // namespace resources

} // namespace hellengine