project "RainbowFileExplorer"
language "C++"
cppdialect "C++17"
targetdir "Binaries/%{cfg.buildcfg}"
debugdir "Binaries/%{cfg.targetdir}"
staticruntime "off"
targetname "RainbowFileExplorer"

files
{
  "Source/**.h", "Source/**.cpp",
  "FileBrowser/**.h", "FileBrowser/**.cpp",
  "MemoryEditor/**.h", "MemoryEditor/**.cpp",

  "../External/fonts/**.h",
  "../External/SDL2/include/**.h",
  "../External/imgui/*.h", "../External/imgui/*.cpp",
  "../External/imgui/backends/**.h", "../External/imgui/backends/**.cpp",
  "../External/imgui/misc/cpp/**.h", "../External/imgui/misc/cpp/**.cpp",
  -- "../External/imgui/FileBrowser/**.h", "../External/imgui/FileBrowser/**.cpp",
}

vpaths {
  ["SDL2"] = {
    "../External/SDL2/include/**.h",
  },
  ["ImGui/*"] = {
    "../External/imgui/*.h",
    "../External/imgui/*.cpp",
    "../External/imgui/backends/*.h",
    "../External/imgui/backends/*.cpp",
    "../External/imgui/misc/cpp/*.h",
    "../External/imgui/misc/cpp/*.cpp",
  }
}

includedirs
{
  "Source",
  "FileBrowser",
  "MemoryEditor",

  "../External/fonts",
  "../External/imgui",
  "../External/imgui/backends",
  "../External/imgui/misc/cpp",
  "../External/SDL2",
}

targetdir("../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

filter "system:windows"
  -- files { '../Windows/Resources/resources.rc', '**.ico' }
  -- vpaths { ['../Windows/Resources/*'] = { '*.rc', '**.ico' } }
  systemversion "latest"
  defines { "_CRT_SECURE_NO_WARNINGS" }
  links {
    "opengl32",
    "SDL2",
    "SDL2main",
  }
  includedirs
  {
    -- Include SDL2
    "../External/SDL2/include",
  }
  libdirs
  {
    -- SDL2
    "../External/SDL2/x86",
  }
  prebuildcommands {
    "{COPYFILE} \"../External/SDL2/x86/SDL2.dll\" \"%{cfg.targetdir}\"",
  }

filter "system:linux"
  buildoptions "`sdl2-config --cflags`"
  linkoptions "-lGL `sdl2-config --libs`"
  links { "pthread" }

filter "system:macosx"
  buildoptions "`sdl2-config --cflags`"
  linkoptions "-framework OpenGL -framework CoreFoundation `sdl2-config --libs`"
  links { "pthread" }

filter "configurations:Debug"
  kind "ConsoleApp"
  defines { "_DEBUG" }
  runtime "Debug"
  symbols "On"

filter "configurations:Release"
  kind "ConsoleApp"
  defines { "_RELEASE" }
  runtime "Release"
  optimize "On"
  symbols "On"

filter { "configurations:Dist", "system:windows or linux" }
  kind "WindowedApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowFileExplorer")

filter { "configurations:Dist", "system:macosx" }
  kind "ConsoleApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowFileExplorer")
  -- includedirs
  -- {
  --   "../macOS"
  -- }
