EnvVars = {}

LibraryDirectories = {}
Library = {}

EnvVars["VULKAN_SDK"] = os.getenv("VULKAN_SDK")

LibraryDirectories["VulkanSDK"] = "%{EnvVars.VULKAN_SDK}/Lib"
LibraryDirectories["SPDLOG"] = "%{wks.location}/HellEngine/dependencies/SPDLOG"
LibraryDirectories["GLFW"] = "%{wks.location}/HellEngine/dependencies/GLFW/lib-vc2022"
LibraryDirectories["GLM"] = "%{wks.location}/HellEngine/dependencies/GLM/lib"
LibraryDirectories["STB"] = "%{wks.location}/HellEngine/dependencies/STB"
LibraryDirectories["ASSIMP"] = "%{wks.location}/HellEngine/dependencies/ASSIMP/lib"
LibraryDirectories["KTX"] = "%{wks.location}/HellEngine/dependencies/KTX/lib"

Library["SPDLOG"] = "%{LibraryDirectories.SPDLOG}/spdlogd.lib"
Library["GLFW"] = "%{LibraryDirectories.GLFW}/glfw3.lib"
Library["GLFW_MT"] = "%{LibraryDirectories.GLFW}/glfw3_mt.lib"
Library["ASSIMP"] = "%{LibraryDirectories.ASSIMP}/assimp-vc143-mtd.lib"
Library["ASSIMP_MT"] = "%{LibraryDirectories.ASSIMP}/assimp-vc143-mt.lib"
Library["Vulkan"] = "%{LibraryDirectories.VulkanSDK}/vulkan-1.lib"
Library["KTX"] = "%{LibraryDirectories.KTX}/ktx.lib"