#include "Menu.hpp"
#include "./../utils/File.hpp"
#include "./fonts/OpenSansBold.byte.hpp"
#include "./fonts/OpenSansExtraBold.byte.hpp"
#include "./fonts/OpenSansLight.byte.hpp"
#include "./fonts/OpenSansMedium.byte.hpp"
#include "./fonts/OpenSansRegular.byte.hpp"
#include "./images/Character.hpp"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <GL/glew.h>
#include <SDL_stdinc.h>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <sys/_types/_uintptr_t.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef HLERP
#define HLERP(a, b, t) ((a) + (t) * ((b) - (a)))
#endif

void fnRenderDebugMenu();
void fnPushMenuStyleVars();
void fnPopMenuStyleVars();

// Simple helper function to load an image from a byte array into an OpenGL
// texture with common settings
bool LoadTextureFromByteArray(const unsigned char *image_data, int data_size,
                              GLuint *out_texture, int *out_width,
                              int *out_height) {
  if (image_data == NULL)
    return false;

  int image_width, image_height, channels;
  unsigned char *decoded_data = stbi_load_from_memory(
      image_data, data_size, &image_width, &image_height, &channels, 4);
  if (decoded_data == NULL)
    return false;

  // Create an OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE); // This is required on WebGL for non
                                     // power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

  // Set pixel storage mode
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, decoded_data);

  stbi_image_free(decoded_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

bool afterTime(float time = 1, bool *affected = nullptr) {
  // Başlangıç zamanını al
  static auto start = std::chrono::high_resolution_clock::now();
  static auto previous = start;

  auto current = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> elapsed = current - start;
  std::chrono::duration<float> deltaTime = current - previous;

  // İlerleme kontrolü
  previous = current;

  // 3 saniye geçtikten sonra işlemi gerçekleştir
  if (elapsed.count() >= time && !*affected) {
    *affected = true;
    return true;
  }
  return false;
}

void imspaceMacro(float x, float y) {
  ImGui::SetCursorPos(
      ImVec2(ImGui::GetCursorPos().x + x, ImGui::GetCursorPos().y + y));
}

bool Spinner(const char *label, float radius, int thickness,
             const ImU32 &color) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext &g = *GImGui;
  const ImGuiStyle &style = g.Style;
  const ImGuiID id = window->GetID(label);

  ImVec2 pos = window->DC.CursorPos;
  ImVec2 size((radius)*2, (radius + style.FramePadding.y) * 2);

  const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
  ImGui::ItemSize(bb, style.FramePadding.y);
  if (!ImGui::ItemAdd(bb, id))
    return false;

  // Render
  window->DrawList->PathClear();

  int num_segments = 30;
  int start = abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

  const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
  const float a_max =
      IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

  const ImVec2 centre =
      ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

  for (int i = 0; i < num_segments; i++) {
    const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
    window->DrawList->PathLineTo(
        ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
               centre.y + ImSin(a + g.Time * 8) * radius));
  }

  window->DrawList->PathStroke(color, false, thickness);

  return true;
}

void CMenu::Init(SDL_Window *window, SDL_GLContext gl_context) {
  ImGuiIO &io = ImGui::GetIO();
  this->io = io;
  struct SFRGC {
    SFRGC(const char *name, float size) {
      this->name = name;
      this->size = size;
    }
    const char *name;
    float size;
  };

  SFRGC fontSizes[] = {SFRGC("sm", 14),  SFRGC("base", 16), SFRGC("lg", 18),
                       SFRGC("xl", 20),  SFRGC("2xl", 24),  SFRGC("3xl", 30),
                       SFRGC("4xl", 36), SFRGC("5xl", 48),  SFRGC("6xl", 60),
                       SFRGC("7xl", 72), SFRGC("8xl", 96),  SFRGC("9xl", 128)};

  for (auto &_font : fontSizes) {
    this->fonts->ifOpenSansLight[_font.name] = io.Fonts->AddFontFromMemoryTTF(
        FontBytes::OpenSansLight, FontBytes::OpenSansLight_size, _font.size);

    this->fonts->ifOpenSansRegular[_font.name] = io.Fonts->AddFontFromMemoryTTF(
        FontBytes::OpenSansRegular, FontBytes::OpenSansRegular_size,
        _font.size);

    this->fonts->ifOpenSansMedium[_font.name] = io.Fonts->AddFontFromMemoryTTF(
        FontBytes::OpenSansMedium, FontBytes::OpenSansMedium_size, _font.size);

    this->fonts->ifOpenSansBold[_font.name] = io.Fonts->AddFontFromMemoryTTF(
        FontBytes::OpenSansBold, FontBytes::OpenSansBold_size, _font.size);

    this->fonts->ifOpenSansExtraBold[_font.name] =
        io.Fonts->AddFontFromMemoryTTF(FontBytes::OpenSansExtraBold,
                                       FontBytes::OpenSansExtraBold_size,
                                       _font.size);
  }

  int w, h;
  w = 512;
  h = 384;
  GLuint xtexture = 0;

  if (LoadTextureFromByteArray(ImageBytes::sir_bloody_miami_darryl_png,
                               sizeof(ImageBytes::sir_bloody_miami_darryl_png),
                               &xtexture, &w, &h)) {
    this->loginRightImage = new ImTextureID((void *)(uintptr_t)xtexture);
    // Successfully loaded texture
    std::cout << "Successfully loaded texture" << std::endl;
  } else {
    // Failed to load texture
    std::cout << "Failed to load texture" << std::endl;
  }

  w = 300;
  h = 168;
  xtexture = 0;

  if (LoadTextureFromByteArray(ImageBytes::LoaderBgImage,
                               sizeof(ImageBytes::LoaderBgImage), &xtexture, &w,
                               &h)) {
    this->loginBgImage = new ImTextureID((void *)(uintptr_t)xtexture);
    // Successfully loaded texture
    std::cout << "Successfully loaded texture" << std::endl;
  } else {
    // Failed to load texture
    std::cout << "Failed to load texture" << std::endl;
  }

  std::cout << this->fonts->ifOpenSansBold.size() << std::endl;

  ImFontConfig config;
  config.OversampleH = 2;
  config.OversampleV = 1;
  config.GlyphExtraSpacing.x = .4f;

  io.FontDefault = fonts->ifOpenSansMedium["xl"];
}

void CMenu::StyleColorsMine(ImGuiStyle *dst) {
  ImVec4 backgroundColor =
      ImVec4(0.06f, 0.06f, 0.06f, 1.0f); // Koyu gri arka plan rengi

  ImVec4 baseColor = ImVec4(0.019f, 0.572f, 0.071f, 1.0f); // #059212 rengi

  ImVec4 buttonColor = ImVec4(0.16f, 0.16f, 0.16f, 1.0f); // Normal buton rengi
  ImVec4 buttonHovered = ImVec4(0.019f, 0.572f, 0.071f, 1.0f); // Hover durumu
  ImVec4 buttonActive = ImVec4(0.012f, 0.412f, 0.051f, 1.0f); // Tıklanmış durum

  ImVec4 frameBgColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Koyu gri
  ImVec4 frameBgHoveredColor =
      ImVec4(0.25f, 0.25f, 0.25f, 1.0f); // Hover durumunda koyu gri
  ImVec4 frameBgActiveColor =
      ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Tıklanmış durumda koyu gri

  ImGuiStyle *style = dst ? dst : &ImGui::GetStyle();
  ImVec4 *colors = style->Colors;

  colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = backgroundColor;
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // colors[ImGuiCol_FrameBg] = ImVec4(0.2, 0.2, 0.2, 0.6);
  // colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2, 0.2, 0.2, 0.6);
  // colors[ImGuiCol_FrameBgActive] = ImVec4(0.2, 0.2, 0.2, 0.6);

  colors[ImGuiCol_FrameBg] = frameBgColor;
  colors[ImGuiCol_FrameBgHovered] = frameBgHoveredColor;
  colors[ImGuiCol_FrameBgActive] = frameBgActiveColor;

  colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Button] = buttonColor;
  colors[ImGuiCol_ButtonHovered] = buttonHovered;
  colors[ImGuiCol_ButtonActive] = buttonActive;
  colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
  colors[ImGuiCol_Tab] =
      ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
  colors[ImGuiCol_TabSelected] = ImLerp(colors[ImGuiCol_HeaderActive],
                                        colors[ImGuiCol_TitleBgActive], 0.60f);
  colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
  colors[ImGuiCol_TabDimmed] =
      ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
  colors[ImGuiCol_TabDimmedSelected] =
      ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
  colors[ImGuiCol_TabDimmedSelectedOverline] =
      ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] =
      ImVec4(0.31f, 0.31f, 0.35f, 1.00f); // Prefer using Alpha=1.0 here
  colors[ImGuiCol_TableBorderLight] =
      ImVec4(0.23f, 0.23f, 0.25f, 1.00f); // Prefer using Alpha=1.0 here
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void CMenu::RenderLoginBase() {

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::BeginChild("##login-left", {(config->width / 2) * fDpiScale, -1}, 0,
                    ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse);
  {
    {
      // Big Title
      ImGui::PushFont(this->fonts->ifOpenSansExtraBold["7xl"]);
      ImVec2 titleSize = ImGui::CalcTextSize("Go BRRRR");
      imspaceMacro(((this->config->width * fDpiScale) / 2) / 2 -
                       titleSize.x / 2,
                   30 * fDpiScale);
      ImGui::Text("Go BRRRR");
      ImGui::PopFont();
    }

    {
      float paddingY = 6 * fDpiScale;
      float spacingY = 10 * fDpiScale;
      float innerSpacingY = 10 * fDpiScale;
      // inputs
      ImGui::PushItemWidth((this->config->width / 2) * fDpiScale -
                           70 * fDpiScale);
      imspaceMacro(40 * fDpiScale, 20 * fDpiScale);
      ImGui::BeginGroup();

      {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.5 * fDpiScale);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                            {8 * fDpiScale, paddingY});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            {0 * fDpiScale, spacingY});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,
                            {0 * fDpiScale, innerSpacingY});
        {

          static char username[128] = "sasa";
          static char password[128] = "sasa";
          ImGui::TextColored(ImVec4(.55, .55, .55, .55), "Username");
          ImGui::InputText("##username", username, IM_ARRAYSIZE(username),
                           ImGuiInputTextFlags_AutoSelectAll);
          ImGui::TextColored(ImVec4(.55, .55, .55, .55), "Password");
          ImGui::InputText("##password", password, IM_ARRAYSIZE(password),
                           ImGuiInputTextFlags_Password |
                               ImGuiInputTextFlags_AutoSelectAll);

          imspaceMacro(0 * fDpiScale, 12 * fDpiScale);
          if (ImGui::Button("Login", {(config->width / 2 - 70) * fDpiScale,
                                      55 * fDpiScale})) {
            printf("Login\n");
          }
        }

        ImGui::PopStyleVar(4);
      }

      ImGui::EndGroup();
      ImGui::PopItemWidth();
    }
  }
  ImGui::EndChild();

  ImGui::SameLine(0, 0); // Aynı satırda devam ediyor, boşluk olmadan

  // SetCursorPosX ile pozisyonu ayarla
  float posX = ImGui::GetCursorPosX();
  ImGui::SetCursorPosX(posX + ImGui::GetStyle().ItemSpacing.x);

  ImGui::BeginChild(
      "##login-right", {(this->config->width / 2) * fDpiScale, -1}, 0,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  {
    // image

    float hfW = config->width / 2;
    float hfH = config->height * 1.17;

    float iWidth = 512;
    float iHeight = 384;

    float cWidth = iWidth + hfH - iHeight;
    float cHeight = hfH;

    imspaceMacro(-89 * fDpiScale, -10 * fDpiScale);
    ImGui::Image(*loginRightImage, {cWidth * fDpiScale, cHeight * fDpiScale});
  }
  ImGui::EndChild();
  ImGui::PopStyleVar(2);

  // imspaceMacro(0 * fDpiScale, 0 * fDpiScale);
  // ImGui::Image(*loginBgImage,
  //              {config->width * fDpiScale, config->height * fDpiScale});
}

void loadingPage() {
  // set bginChild alpha
  ImGui::BeginChild("##xxoverlayZSWEF", {-1, -1});

  imspaceMacro((CMenu::Get().config->width * CMenu::Get().fDpiScale) / 2 - 18,
               (CMenu::Get().config->height * CMenu::Get().fDpiScale) / 2 - 18);
  Spinner("sex", 22, 4, ImColor(80, 200, 80, 255));

  ImGui::EndChild();
}

void CMenu::Render() {
  static GLuint texture = 0;

  if (!this->bRenderInit) {

    this->bRenderInit = true;
  }

  if (fLastDpiScale != fDpiScale) {
    fLastDpiScale = fDpiScale;
    ImGui::GetIO().FontGlobalScale = fDpiScale;
  }

  fnPushMenuStyleVars();

  static float width, height = 200 * fDpiScale;
  width = HLERP(width, this->config->width * fDpiScale, io.DeltaTime * 7);
  height = HLERP(height, this->config->height * fDpiScale, io.DeltaTime * 7);
  ImGui::SetNextWindowSize({width, height});

  // Render your GUI
  ImGui::Begin("##main menu homebase", (bool *)false,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse);
  {
    // // Kapatma düğmesi sağ üst köşede
    // imspaceMacro(this->config->width * fDpiScale - 30, 10 * fDpiScale);

    // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12 * fDpiScale);
    // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 0});
    // if (ImGui::Button("X", {20, 20})) {
    //   // Exit the all them all
    // }
    // ImGui::PopStyleVar(2);

    if (this->config->state != config->lastState && config->stateOpacity > 0) {
      config->stateOpacity -= .1f;
    }

    if (config->lastState == config->state && config->stateOpacity < 1) {
      config->stateOpacity += .1;
    }

    if (config->state != config->lastState && config->stateOpacity <= 0) {
      config->lastState = config->state;
    }

    // render states in switch
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, config->stateOpacity);
    ImGui::BeginChild("##state", {-1, -1}, 0,
                      ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoScrollWithMouse);

    static bool wfst, wfst2 = false;
    if (afterTime(3, &wfst)) {
      config->width = 750;
      config->height = 400;
    } else if (afterTime(4, &wfst2) && wfst) {
      config->state = EMenuState::LOGIN;
    }

    if (config->stateOpacity <= .8f && config->lastState == config->state) {
      switch (config->lastState) {
      case EMenuState::LOGIN: {
        config->width = 750;
        config->height = 400;
      } break;
      case EMenuState::MAIN:
        config->width = 200;
        config->height = 200;
        break;
      case EMenuState::EXIT:
        config->width = 200;
        config->height = 200;
        break;
      }
    }

    {
      switch (config->lastState) {
      case EMenuState::LOADING:
        loadingPage();
        break;
      case EMenuState::LOGIN:
        this->RenderLoginBase();
        break;
      case EMenuState::MAIN: {
        ImGui::Text("Hello From MAIN");
      } break;
      case EMenuState::SETTINGS: {
        ImGui::Text("Hello From SETTINGS");
      } break;
      case EMenuState::ABOUT: {
        ImGui::Text("Hello From ABOUT");
      } break;
      case EMenuState::EXIT:
        loadingPage();
        break;
      }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(1);
  }
  ImGui::End();

  // ImGui::SetNextWindowPos({0, 0});
  // ImGui::SetNextWindowSize({160, 100});
  fnPopMenuStyleVars();

  fnRenderDebugMenu();
  ImClamp(config->stateOpacity, 0.f, 1.f);
}

void CMenu::Shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void fnRenderDebugMenu() {
  ImGui::Begin("Debug Menu");

  ImGui::SliderFloat("DPI Size", &CMenu::Get().fDpiScale, 0, 3);

  Spinner("sex564qw6e46qw4e", 16, 4,
          IM_COL32(0.019f * 255, 0.572f * 255, 0.071f * 255, 1.f * 255));

  if (ImGui::Button("state login")) {
    CMenu::Get().config->state = EMenuState::LOGIN;
  }

  if (ImGui::Button("state main")) {
    CMenu::Get().config->state = EMenuState::MAIN;
  }

  if (ImGui::Button("state settings")) {
    CMenu::Get().config->state = EMenuState::SETTINGS;
  }

  if (ImGui::Button("state about")) {
    CMenu::Get().config->state = EMenuState::ABOUT;
  }

  if (ImGui::Button("state EXIT")) {
    CMenu::Get().config->state = EMenuState::EXIT;
  }

  ImGui::End();
}

void fnPushMenuStyleVars() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8 * CMenu::Get().fDpiScale);
  ImGui::SetNextWindowBgAlpha(.99);
}

void fnPopMenuStyleVars() { ImGui::PopStyleVar(2); }