include "libraries.lua"

IncludeDirs = {}

IncludeDirs["SPDLOG"] = "%{wks.location}/HellEngine/dependencies/SPDLOG/include"
IncludeDirs["GLFW"] = "%{wks.location}/HellEngine/dependencies/GLFW/include"
IncludeDirs["GLM"] = "%{wks.location}/HellEngine/dependencies/GLM/include"
IncludeDirs["STB"] = "%{wks.location}/HellEngine/dependencies/STB"
IncludeDirs["ASSIMP"] = "%{wks.location}/HellEngine/dependencies/ASSIMP/include"

IncludeDirs["VULKAN"] = "%{EnvVars.VULKAN_SDK}/Include"