// Create Singleton class Menu that will handle all the ImGui stuff.
#include "imgui.h"
#include <SDL2/SDL.h>

struct SMenuConfig {
  bool opacity = 1.f;
  bool show_demo_window = false;
  float width = 800;
  float height = 600;
};

struct SMenuFonts {
  ImFont *ifVerdana = nullptr;
};

class CMenu {
private:
  void RenderSidebar();
  void RenderTopBar();
  void RenderContent();
  void RenderCheatSettings();
  void RenderWatermark();

public:
  CMenu() = default;
  ~CMenu() = default;

  // io
  ImGuiIO &io = ImGui::GetIO();
  bool bRenderInit = false;
  float fDpiScale = 1.0f;

  SMenuConfig *config = new SMenuConfig();
  SMenuFonts *fonts = new SMenuFonts();

  void Init(SDL_Window *window, SDL_GLContext gl_context);
  void Render();
  void Shutdown();

  static CMenu &Get() {
    static CMenu instance;
    return instance;
  }
};