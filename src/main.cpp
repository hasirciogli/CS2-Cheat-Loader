// main.cpp
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <array>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "Hoffer.hpp"
#include "menu/Menu.hpp"
#include "utils/File.hpp"

#ifndef HLERP
#define HLERP(a, b, t) ((a) + (t) * ((b) - (a)))
#endif

#ifdef BUILD_DEV
struct SLFont {
  SLFont(std::string _name, std::string _path) : name(_name), path(_path) {
    this->name = _name;
    this->path = _path;
  }

  std::string name;
  std::string path;
};
std::vector<SLFont> lmFonts = {
    SLFont("OpenSansMedium", "./assets/fonts/open-sans/OpenSans-Medium.ttf"),
    SLFont("OpenSansBold", "./assets/fonts/open-sans/OpenSans-Bold.ttf"),
    SLFont("OpenSansExtraBold",
           "./assets/fonts/open-sans/OpenSans-ExtraBold.ttf"),
    SLFont("OpenSansLight", "./assets/fonts/open-sans/OpenSans-Light.ttf"),
    SLFont("OpenSansRegular", "./assets/fonts/open-sans/OpenSans-Regular.ttf"),
};
void buildFonts() {
  for (auto &font : lmFonts) {
    File fileX(font.path.c_str());

    auto xx = fileX.readFileToByteArray(font.path);

    // write to file.

    fileX.saveByteArrayToFile(+"./src/menu/fonts/" + font.name + ".byte.hpp",
                              xx, font.name);

    std::cout << "xx.size() = " << xx.size() << std::endl;
  }
}
#endif

int main(int, char **) {
#ifdef BUILD_DEV
  // buildFonts();
  // return 0;

  // File fileX("./assets/images/images.jpeg");

  // auto xx =
  //     fileX.readFileToByteArray("./assets/images/images.jpeg");

  // // write to file.

  // fileX.saveByteArrayToFile("./assets/images/images.png.txt",
  //                           xx, "LoaderBgImage");

  // std::cout << "xx.size() = " << xx.size() << std::endl;
#endif

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  // Setup window
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                        SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Window *window = SDL_CreateWindow(
      "Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
  bool err = glewInit() != GLEW_OK;
  if (err) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  CMenu::Get().Init(window, gl_context);

  CMenu::Get().StyleColorsMine();

  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Main loop
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    CMenu::Get().Render();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
