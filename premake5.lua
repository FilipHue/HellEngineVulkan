include "includes.lua"
include "libraries.lua"

workspace "HellEngineVulkan"
    architecture "x64"
    configurations 
    { 
        "Debug", 
        "Release" 
    }
    startproject "Editor"

    output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "HellEngine"
    location "HellEngine"
    -- kind "SharedLib"
    kind "StaticLib"
    language "C++"

    -- This is to make sure that the runtime library is linked dynamically
    -- ON means /MT and OFF means /MD
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. output_dir .. "/%{prj.name}")

    pchheader "hepch.h"
    pchsource "HellEngine/src/hepch.cpp"

    files 
    { 
        "%{prj.name}/**.h*",
        "%{prj.name}/**.c*"
    }

    includedirs 
    { 
        "%{prj.name}/src",
        "%{IncludeDirs.SPDLOG}",
        "%{IncludeDirs.GLAD}",
        "%{IncludeDirs.GLFW}",
        "%{IncludeDirs.GLM}",
        "%{IncludeDirs.STB}",
        "%{IncludeDirs.ASSIMP}",
        "%{IncludeDirs.KTX}",
        "%{IncludeDirs.IMGUI}",
        "%{IncludeDirs.ENTT}",

        "%{IncludeDirs.VULKAN}"
    }

    links
    {
        "%{Library.SPDLOG}",

        -- If static runtime is on, link against glfw3_mt.lib
        "%{Library.GLFW}",

        "%{Library.ASSIMP}",

        "%{Library.KTX}",
        
        "%{Library.Vulkan}"
    }

    libdirs
    {
        "%{LibraryDirectories.SPDLOG}",
        "%{LibraryDirectories.GLFW}",
        "%{LibraryDirectories.ASSIMP}",
        "%{LibraryDirectories.KTX}",

        "%{LibraryDirectories.VulkanSDK}"
    }

    -- If it is a fresh build, comment out the post build command
    -- It will give an error since the folder does not exist
    postbuildcommands
    {
        ("{COPYFILE} %[%{!wks.location}bin/%{output_dir}/HellEngine/HellEngine.dll] %[%{!wks.location}bin/%{output_dir}/Sandbox/]")
    }

    filter "files:HellEngine/dependencies/GLM/include/**.c*"
        flags { "NoPCH" }

    filter "files:HellEngine/dependencies/GLAD/glad.c"
        flags { "NoPCH" }

    filter "files:HellEngine/dependencies/STB/**.c*"
        flags { "NoPCH" }

    filter "files:HellEngine/dependencies/KTX/**.c*"
        flags { "NoPCH" }

    filter "files:HellEngine/dependencies/IMGUI/**.c*"
        flags { "NoPCH" }

    filter "system:windows"
        cppdialect "C++20"
        defines 
        { 
            "HE_PLATFORM_WINDOWS",
            -- "HE_BUILD_DLL",
            -- "HE_DLL"
        }

        buildoptions
        {
            "/Zc:__cplusplus"
        }

        flags
        {
            "MultiProcessorCompile"
        }

    filter "configurations:Debug"
        runtime "Debug"
        defines "HE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        defines {"HE_RELEASE", "NDEBUG"}
        optimize "On"

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"

    -- This is to make sure that the runtime library is linked dynamically
    -- ON means /MT and OFF means /MD
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. output_dir .. "/%{prj.name}")

    files 
    { 
        "%{prj.name}/**.h", 
        "%{prj.name}/**.cpp" 
    }

    includedirs 
    { 
        "HellEngine/src",
        "HellEngine/dependencies/SPDLOG/include",
        "HellEngine/dependencies/GLFW/include",
        "HellEngine/dependencies/GLM/include",
        "HellEngine/dependencies/STB",
        "HellEngine/dependencies/ASSIMP/include",
        "HellEngine/dependencies/KTX/include",
        "HellEngine/dependencies/IMGUI",
        "HellEngine/dependencies/ENTT/include",

        "%{IncludeDirs.VULKAN}"
    }

    links 
    { 
        "HellEngine"
    }

    postbuildcommands
    {
        ("{COPYFILE} %[%{!wks.location}bin/%{output_dir}/HellEngine/HellEngine.dll] %[%{!wks.location}bin/%{output_dir}/Editor/]")
    }

    filter "system:windows"
        cppdialect "C++20"
        defines 
        {
            "HE_PLATFORM_WINDOWS",
            -- "HE_DLL"
        }

        buildoptions
        {
            "/Zc:__cplusplus"
        }

        flags
        {
            "MultiProcessorCompile"
        }

    filter "configurations:Debug"
        runtime "Debug"
        defines "HE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        defines {"HE_RELEASE", "NDEBUG"}
        optimize "On"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"

    -- This is to make sure that the runtime library is linked dynamically
    -- ON means /MT and OFF means /MD
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. output_dir .. "/%{prj.name}")

    files 
    { 
        "%{prj.name}/**.h", 
        "%{prj.name}/**.cpp" 
    }

    includedirs 
    { 
        "HellEngine/src",
        "HellEngine/dependencies/SPDLOG/include",
        "HellEngine/dependencies/GLFW/include",
        "HellEngine/dependencies/GLM/include",
        "HellEngine/dependencies/STB/include",
        "HellEngine/dependencies/ASSIMP/include",
        "HellEngine/dependencies/KTX/include",
        "HellEngine/dependencies/IMGUI",
        "HellEngine/dependencies/ENTT/include",

        "%{IncludeDirs.VULKAN}"
    }

    links 
    { 
        "HellEngine"
    }

    postbuildcommands
    {
        ("{COPYFILE} %[%{!wks.location}bin/%{output_dir}/HellEngine/HellEngine.dll] %[%{!wks.location}bin/%{output_dir}/Sandbox/]")
    }

    filter "system:windows"
        cppdialect "C++20"
        defines 
        {
            "HE_PLATFORM_WINDOWS",
            -- "HE_DLL"
        }

        buildoptions
        {
            "/Zc:__cplusplus"
        }

        flags
        {
            "MultiProcessorCompile"
        }

    filter "configurations:Debug"
        runtime "Debug"
        defines "HE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        defines "HE_RELEASE"
        optimize "On"