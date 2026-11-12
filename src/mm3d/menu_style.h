// SPDX-License-Identifier: GPL-2.0-or-later
// Settings menu presentation, adapted from Majora ReMasked (OTPR26/DekUI-Fork,
// src/mm3d/android_menu_style.h). Setting storage and game actions stay in
// overlay.cpp. Applies to packaged desktop builds; the developer editor and
// Android keep the default ImGui widgets.
#pragma once
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_internal.h>

#if defined(ENABLE_MM3D_GAME_FOLDER) && !defined(__ANDROID__)
#define MM3D_STYLED_MENU 1
#endif

namespace MM3D::MenuStyle {
inline constexpr ImVec4 Accent{.725f,.49f,.95f,1};
inline constexpr ImVec4 Muted{.69f,.65f,.74f,1};

inline void Theme(ImGuiStyle& s) {
    s.WindowPadding={12,10}; s.FramePadding={8,6}; s.ItemSpacing={8,9};
    s.ItemInnerSpacing={8,6}; s.ScrollbarSize=16; s.GrabMinSize=15;
    s.WindowRounding=0; s.ChildRounding=5; s.FrameRounding=3;
    s.PopupRounding=4; s.ScrollbarRounding=6; s.GrabRounding=8;
    s.WindowBorderSize=0; s.ChildBorderSize=0; s.FrameBorderSize=1;
    auto& c=s.Colors;
    c[ImGuiCol_Text]={.95f,.95f,.95f,1};
    c[ImGuiCol_TextDisabled]={.55f,.52f,.61f,1};
    c[ImGuiCol_WindowBg]={.032f,.028f,.051f,1};
    c[ImGuiCol_ChildBg]={.064f,.05f,.09f,1};
    c[ImGuiCol_PopupBg]={.095f,.07f,.14f,1};
    c[ImGuiCol_Border]={.28f,.22f,.35f,.6f};
    c[ImGuiCol_FrameBg]={.14f,.11f,.19f,1};
    c[ImGuiCol_FrameBgHovered]={.24f,.13f,.35f,1};
    c[ImGuiCol_FrameBgActive]={.28f,.12f,.45f,1};
    c[ImGuiCol_Button]={.16f,.11f,.22f,1};
    c[ImGuiCol_ButtonHovered]={.30f,.15f,.44f,1};
    c[ImGuiCol_ButtonActive]={.36f,.19f,.52f,1};
    c[ImGuiCol_Header]={.24f,.08f,.41f,1};
    c[ImGuiCol_HeaderHovered]={.29f,.12f,.46f,1};
    c[ImGuiCol_HeaderActive]={.32f,.14f,.49f,1};
    c[ImGuiCol_Tab]={.14f,.11f,.19f,1};
    c[ImGuiCol_TabHovered]={.30f,.15f,.44f,1};
    c[ImGuiCol_TabSelected]={.24f,.08f,.41f,1};
    c[ImGuiCol_CheckMark]=c[ImGuiCol_SliderGrab]=Accent;
    c[ImGuiCol_SliderGrabActive]={.9f,.73f,1,1};
    c[ImGuiCol_Separator]={.25f,.18f,.32f,1};
    c[ImGuiCol_ScrollbarBg]={0,0,0,0};
    c[ImGuiCol_ScrollbarGrab]={.35f,.25f,.43f,1};
    c[ImGuiCol_NavCursor]=Accent;
}

// Visible part of an ImGui label ("Text##id" -> "Text").
inline const char* LabelEnd(const char* label) {
    const char* hash=std::strstr(label,"##");
    return hash?hash:label+std::strlen(label);
}

// Menu size relative to a 720p window, times the user's Settings size.
inline float ContentScale(float height,float user_scale) {
    return std::clamp(height/720.f,.65f,4.f)*1.35f*std::clamp(user_scale,.5f,2.f);
}
// System fonts are loaded at FontPixels and drawn at the built-in font's 13px
// baseline before ContentScale, so layouts match the fallback font.
inline constexpr float FontPixels=52.f, FontScale=13.f/FontPixels;
inline bool LoadSystemFont(ImGuiIO& io) {
    std::vector<std::string> candidates;
#ifdef _WIN32
    const char* windir=std::getenv("WINDIR");
    const std::string fonts=std::string(windir&&*windir?windir:"C:\\Windows")+"\\Fonts\\";
    for(const char* name:{"segoeui.ttf","arial.ttf","tahoma.ttf"})candidates.push_back(fonts+name);
#elif defined(__APPLE__)
    candidates={"/System/Library/Fonts/Supplemental/Arial.ttf","/Library/Fonts/Arial.ttf",
                "/System/Library/Fonts/Supplemental/Verdana.ttf"};
#else
    candidates={"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf","/usr/share/fonts/TTF/DejaVuSans.ttf",
                "/usr/share/fonts/dejavu/DejaVuSans.ttf","/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
                "/usr/share/fonts/noto/NotoSans-Regular.ttf","/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
                "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"};
#endif
    for(const auto& path:candidates) {
        std::FILE* file=std::fopen(path.c_str(),"rb");
        if(!file)continue;
        std::fclose(file);
        // Default glyph ranges include Latin-1 for the French and Spanish text.
        if(io.Fonts->AddFontFromFileTTF(path.c_str(),FontPixels))return true;
    }
    return false;
}

inline float ControlWidth() {
    return std::min(ImGui::GetContentRegionAvail().x,ImGui::GetFontSize()*28.f);
}

// Full-width row with the label on the left and On/Off (or a radio dot) on the right.
inline bool Checkbox(const char* label,bool* value,bool radio=false) {
#ifdef MM3D_STYLED_MENU
    const float f=ImGui::GetFontSize(), w=ControlWidth();
    const float pad=f*.55f, toggle=f*3.f;
    const float text_w=std::max(f*2,w-toggle-pad*3);
    const auto text=ImGui::CalcTextSize(label,nullptr,true,text_w);
    const float h=std::max(f*2.1f,text.y+pad*2);
    const auto p=ImGui::GetCursorScreenPos();
    const bool clicked=ImGui::InvisibleButton(label,{w,h},ImGuiButtonFlags_EnableNav);
    if(clicked)*value=!*value;
    auto* d=ImGui::GetWindowDrawList();
    const bool hovered=ImGui::IsItemHovered();
    d->AddRectFilled(p,{p.x+w,p.y+h},ImGui::GetColorU32(hovered?ImGuiCol_FrameBgHovered:ImGuiCol_ChildBg),f*.3f);
    d->AddRect(p,{p.x+w,p.y+h},ImGui::GetColorU32(ImGuiCol_Border),f*.3f);
    if(ImGui::IsItemFocused())d->AddRect(p,{p.x+w,p.y+h},ImGui::GetColorU32(Accent),f*.2f,0,2);
    const char* end=LabelEnd(label);
    d->AddText(ImGui::GetFont(),f,{p.x+pad,p.y+(h-text.y)*.5f},
        ImGui::GetColorU32(ImGuiCol_Text),label,end,text_w);
    const ImVec2 a{p.x+w-toggle-pad,p.y+(h-f*.85f)*.5f};
    if(radio) {
        const ImVec2 center{a.x+toggle-f*.425f,a.y+f*.425f};
        d->AddCircle(center,f*.40f,ImGui::GetColorU32(Accent),0,2);
        if(*value)d->AddCircleFilled(center,f*.24f,ImGui::GetColorU32(Accent));
    } else {
        const char* state=*value?"On":"Off";
        const auto state_size=ImGui::CalcTextSize(state);
        d->AddText({p.x+w-pad-state_size.x,p.y+(h-state_size.y)*.5f},
                   ImGui::GetColorU32(*value?Accent:Muted),state);
    }
    return clicked;
#else
    return ImGui::Checkbox(label,value);
#endif
}

inline void Label(const char* label) {
    const float width=ControlWidth();
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX()+width);
    ImGui::TextUnformatted(label,LabelEnd(label));
    ImGui::PopTextWrapPos();
    ImGui::SetNextItemWidth(width);
}
// Use in place of SameLine() after a styled control; styled controls span the row.
inline void AfterControl() {
#ifndef MM3D_STYLED_MENU
    ImGui::SameLine();
#endif
}
inline bool RadioButton(const char* label,int* v,int choice) {
#ifdef MM3D_STYLED_MENU
    bool selected=*v==choice;
    if(Checkbox(label,&selected,true)){*v=choice;return true;}
    return false;
#else
    return ImGui::RadioButton(label,v,choice);
#endif
}
inline bool SliderInt(const char* label,int* v,int min,int max,const char* format="%d",ImGuiSliderFlags flags=0) {
#ifdef MM3D_STYLED_MENU
    ImGui::PushID(label); Label(label);
    const bool changed=ImGui::SliderInt("##value",v,min,max,format,flags);
    ImGui::PopID();return changed;
#else
    return ImGui::SliderInt(label,v,min,max,format,flags);
#endif
}
inline bool SliderFloat(const char* label,float* v,float min,float max,const char* format="%.3f",ImGuiSliderFlags flags=0) {
#ifdef MM3D_STYLED_MENU
    ImGui::PushID(label); Label(label);
    const bool changed=ImGui::SliderFloat("##value",v,min,max,format,flags);
    ImGui::PopID();return changed;
#else
    return ImGui::SliderFloat(label,v,min,max,format,flags);
#endif
}
inline bool Combo(const char* label,int* v,const char* items,int height=-1) {
#ifdef MM3D_STYLED_MENU
    ImGui::PushID(label);Label(label);
    const bool changed=ImGui::Combo("##value",v,items,height);
    ImGui::PopID();return changed;
#else
    return ImGui::Combo(label,v,items,height);
#endif
}

// Pages in rail order: id used by begin_page(), rail title, description.
// Keep "Diagnostics" last; it is only listed when DEKUI_DIAGNOSTICS=1.
// tools/mm3d/reapply_menu_style.py inserts new pages above "Diagnostics".
struct PageInfo {const char* id; const char* title; const char* description;};
inline constexpr PageInfo Pages[]={
    {"General","General","Make Dekui comfortable on your screen."},
    {"Controls","Controls","Map your controller and game controls."},
    {"Graphics","Graphics","Frame rate, resolution and custom textures."},
    {"Audio","Audio","Sound, music and volume."},
    {"HUD","HUD","Adjust the HUD while keeping your chosen layout."},
    {"Difficulty","Difficulty","Damage, healing and world rules."},
    {"Restoration","Restoration","Choose your Project Restoration features."},
    {"QOL","Quality of life","Small changes for a more comfortable adventure."},
    {"About","About","About Dekui and its contributors."},
    {"Diagnostics","Diagnostics","Performance capture for bug reports."},
};
inline constexpr int PageCount(bool diagnostics) {
    return int(std::size(Pages))-(diagnostics?0:1);
}

// Controller/keyboard focus between the page rail and the page content.
// ImGui only moves left/right between items that line up horizontally, which
// the rail and the settings rows don't, so these jumps are explicit.
struct NavState {
    bool focus_rail=false, focus_content=false, show_cursor=false, was_content=false;
    ImGuiID left_from=0; // content item where Left was pressed last frame
    ImGuiWindow* rail=nullptr;
};
inline NavState nav;
inline bool NavPressed(ImGuiKey gamepad,ImGuiKey keyboard) {
    return ImGui::IsKeyPressed(gamepad,false)||ImGui::IsKeyPressed(keyboard,false);
}

struct Layout {float f,gap,header,footer,rail,body;};
inline Layout Measure(int width,int height) {
    Layout l{};
    l.f=ImGui::GetFontSize();
    l.gap=std::max({12.f,float(height)*.018f,ImGui::GetStyle().WindowPadding.x,ImGui::GetStyle().WindowPadding.y});
    l.header=std::clamp(l.f*2.7f,float(height)*.095f,float(height)*.18f);
    l.footer=std::clamp(l.f*2.4f,float(height)*.09f,float(height)*.15f);
    l.rail=std::clamp(l.f*8.8f,float(width)*.22f,float(width)*.32f);
    l.body=float(height)-l.header-l.footer-l.gap*3;
    return l;
}

// Fixed header/footer, independently scrolling page rail and content. Call
// inside a full-screen window; pair with End().
inline void Begin(int& page,int count,int width,int height,bool& resume,bool& quit) {
    count=std::clamp(count,1,int(std::size(Pages)));
    if(page<0||page>=count)page=0;
    if(ImGui::IsKeyPressed(ImGuiKey_PageUp))page=(page+count-1)%count;
    if(ImGui::IsKeyPressed(ImGuiKey_PageDown))page=(page+1)%count;
    const auto l=Measure(width,height);
    ImGuiContext& g=*GImGui;
    // Left in the content that ImGui couldn't use (NavId unchanged since the
    // press) returns to the rail.
    if(nav.left_from && g.NavId==nav.left_from && !ImGui::IsAnyItemActive())nav.focus_rail=true;
    // Any move from the content into the rail (Left or Up at the edge) lands
    // on the current page rather than whichever entry happens to line up.
    if(nav.was_content && nav.rail && g.NavWindow==nav.rail)nav.focus_rail=true;
    nav.left_from=0;
    // ImGui hides the controller highlight after a programmatic focus change,
    // and A only acts on a highlighted item.
    if(nav.show_cursor){nav.show_cursor=false;g.NavCursorVisible=true;g.NavHighlightItemUnderNav=true;}
    ImGui::SetCursorPos({l.gap,l.gap});
    ImGui::TextColored(Accent,"Dekui");ImGui::SameLine();ImGui::TextUnformatted("/ Settings");
    ImGui::SetCursorPos({float(width)-l.gap-l.f*5.5f,l.gap});
    resume=ImGui::Button("Resume",{l.f*5.5f,l.header-l.gap});
    ImGui::SetCursorPos({l.gap,l.header+l.gap});
    ImGui::BeginChild("Categories",{l.rail,l.body},ImGuiChildFlags_NavFlattened);
    nav.rail=ImGui::GetCurrentWindow();
    if(ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() &&
       NavPressed(ImGuiKey_GamepadDpadRight,ImGuiKey_RightArrow))nav.focus_content=true;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,0);
    for(int i=0;i<count;++i) {
        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button,i==page?ImVec4{.24f,.08f,.41f,1}:ImVec4{0,0,0,0});
        if(ImGui::Button(Pages[i].title,{ImGui::GetContentRegionAvail().x,std::max(l.f*1.9f,float(height)*.065f)}))page=i;
        if(i==page) {
            ImGui::SetItemDefaultFocus();
            if(nav.focus_rail) {
                nav.focus_rail=false;
                ImGui::SetFocusID(ImGui::GetItemID(),ImGui::GetCurrentWindow());
                ImGui::ScrollToItem();
                nav.show_cursor=true;
            }
        }
        ImGui::PopStyleColor();ImGui::PopID();
    }
    ImGui::PopStyleVar();ImGui::EndChild();
    ImGui::SetCursorPos({l.gap,l.header+l.gap+l.body+l.gap});
    quit=ImGui::Button("Quit game",{l.rail,l.footer-l.gap});
    ImGui::SetCursorPos({l.rail+l.gap*2,l.header+l.gap});
    ImGui::BeginChild("Settings panel",{float(width)-l.rail-l.gap*3,l.body},ImGuiChildFlags_AlwaysUseWindowPadding|ImGuiChildFlags_NavFlattened);
    ImGui::TextColored(Accent,"%s",Pages[page].title);
    ImGui::PushStyleColor(ImGuiCol_Text,Muted);
    ImGui::TextWrapped("%s",Pages[page].description);
    ImGui::PopStyleColor();ImGui::Separator();
    const std::string id=std::string("Page/")+Pages[page].id;
    ImGui::BeginChild(id.c_str(),{0,0},ImGuiChildFlags_NavFlattened);
    if(ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && !ImGui::IsAnyItemActive() &&
       NavPressed(ImGuiKey_GamepadDpadLeft,ImGuiKey_LeftArrow))nav.left_from=g.NavId;
    if(nav.focus_content) {
        // Focus the first setting on this page as it is submitted below.
        nav.focus_content=false;
        ImGui::FocusWindow(ImGui::GetCurrentWindow());
        ImGui::NavInitWindow(ImGui::GetCurrentWindow(),true);
        nav.show_cursor=true;
    }
    nav.was_content=ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x*.62f);
}
inline bool Page(int current,const char* name) {
    return current>=0&&current<int(std::size(Pages))&&std::string(name)==Pages[current].id;
}
inline void End(const char* status,int width,int height) {
    ImGui::PopItemWidth();ImGui::EndChild();ImGui::EndChild();
    const auto l=Measure(width,height);
    ImGui::SetCursorPos({l.rail+l.gap*2,float(height)-l.footer-l.gap});
    ImGui::BeginChild("Context help",{float(width)-l.rail-l.gap*3,l.footer},ImGuiChildFlags_None);
    ImGui::SetWindowFontScale(.72f);
    ImGui::PushStyleColor(ImGuiCol_Text,Muted);
    ImGui::TextWrapped("%s",*status?status:"Hover a setting for details. Controller: D-pad to move, A to select, ZL / ZR to switch pages, B or Start + Select to close.");
    ImGui::PopStyleColor();ImGui::EndChild();
}
} // namespace MM3D::MenuStyle
