// Create Singleton class Menu that will handle all the ImGui stuff.
#include "imgui.h"
#include <SDL2/SDL.h>

enum class EMenuState { LOGIN, MAIN, SETTINGS, ABOUT, EXIT };
struct SMenuConfig {
  bool opacity = 1.f;
  float width = 400;
  float height = 400;

  // state
  EMenuState state = EMenuState::LOGIN;
  EMenuState lastState = EMenuState::LOGIN;
  float stateOpacity = 1.f;
};

struct SMenuFonts {
  ImFont *ifVerdana = nullptr;
  ImFont *ifVerdanaLoginTitle = nullptr;
};

class CMenu {
private:
  void RenderLoginBase();

public:
  CMenu() = default;
  ~CMenu() = default;

  // io
  ImGuiIO &io = ImGui::GetIO();
  bool bRenderInit = false;
  float fDpiScale = 1.0f;
  float fLastDpiScale = 1.0f;

  SMenuConfig *config = new SMenuConfig();
  SMenuFonts *fonts = new SMenuFonts();

  void Init(SDL_Window *window, SDL_GLContext gl_context);
  void StyleColorsMine(ImGuiStyle *dst = NULL);
  void Render();
  void Shutdown();

  static CMenu &Get() {
    static CMenu instance;
    return instance;
  }
};