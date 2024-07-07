// Create Singleton class Menu that will handle all the ImGui stuff.
#include "imgui.h"
#include <SDL2/SDL.h>
#include <vector>
#include <unordered_map>

enum class EMenuState { LOADING, LOGIN, MAIN, SETTINGS, ABOUT, EXIT };
struct SMenuConfig {
  bool opacity = 1.f;
  float width = 350;
  float height = 350;

  // state
  EMenuState state = EMenuState::LOADING;
  EMenuState lastState = EMenuState::LOADING;
  float stateOpacity = 1.f;
};

struct SMenuFonts {
  std::unordered_map<std::string, ImFont*> ifOpenSansBold;
  std::unordered_map<std::string, ImFont*> ifOpenSansExtraBold;
  std::unordered_map<std::string, ImFont*> ifOpenSansLight;
  std::unordered_map<std::string, ImFont*> ifOpenSansMedium;
  std::unordered_map<std::string, ImFont*> ifOpenSansRegular;

  ImFont* sex = nullptr;
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
  float fDpiScale = .8f;
  float fLastDpiScale = 1.0f;

  ImTextureID* loginRightImage = nullptr;
  ImTextureID* loginBgImage = nullptr;

  bool firstLoadInited = false;

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