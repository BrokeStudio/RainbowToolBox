#include <stdio.h>

// imgui
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

// SDL
#include <SDL.h>
#include <SDL_opengl.h>
#ifdef _WIN32
  #include <windows.h> // SetProcessDPIAware()
#endif

// FontAwesome
#include "IconsFontAwesome6.h"
#include "fa_regular_400.h"
#include "fa_solid_900.h"
#include "RobotoMonoRegular.h"
#include "RubikRegular.h"

#include "About.h"
#include "Menu.h"
#include "Settings.h"

// File System Explorer
#include <RainbowFileSystemGUI/Explorer.h>
#include <RainbowPatcherGUI/PatcherGUI.h>

/**
 * @brief Rainbow Toolbox graphical entry point.
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument values.
 * @return Process exit code.
 */
int main(int argc, char** argv)
{
  // Setup SDL
#ifdef _WIN32
  ::SetProcessDPIAware();
#endif
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Setup window
  int min_width = 720;
  int min_height = 350;
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Window* window = SDL_CreateWindow("Rainbow Toolbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(min_width * main_scale), (int)(min_height * main_scale), window_flags);
  if(window == nullptr) {
    SDL_Quit();
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return -1;
  }

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  SDL_SetWindowMinimumSize(window, (int)(min_width * main_scale), (int)(min_height * main_scale));
  // SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;
  io.IniFilename = nullptr;

  // #ifdef __APPLE__
  //   std::string iniPath;
  // #ifdef _DIST
  //   getResourcesPath(iniPath); // TODO: handle error
  // #else
  //   getExecutablePath(iniPath); // TODO: handle error
  // #endif
  //   iniPath += "imgui.ini";
  //   io.IniFilename = iniPath.c_str();
  // #endif

#ifdef __APPLE__
  char* basePath = SDL_GetBasePath();
  char* prefPath = SDL_GetPrefPath("Broke Studio", "RainbowToolbox");

  // if(basePath != nullptr) {
  //   luaPath = basePath;
  //   SDL_free(basePath);
  // }

  // if(prefPath != nullptr) {
  //   writePath = prefPath;
  //   SDL_free(prefPath);
  // }

  // std::filesystem::create_directories(
  //   std::filesystem::path(writePath) / "ignore");
#endif

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup scaling
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
  style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL2_Init();

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  ImFont* DefaultFont = io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  // IM_ASSERT(font != nullptr);

  float baseFontSize = 18.0f; // 13.0f is the size of the default font. Change to the font size you use.

  // add FontAwesome fonts
  float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
  ImFontConfig icons_config;
  icons_config.MergeMode = true;
  icons_config.PixelSnapH = true;
  icons_config.GlyphMinAdvanceX = iconFontSize;
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_regular_400_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_solid_900_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);

  // add Roboto Mono Regular font
  ImFont* RobotoMonoRegularFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoMonoRegular_compressed_data_base85, baseFontSize);

  // add FontAwesome fonts
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_regular_400_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_solid_900_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);

  // add Rubik Regular font
  ImFont* RubikRegularFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(RubikRegular_compressed_data_base85, baseFontSize);

  // add FontAwesome fonts
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_regular_400_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_solid_900_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);

  ImFont* fonts[] = { DefaultFont, RobotoMonoRegularFont, RubikRegularFont };

  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  std::string droppedFilename = "";

  Patcher::init();

  /*
  8b    d8    db    88 88b 88     88      dP"Yb   dP"Yb  88""Yb
  88b  d88   dPYb   88 88Yb88     88     dP   Yb dP   Yb 88__dP
  88YbdP88  dP__Yb  88 88 Y88     88  .o Yb   dP Yb   dP 88"""
  88 YY 88 dP""""Yb 88 88  Y8     88ood8  YbodP   YbodP  88
  */

  bool done = false;
  while(!done) {
    // reset dropped filename
    droppedFilename = "";

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if(event.type == SDL_QUIT) {
        done = true;
      }

      if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
        done = true;
      }

      if(event.type == SDL_DROPFILE) {
        droppedFilename = std::string(event.drop.file);
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(fonts[Settings::settings.font]);

    // #if 0

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
    } else {
      ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
      ImGui::SetNextWindowSize(viewport->Size); // ImGui::GetIO().DisplaySize);
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Main", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize); // ImGuiWindowFlags_MenuBar

    // #endif

#if defined(_DEBUG)
    if(Settings::settings.imgui_demo) {
      ImGui::ShowDemoWindow(&Settings::settings.imgui_demo);
    }
#endif

    ImVec2 MenuButtonSize = ImVec2(64.0f, 64.0f);

    // TOP
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImGui::GetContentRegionAvail());
    ImGui::BeginChild("Top", ImVec2(0.0f, viewport->Size.y), 0); // TOP start
    ImGui::BeginChild("MenuPanel", ImVec2(220.0f * main_scale, 0.0f), ImGuiChildFlags_Border);
    Menu::render();
    ImGui::EndChild(); // MenuPanel end
    ImGui::SameLine();
    ImGui::BeginChild("Content", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border);
    switch(Menu::current_selection) {
      case 1:
        // render file system explorer
        FileSystem::render(droppedFilename, main_scale);
        break;

      case 2:
        Patcher::render(droppedFilename, main_scale);
        break;

      case 3:
        Settings::render();
        break;

      case 4:
        About::render();
        break;

      default:
        break;
    }
    ImGui::EndChild(); // Content end
    ImGui::EndChild(); // TOP end

    ImGui::PopStyleVar();
    ImGui::End(); // MAIN
    ImGui::PopFont();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    // glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(window);
  }

  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
