project "RainbowFileExplorer"
language "C++"
cppdialect "C++17"
targetdir "Binaries/%{cfg.buildcfg}"
debugdir "Binaries/%{cfg.targetdir}"
staticruntime "on"
targetname "RainbowFileExplorer"

files {
  "Source/**.h", "Source/**.cpp",
  "FileBrowser/**.h", "FileBrowser/**.cpp",
  "MemoryEditor/**.h", "MemoryEditor/**.cpp",

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
      "Imm32.lib",
      "opengl32",
      "SDL2"
  }

filter { "system:windows", "configurations:Debug" }
  libdirs {
      "../External/SDL2/lib/x86-static-debug"
  }

filter { "system:windows", "configurations:Release" }
  libdirs {
      "../External/SDL2/lib/x86-static-release"
  }

filter { "system:windows", "configurations:Dist" }
  kind "WindowedApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowFileExplorer")
  entrypoint "mainCRTStartup"
  libdirs {
      "../External/SDL2/lib/x86-static-release"
  }
  postbuildcommands {
    "{DELETE} \"../Binaries/" .. OutputDir .. "/RainbowFileExplorer/RainbowFileExplorer.exp\"",
    "{DELETE} \"../Binaries/" .. OutputDir .. "/RainbowFileExplorer/RainbowFileExplorer.lib\""
  }

-- Linux

filter "system:linux"
  -- buildoptions "`sdl2-config --cflags`"
  -- linkoptions "-lGL -lX11 -lXext -lXrandr -lXrender -lXinerama -lXi -lXcursor `sdl2-config --libs` -static"
  -- linkoptions { "-static-libsan" }
  -- -lasound -lpulse
  libdirs {
    "../External/SDL2/lib/linux"
  }
  includedirs {
    "../External/SDL2/include",
  }
  links {
        -- "SDL2", -- Pour libSDL2.a
        "GL",   -- Pour libGL.so (OpenGL)
        -- "X11",
        -- "Xext",
        -- "Xrandr",
        -- "Xrender",
        -- "Xinerama",
        -- "Xi",
        -- "Xcursor",
        -- "asound",
        -- "pulse",
        "dl",
        -- "pthread"

    "pthread",
    "SDL2" -- ou le chemin vers libSDL2.a si nécessaire
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
  linkoptions {
    "-framework OpenGL -framework CoreFoundation",
    "-framework CoreVideo -framework AudioToolbox -framework Carbon -framework IOKit",
    "-framework Cocoa -framework ForceFeedback -framework CoreAudio",
    "-framework Foundation -framework Metal",
    "-framework GameController -framework CoreHaptics",
    "-static-libsan",
  }
  links {
    "pthread",
    "m",
    "dl",
    "iconv",
    "SDL2"
  }
  libdirs {
    "../External/SDL2/lib/macOS"
  }
  includedirs {
    "../External/SDL2/include",
    "../macOS"
  }

filter { "configurations:Dist", "system:macosx" }
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
