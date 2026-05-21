include "libraries.lua"

IncludeDirs = {}

IncludeDirs["ASSIMP"] = "%{wks.location}/HellEngine/dependencies/ASSIMP/include"
IncludeDirs["DDS"] = "%{wks.location}/HellEngine/dependencies/DDS/include"
IncludeDirs["ENTT"] = "%{wks.location}/HellEngine/dependencies/ENTT/include"
IncludeDirs["GLFW"] = "%{wks.location}/HellEngine/dependencies/GLFW/include"
IncludeDirs["GLM"] = "%{wks.location}/HellEngine/dependencies/GLM/include"
IncludeDirs["IMGUI"] = "%{wks.location}/HellEngine/dependencies/IMGUI"
IncludeDirs["IMGUIZMO"] = "%{wks.location}/HellEngine/dependencies/IMGUIZMO/include"
IncludeDirs["KTX"] = "%{wks.location}/HellEngine/dependencies/KTX/include"
IncludeDirs["SPDLOG"] = "%{wks.location}/HellEngine/dependencies/SPDLOG/include"
IncludeDirs["STB"] = "%{wks.location}/HellEngine/dependencies/STB/include"
IncludeDirs["YAML"] = "%{wks.location}/HellEngine/dependencies/YAML/include"

IncludeDirs["VULKAN"] = "%{EnvVars.VULKAN_SDK}/Include"