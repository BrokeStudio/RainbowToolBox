project "CLI"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
debugdir "Binaries/%{cfg.targetdir}"
staticruntime "off"
targetname "RainbowToolboxCLI"

files
{
  "Source/**.h", "Source/**.cpp",

  "../Common/Source/**.h", "../Common/Source/**.cpp",

  "../RainbowFileSystem/Include/**.h",
  "../RainbowFileSystem/Source/**.cpp",

  "../RainbowPatcher/Include/**.h",
  "../RainbowPatcher/Source/**.cpp",

  "../External/CLI11/*.hpp",
  "../External/termcolor/*.hpp",
}

vpaths {
  ["Common"] = {
    "../Common/Source/**.h",
    "../Common/Source/**.cpp",
  },
  ["RainbowFileSystem"] = {
    "../RainbowFileSystem/Include/**.h",
    "../RainbowFileSystem/Source/**.cpp",
  },
  ["RainbowPatcher"] = {
    "../RainbowPatcher/Include/**.h",
    "../RainbowPatcher/Source/**.cpp",
  },
  ["CLI11"] = { "../External/CLI11/*.hpp" },
  ["termcolor"] = { "../External/termcolor/*.hpp" },
}

includedirs
{
  "Source",

  "../Common/Source",
  "../Common/generated",

  "../RainbowFileSystem/Include",

  "../RainbowPatcher/Include",

  "../External/CLI11",
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

  filter { "configurations:Dist", "system:windows or linux" }
  kind "ConsoleApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowToolbox")

filter { "configurations:Dist", "system:macosx" }
  kind "ConsoleApp"
  defines { "_DIST" }
  runtime "Release"
  optimize "On"
  symbols "Off"
  targetdir("../Binaries/" .. OutputDir .. "/RainbowToolbox")

filter "configurations:*"
  postbuildcommands
  {
    "{COPY} \"../README.txt\" \"%{cfg.targetdir}\""
  }

-- Windows

filter { "system:windows" }
  staticruntime "on"

filter { "system:windows", "platforms:x86" }
  linkoptions { "/SAFESEH:NO" } -- Image Has Safe Exception Handers: No

filter "system:windows"
  files { '../Windows/Resources/resources.rc', '**.ico' }
  vpaths { ["Resources"] = { "../Windows/Resources/*.rc", "../Windows/Resources/*.ico" } }
  systemversion "latest"
  defines { "_CRT_SECURE_NO_WARNINGS" }
  links {
    "opengl32"
  }

filter { "system:windows", "configurations:Debug", "platforms:x86" }
  libdirs { "../External/libusb/VS2022/MS32/static" }

filter { "system:windows", "configurations:Debug", "platforms:x86_64" }
  libdirs { "../External/libusb/VS2022/MS64/static" }

filter { "system:windows", "configurations:Release or Dist", "platforms:x86" }
  libdirs { "../External/libusb/VS2022/MS32/static" }

filter { "system:windows", "configurations:Release or Dist", "platforms:x86_64" }
  libdirs { "../External/libusb/VS2022/MS64/static" }

-- Linux

filter "system:linux"
  links {
    "udev",
    "pthread"
  }

-- macOS

filter "system:macosx"
