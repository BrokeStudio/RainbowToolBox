project "GUI"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
debugdir "Binaries/%{cfg.targetdir}"
staticruntime "off"
targetname "RainbowToolbox"

files
{
  "./Source/**.h", "./Source/**.cpp",
  "./fonts/**.h",

  "../Common/Source/**.h", "../Common/Source/**.cpp",

  "../RainbowFileSystem/Include/**.h",
  "../RainbowFileSystem/Source/**.cpp",
  "../RainbowFileSystem/GUI/Include/**.h",
  "../RainbowFileSystem/GUI/Source/**.cpp",

  "../RainbowPatcher/Include/**.h",
  "../RainbowPatcher/Source/**.cpp",
  "../RainbowPatcher/GUI/Include/**.h",
  "../RainbowPatcher/GUI/Source/**.cpp",

  "../External/SDL2/include/**.h",
  "../External/imgui/*.h", "../External/imgui/*.cpp",
  "../External/imgui/backends/**.h", "../External/imgui/backends/**.cpp",
  "../External/imgui/misc/cpp/**.h", "../External/imgui/misc/cpp/**.cpp",
  "../External/FileBrowser/**.h", "../External/FileBrowser/**.cpp",
  "../External/MemoryEditor/**.h", "../External/MemoryEditor/**.cpp",
}

vpaths {
  ["Common"] = {
    "../Common/Source/**.h",
    "../Common/Source/**.cpp",
  },
  ["RainbowFileSystemGUI"] = {
    "../RainbowFileSystem/Include/**.h",
    "../RainbowFileSystem/Source/**.cpp",
    "../RainbowFileSystem/GUI/Include/**.h",
    "../RainbowFileSystem/GUI/Source/**.cpp",
  },
  ["RainbowPatcher"] = {
    "../RainbowPatcher/Include/**.h",
    "../RainbowPatcher/Source/**.cpp",
    "../RainbowPatcher/GUI/Include/**.h",
    "../RainbowPatcher/GUI/Source/**.cpp",
  },
  ["SDL2"] = {
    "../External/SDL2/include/**.h",
  },
  ["ImGui"] = {
    "../External/imgui/**.h",
    "../External/imgui/**.cpp",
    "../External/imgui/backends/**.h",
    "../External/imgui/backends/**.cpp",
    "../External/imgui/misc/cpp/*.h",
    "../External/imgui/misc/cpp/*.cpp",
    "../External/FileBrowser/**.h",
    "../External/FileBrowser/**.cpp",
    "../External/MemoryEditor/**.h",
    "../External/MemoryEditor/**.cpp",
  }
}

includedirs
{
  "./Source",
  "./fonts",

  "../Common/Source",
  "../Common/generated",

  "../RainbowFileSystem/Include",
  "../RainbowFileSystem/GUI/Include",
  "../RainbowFileSystem/GUI/Source",

  "../RainbowPatcher/Include",
  "../RainbowPatcher/GUI/Include",

  "../External/imgui",
  "../External/imgui/backends",
  "../External/imgui/misc/cpp",
  "../External/FileBrowser",
  "../External/MemoryEditor",
  "../External/termcolor",
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

filter "configurations:Debug or Release or Dist"
  postbuildcommands
  {
    "{COPY} \"../README.txt\" \"%{cfg.targetdir}\""
  }

-- Windows

filter { "system:windows" }
  staticruntime "on"

filter { "system:windows", "platforms:x86" }
  linkoptions { "/SAFESEH:NO" } -- Image Has Safe Exception Handers: No

filter { "system:windows", "configurations:Dist" }
  kind "WindowedApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowToolbox")
  entrypoint "mainCRTStartup"

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
  links {
    "SDL2-staticd"
  }
  libdirs {
    "../External/SDL2/lib/x86-static-debug"
  }

filter { "system:windows", "configurations:Debug", "platforms:x86_64" }
  links {
    "SDL2-staticd"
  }
  libdirs {
    "../External/SDL2/lib/x64-static-debug"
  }

filter { "system:windows", "configurations:Release or Dist", "platforms:x86" }
  links {
    "SDL2-static"
  }
  libdirs {
    "../External/SDL2/lib/x86-static-release"
  }

filter { "system:windows", "configurations:Release or Dist", "platforms:x86_64" }
  links {
    "SDL2-static"
  }
  libdirs {
    "../External/SDL2/lib/x64-static-release"
  }

-- Linux

filter "system:linux"
  buildoptions {
    "`sdl2-config --cflags`"
  }
  linkoptions {
    "`sdl2-config --static-libs`"
  }
  links {
    "GL",
    "udev",
    "pthread",
  }

filter { "system:linux", "configurations:Dist" }
  kind "WindowedApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowToolbox")

-- macOS

filter "system:macosx"
  includedirs {
    "../macOS",
    "../External/SDL2-macOS/SDL2.framework/Headers",
  }

  buildoptions {
    "-mmacosx-version-min=12.0",
    "-F../External/SDL2-macOS"
  }

  linkoptions {
    "-mmacosx-version-min=12.0",
    "-F../External/SDL2-macOS",
    "-framework SDL2",
    "-Wl,-rpath,@executable_path/../Frameworks",
    "-framework OpenGL",
    "-framework CoreFoundation"
  }

filter { "system:macosx", "configurations:Dist" }
  kind "ConsoleApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowToolbox")
  postbuildcommands
  {
    "{RMDIR} \"%{cfg.targetdir}/../app/RainbowToolbox.app\"",
    "{MKDIR} \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/MacOS\"",
    "{MKDIR} \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/Resources/shared\"",
    "{MKDIR} \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/Frameworks\"",

    "ditto \"../External/SDL2-macOS/SDL2.framework\" \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/Frameworks/SDL2.framework\"",
    "{COPY} \"../macOS/info.plist\" \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/Info.plist\"",
    "{COPY} \"../macOS/AppIcon.icns\" \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/Resources/AppIcon.icns\"",
    "{COPY} \"%{cfg.targetdir}/RainbowToolbox\" \"%{cfg.targetdir}/../app/RainbowToolbox.app/Contents/MacOS/RainbowToolbox\""
  }
