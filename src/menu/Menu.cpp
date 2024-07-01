#include "Menu.hpp"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <GL/glew.h>
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

void fnRenderDebugMenu();
void fnPushMenuStyleVars();
void fnPopMenuStyleVars();

void CMenu::Init(SDL_Window *window, SDL_GLContext gl_context) {
  ImGuiIO &io = ImGui::GetIO();
  this->io = io;
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  return;
  // load fonts here
  this->fonts->ifVerdana = io.Fonts->AddFontFromFileTTF(
      "assets/fonts/verdana.ttf", 18.0f * fDpiScale);

  io.FontDefault = this->fonts->ifVerdana;
}

void CMenu::RenderWatermark() {
  ImGui::SetNextWindowPos({20 * fDpiScale, 20 * fDpiScale});
  ImGui::SetNextWindowSize({160 * fDpiScale, 100 * fDpiScale});
  ImGui::Begin("##Watermark", (bool *)false,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoTitleBar);
  { ImGui::Text("Hello From America"); }
  ImGui::End();
}

void CMenu::RenderSidebar() {
  ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(40, 100, 30, 120));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(40, 160, 30, 185));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1500 * fDpiScale);
  {
    ImGui::BeginChild("##SideBar", ImVec2(80 * fDpiScale, -1));
    imspaceMacro(10 * fDpiScale, 10 * fDpiScale);
    ImGui::Button("RB", {(80 - 20) * fDpiScale, 60 * fDpiScale});
    imspaceMacro(10 * fDpiScale, 10 * fDpiScale);
    ImGui::Button("AA", {(80 - 20) * fDpiScale, 60 * fDpiScale});
    imspaceMacro(10 * fDpiScale, 10 * fDpiScale);
    ImGui::Button("LB", {(80 - 20) * fDpiScale, 60 * fDpiScale});
    imspaceMacro(10 * fDpiScale, 10 * fDpiScale);
    ImGui::Button("VI", {(80 - 20) * fDpiScale, 60 * fDpiScale});
    imspaceMacro(10 * fDpiScale, 10 * fDpiScale);
    ImGui::Button("SX", {(80 - 20) * fDpiScale, 60 * fDpiScale});
    ImGui::EndChild();
  }
  ImGui::PopStyleVar(1);
  ImGui::PopStyleColor(3);
}

void CMenu::RenderTopBar() {
  ImGui::BeginChild("##TopBar", ImVec2(-1, 100 * fDpiScale));
  ImGui::Text("Hello, world!");
  ImGui::EndChild();
}

void CMenu::RenderContent() {
  ImGui::BeginChild("##ContentSide", ImVec2(-1, 100 * fDpiScale));
  ImGui::Text("Hello, Contest!");
  ImGui::EndChild();
}

void CMenu::Render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  this->RenderWatermark();

  if (!this->bRenderInit) {
    this->bRenderInit = true;
  }

  fnPushMenuStyleVars();
  ImGui::SetNextWindowSize({this->config->width * fDpiScale,
                            this->config->height * this->fDpiScale});
  // Render your GUI
  ImGui::Begin("Hello, world!s a asa", (bool *)false,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoTitleBar);
  {
    this->RenderSidebar();
    ImGui::SameLine();
    ImGui::BeginChild("##ContentBaser", ImVec2(-1, -1));
    {
      this->RenderTopBar();
      this->RenderContent();
    }
    ImGui::EndChild();
  }
  ImGui::End();

  //ImGui::SetNextWindowPos({0, 0});
  ImGui::SetNextWindowSize({160, 100});
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

  Spinner("sex564qw6e46qw4e", 10, 2, IM_COL32(255, 255, 255, 255));

  ImGui::End();
}

void fnPushMenuStyleVars() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                      12 * CMenu::Get().fDpiScale);
  ImGui::PushFont(CMenu::Get().fonts->ifVerdana);
  ImGui::SetNextWindowBgAlpha(.97);
}

void fnPopMenuStyleVars() {
  ImGui::PopFont();
  ImGui::PopStyleVar(2);
}