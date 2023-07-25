@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@set OUT_DIR=Release
@set OUT_EXE=RainbowToolBox
@set INCLUDES=/I. /Iimgui /Iimgui\misc\cpp /IFileBrowser /IMemoryEditor /Iimgui\backends /ISDL2\include
@set SOURCES=src\*.cpp imgui\backends\*.cpp imgui\imgui*.cpp FileBrowser\ImGuiFileBrowser.cpp imgui\misc\cpp\*.cpp
@set LIBS=/LIBPATH:SDL2\lib\x86 SDL2.lib SDL2main.lib opengl32.lib shell32.lib
mkdir %OUT_DIR%
cl /nologo /Zi /MD /EHsc /Zc:forScope /Gd /FAsc /utf-8 %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /subsystem:windows
@cp SDL2\SDL2.dll %OUT_DIR%
