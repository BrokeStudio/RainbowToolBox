project "RainbowFileExplorer"
language "C++"
cppdialect "C++17"
targetdir "Binaries/%{cfg.buildcfg}"
debugdir "Binaries/%{cfg.targetdir}"
staticruntime "off"
targetname "RainbowFileExplorer"

files {
  "./Source/**.h", "./Source/**.cpp",
  "./FileBrowser/**.h", "./FileBrowser/**.cpp",
  "./MemoryEditor/**.h", "./MemoryEditor/**.cpp",

  "../External/fonts/**.h",
  "../External/SDL2/include/**.h",
  "../External/imgui/*.h", "../External/imgui/*.cpp",
  "../External/imgui/backends/**.h", "../External/imgui/backends/**.cpp",
  "../External/imgui/misc/cpp/**.h", "../External/imgui/misc/cpp/**.cpp",
}

vpaths {
  ["SDL2"] = {
    "../External/SDL2/include/**.h",
  },
  ["ImGui"] = {
    "../External/imgui/*.h",
    "../External/imgui/*.cpp",
    "../External/imgui/backends/*.h",
    "../External/imgui/backends/*.cpp",
    "../External/imgui/misc/cpp/*.h",
    "../External/imgui/misc/cpp/*.cpp",
  }
}

includedirs {
  "./Source",
  "./FileBrowser",
  "./MemoryEditor",

  "../External/fonts",
  "../External/imgui",
  "../External/imgui/backends",
  "../External/imgui/misc/cpp",
}

targetdir("../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

-- Windows / Linux / macOS

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

-- Windows

filter { "system:windows" }
  staticruntime "on"

filter { "system:windows", "configurations:Dist" }
  kind "WindowedApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/Demo")
  entrypoint "mainCRTStartup"

filter "platforms:x86"
    system "Windows"
    architecture "x86"

filter "platforms:x86_64"
    system "Windows"
    architecture "x86_64"

filter "system:windows"
  files { '../Windows/Resources/resources.rc', '**.ico' }
  vpaths { ["Resources"] = { "../Windows/Resources/*.rc", "../Windows/Resources/*.ico" } }
  systemversion "latest"
  defines {
    "_CRT_SECURE_NO_WARNINGS",
    "SDL_MAIN_HANDLED", -- to avoid SDL_main
  }
  includedirs {
    "../External/SDL2/include"
  }
  links {
    "winmm.lib",
    "setupapi.lib",
    "version.lib",
    -- "Imm32.lib",
    "opengl32",
  }

filter { "system:windows", "configurations:Debug", "platforms:x86" }
  links { "SDL2-staticd" }
  libdirs { "../External/SDL2/lib/x86-static-debug" }

filter { "system:windows", "configurations:Debug", "platforms:x86_64" }
  links { "SDL2-staticd" }
  libdirs { "../External/SDL2/lib/x64-static-debug" }

filter { "system:windows", "configurations:Release or Dist", "platforms:x86" }
  links { "SDL2-static" }
  libdirs { "../External/SDL2/lib/x86-static-release" }

filter { "system:windows", "configurations:Release or Dist", "platforms:x86_64" }
  links { "SDL2-static" }
  libdirs { "../External/SDL2/lib/x64-static-release" }

-- Linux

filter "system:linux"
  buildoptions { "`sdl2-config --cflags`" }
  linkoptions { "`sdl2-config --libs`" }
  links {
    "GL",
    -- "dl",
    "pthread",
    "SDL2"
  }

filter { "system:linux", "configurations:Dist" }
  kind "WindowedApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowFileExplorer")

-- macOS

filter "system:macosx"
  buildoptions { "`sdl2-config --cflags`" }
  linkoptions {
    "`sdl2-config --libs`",
    "-framework OpenGL",
    "-framework CoreFoundation"
  }

filter { "system:macosx", "configurations:Dist" }
  kind "ConsoleApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowFileExplorer")
  postbuildcommands {
    "{RMDIR} \"%{cfg.targetdir}/RainbowFileExplorer.app\"",
    "{MKDIR} \"%{cfg.targetdir}/RainbowFileExplorer.app\"",
    "{MKDIR} \"%{cfg.targetdir}/RainbowFileExplorer.app/Contents\"",
    "{MKDIR} \"%{cfg.targetdir}/RainbowFileExplorer.app/Contents/MacOS\"",
    "{MKDIR} \"%{cfg.targetdir}/RainbowFileExplorer.app/Contents/Resources\"",
    "{COPY} \"../macOS/Info.plist\" \"%{cfg.targetdir}/RainbowFileExplorer.app/Contents\"",
    "{COPY} \"%{cfg.targetdir}/RainbowFileExplorer\" \"%{cfg.targetdir}/RainbowFileExplorer.app/Contents/MacOS\"",
    "{COPY} \"../macOS/Rainbow.png\" \"%{cfg.targetdir}/RainbowFileExplorer.app/Contents/Resources\"",
  }
