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
        "%{IncludeDirs.IMGUIZMO}",
        "%{IncludeDirs.ENTT}",
        "%{IncludeDirs.YAML}",

        "%{IncludeDirs.VULKAN}"
    }

    -- If it is a fresh build or you build for static, comment out the post build command
    -- It will give an error since the folder does not exist
    -- postbuildcommands
    -- {
    --     ("{COPYFILE} %[%{!wks.location}bin/%{output_dir}/HellEngine/HellEngine.dll] %[%{!wks.location}bin/%{output_dir}/Sandbox/]")
    -- }

    filter "files:HellEngine/dependencies/GLM/include/**.c*"
        enablepch "Off"

    filter "files:HellEngine/dependencies/GLAD/glad.c"
        enablepch "Off"

    filter "files:HellEngine/dependencies/STB/**.c*"
        enablepch "Off"

    filter "files:HellEngine/dependencies/KTX/**.c*"
        enablepch "Off"

    filter "files:HellEngine/dependencies/IMGUI/**.c*"
        enablepch "Off"

    filter "files:HellEngine/dependencies/IMGUIZMO/**.c*"
        enablepch "Off"

    filter "files:HellEngine/dependencies/YAML/**.c*"
        enablepch "Off"

    filter "system:windows"
        cppdialect "C++20"
        defines 
        { 
            "HE_PLATFORM_WINDOWS",
            "YAML_CPP_STATIC_DEFINE",
            -- "HE_BUILD_DLL",
            -- "HE_DLL"
        }

        buildoptions
        {
            "/Zc:__cplusplus"
        }

        multiprocessorcompile ("On")

    filter "configurations:Debug"
        runtime "Debug"
        defines "HE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        defines {"HE_RELEASE", "NDEBUG", "YAML_CPP_STATIC_DEFINE"}
        optimize "On"

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
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
        "HellEngine/dependencies/IMGUIZMO/include",
        "HellEngine/dependencies/ENTT/include",
        "HellEngine/dependencies/YAML/include",
        "%{IncludeDirs.VULKAN}"
    }

    libdirs
    {
        "%{LibraryDirectories.SPDLOG}",
        "%{LibraryDirectories.GLFW}",
        "%{LibraryDirectories.ASSIMP}",
        "%{LibraryDirectories.KTX}",
        "%{LibraryDirectories.YAML}",
        "%{LibraryDirectories.VulkanSDK}"
    }

    links 
    { 
        "HellEngine"
    }

    filter "system:windows"
        cppdialect "C++20"
        defines 
        {
            "HE_PLATFORM_WINDOWS",
        }
        buildoptions { "/Zc:__cplusplus" }
        multiprocessorcompile ("On")

    filter "configurations:Debug"
        runtime "Debug"
        defines {"HE_DEBUG", "HE_EDITOR", "YAML_CPP_STATIC_DEFINE"}
        symbols "On"
        
        links
        {
            "%{Library.SPDLOG}",
            "%{Library.GLFW}",
            "%{Library.ASSIMP}",
            "%{Library.KTX}",
            "%{Library.YAMLD}",
            "%{Library.Vulkan}"
        }

    filter "configurations:Release"
        runtime "Release"
        defines {"HE_RELEASE", "NDEBUG", "HE_DEBUG", "YAML_CPP_STATIC_DEFINE"}
        optimize "On"
        
        links
        {
            "%{Library.SPDLOG}",
            "%{Library.GLFW}",
            "%{Library.ASSIMP}",
            "%{Library.KTX}",
            "%{Library.YAML}",
            "%{Library.Vulkan}"
        }

    filter {}  -- Reset filter

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
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
        "HellEngine/dependencies/IMGUIZMO/include",
        "HellEngine/dependencies/ENTT/include",
        "HellEngine/dependencies/YAML/include",
        "%{IncludeDirs.VULKAN}"
    }

    libdirs
    {
        "%{LibraryDirectories.SPDLOG}",
        "%{LibraryDirectories.GLFW}",
        "%{LibraryDirectories.ASSIMP}",
        "%{LibraryDirectories.KTX}",
        "%{LibraryDirectories.YAML}",
        "%{LibraryDirectories.VulkanSDK}"
    }

    links 
    { 
        "HellEngine"
    }

    filter "system:windows"
        cppdialect "C++20"
        defines 
        {
            "HE_PLATFORM_WINDOWS",
        }
        buildoptions { "/Zc:__cplusplus" }
        multiprocessorcompile ("On")

    filter "configurations:Debug"
        runtime "Debug"
        defines {"HE_DEBUG", "YAML_CPP_STATIC_DEFINE"}
        symbols "On"

        links
        {
            "%{Library.SPDLOG}",
            "%{Library.GLFW}",
            "%{Library.ASSIMP}",
            "%{Library.KTX}",
            "%{Library.YAMLD}",
            "%{Library.Vulkan}"
        }

    filter "configurations:Release"
        runtime "Release"
        defines {"HE_RELEASE", "NDEBUG", "YAML_CPP_STATIC_DEFINE"}
        optimize "On"

        links
        {
            "%{Library.SPDLOG}",
            "%{Library.GLFW}",
            "%{Library.ASSIMP}",
            "%{Library.KTX}",
            "%{Library.YAML}",
            "%{Library.Vulkan}"
        }

    filter {}
