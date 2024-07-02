#include "Menu.hpp"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <GL/glew.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdio.h>

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

void fnSpinnedMessageOverlay(const char *message = "Loading...") {
  auto cm = CMenu::Get();
  float radius = 16;
  float thickness = 3;
  float messagePadding = 10;

  ImGui::BeginChild("##overlay", {-1, -1});
  ImVec2 tsize = ImGui::CalcTextSize(message);
  imspaceMacro(cm.config->width / 2 - (radius + thickness),
               cm.config->height / 2 - (radius + thickness) - tsize.y / 2 -
                   messagePadding);
  Spinner("##spinner", radius, thickness, IM_COL32(20, 186, 40, 255));

  imspaceMacro(cm.config->width / 2 - (tsize.x / 2), messagePadding);
  ImGui::Text(message);

  ImGui::EndChild();
}
void fnRenderDebugMenu();
void fnPushMenuStyleVars();
void fnPopMenuStyleVars();

void CMenu::Init(SDL_Window *window, SDL_GLContext gl_context) {
  ImGuiIO &io = ImGui::GetIO();
  this->io = io;
  this->fonts->ifVerdana = io.Fonts->AddFontFromFileTTF(
      "assets/fonts/verdana.ttf", 18.0f * fDpiScale);

  io.FontDefault = this->fonts->ifVerdana;
  fonts->ifVerdanaLoginTitle = io.Fonts->AddFontFromFileTTF(
      "assets/fonts/verdana.ttf", 86.0f * fDpiScale);

  ImFontConfig config;
  config.OversampleH = 2;
  config.OversampleV = 1;
  config.GlyphExtraSpacing.x = .4f;
  fonts->ifVerdanaLoginTitle = io.Fonts->AddFontFromFileTTF(
      "assets/fonts/verdana.ttf", 60.0f * fDpiScale, &config);
}

void CMenu::StyleColorsMine(ImGuiStyle *dst) {
  ImVec4 backgroundColor =
      ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Koyu gri arka plan rengi

  ImVec4 baseColor = ImVec4(0.019f, 0.572f, 0.071f, 1.0f); // #059212 rengi

  ImVec4 buttonColor =
      ImVec4(0.019f, 0.572f, 0.071f, 0.6f); // Normal buton rengi
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
  {
    // Big Title
    imspaceMacro(0 * fDpiScale, 70 * fDpiScale);
    ImGui::PushFont(this->fonts->ifVerdanaLoginTitle);
    ImVec2 titleSize = ImGui::CalcTextSize("Go BRRRR");
    imspaceMacro((this->config->width * fDpiScale) / 2 - titleSize.x / 2, 0);
    ImGui::Text("Go BRRRR");
    ImGui::PopFont();
  }

  {
    float paddingY = 6 * fDpiScale;
    float spacingY = 10 * fDpiScale;
    float innerSpacingY = 10 * fDpiScale;
    // inputs
    ImGui::PushItemWidth(this->config->width * fDpiScale - 80 * fDpiScale);
    imspaceMacro(40 * fDpiScale, 30 * fDpiScale);
    ImGui::BeginGroup();

    {
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.5 * fDpiScale);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                          {8 * fDpiScale, paddingY});
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0 * fDpiScale, spacingY});
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
        if (ImGui::Button("Login",
                          {this->config->width * fDpiScale - 80 * fDpiScale,
                           40 * fDpiScale})) {
          printf("Login\n");
        }
      }

      ImGui::PopStyleVar(4);
    }

    ImGui::EndGroup();
    ImGui::PopItemWidth();
  }
}

void CMenu::Render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  if (!this->bRenderInit) {
    this->bRenderInit = true;
  }

  if (fLastDpiScale != fDpiScale) {
    fLastDpiScale = fDpiScale;
    ImGui::GetIO().FontGlobalScale = fDpiScale;
  }

  fnPushMenuStyleVars();
  ImGui::SetNextWindowSize({this->config->width * fDpiScale,
                            this->config->height * this->fDpiScale});
  // Render your GUI
  ImGui::Begin("Hello, world!s a asa", (bool *)false,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoTitleBar);
  {
    // // Kapatma düğmesi sağ üst köşede
    // imspaceMacro(this->config->width * fDpiScale - 30, 10 * fDpiScale);

    // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12 * fDpiScale);
    // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 0});
    // if (ImGui::Button("X", {20, 20})) {
    //   // Exit the all them all
    // }
    // ImGui::PopStyleVar(2);

    if (false)
      fnSpinnedMessageOverlay();
    else {
      if (this->config->state != config->lastState &&
          config->stateOpacity > 0) {
        config->stateOpacity -= .1;
      }

      if (config->lastState == config->state && config->stateOpacity < 1) {
        config->stateOpacity += .1;
      }

      if (config->state != config->lastState && config->stateOpacity <= 0) {
        config->lastState = config->state;
      }

      // render states in switch
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, config->stateOpacity);

      ImGui::BeginChild("##state", {this->config->width * fDpiScale,
                                    this->config->height * fDpiScale});
      {
        switch (config->lastState) {
        case EMenuState::LOGIN:
          RenderLoginBase();
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
          break;
        }
      }
      ImGui::EndChild();

      ImGui::PopStyleVar(1);

      ImClamp(config->stateOpacity, 0.f, 1.f);
    }
  }
  ImGui::End();

  // ImGui::SetNextWindowPos({0, 0});
  // ImGui::SetNextWindowSize({160, 100});
  fnPopMenuStyleVars();

  fnRenderDebugMenu();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

  ImGui::End();
}

void fnPushMenuStyleVars() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8 * CMenu::Get().fDpiScale);
  ImGui::PushFont(CMenu::Get().fonts->ifVerdana);
  ImGui::SetNextWindowBgAlpha(.97);
}

void fnPopMenuStyleVars() {
  ImGui::PopFont();
  ImGui::PopStyleVar(2);
}