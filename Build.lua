-- premake5.lua
newoption {
  trigger = "arch",
  value = "ARCH",
  description = "Target architecture",
  allowed = {
    { "arm64", "Apple Silicon" },
    { "x86_64", "Intel 64-bit" },
  },
}

workspace "RainbowToolbox"
  configurations { "Debug", "Release", "Dist" }
  startproject "GUI"

-- Workspace-wide build options for MSVC
filter "system:windows"
  platforms { "x86", "x86_64" }
  buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

filter { "system:windows", "platforms:x86" }
  architecture "x86"

filter { "system:windows", "platforms:x86_64" }
  system "Windows"
  architecture "x86_64"

filter "system:linux"
  architecture "x64"

filter "system:macosx"
  if _OPTIONS["arch"] == "x86_64" then
    architecture "x86_64"
  else
    architecture "ARM64"
  end

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

include "CLI/Build-CLI.lua"
include "GUI/Build-GUI.lua"
