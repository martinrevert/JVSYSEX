#include "ImGuiHost.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <mutex>

namespace jv1010 {

namespace {
int mapJuceKeyToImGuiKey(int keyCode) {
    if (keyCode == juce::KeyPress::backspaceKey) return ImGuiKey_Backspace;
    if (keyCode == juce::KeyPress::deleteKey)    return ImGuiKey_Delete;
    if (keyCode == juce::KeyPress::returnKey)    return ImGuiKey_Enter;
    if (keyCode == juce::KeyPress::escapeKey)    return ImGuiKey_Escape;
    if (keyCode == juce::KeyPress::tabKey)       return ImGuiKey_Tab;
    if (keyCode == juce::KeyPress::leftKey)      return ImGuiKey_LeftArrow;
    if (keyCode == juce::KeyPress::rightKey)     return ImGuiKey_RightArrow;
    if (keyCode == juce::KeyPress::upKey)        return ImGuiKey_UpArrow;
    if (keyCode == juce::KeyPress::downKey)      return ImGuiKey_DownArrow;
    if (keyCode == juce::KeyPress::homeKey)      return ImGuiKey_Home;
    if (keyCode == juce::KeyPress::endKey)       return ImGuiKey_End;
    if (keyCode == juce::KeyPress::spaceKey)     return ImGuiKey_Space;
    return ImGuiKey_None;
}
} // namespace

ImGuiHost::ImGuiHost() {
    setWantsKeyboardFocus(true);
    setOpaque(true);
    openGLContext_.setRenderer(this);
    openGLContext_.setContinuousRepainting(true);
    openGLContext_.attachTo(*this);
}

ImGuiHost::~ImGuiHost() {
    openGLContext_.detach();
}

void ImGuiHost::resized() {
}

void ImGuiHost::applyJV1010ImGuiTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 5.0f);

    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg]         = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
    c[ImGuiCol_ChildBg]          = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    c[ImGuiCol_PopupBg]          = ImVec4(0.11f, 0.12f, 0.14f, 0.98f);
    c[ImGuiCol_Border]           = ImVec4(0.24f, 0.25f, 0.28f, 1.00f);
    c[ImGuiCol_FrameBg]          = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    c[ImGuiCol_FrameBgHovered]   = ImVec4(0.20f, 0.21f, 0.25f, 1.00f);
    c[ImGuiCol_FrameBgActive]    = ImVec4(0.85f, 0.45f, 0.08f, 0.65f);
    c[ImGuiCol_TitleBg]          = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    c[ImGuiCol_TitleBgActive]    = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    c[ImGuiCol_Button]           = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    c[ImGuiCol_ButtonHovered]    = ImVec4(0.85f, 0.45f, 0.08f, 0.85f);
    c[ImGuiCol_ButtonActive]     = ImVec4(0.92f, 0.52f, 0.10f, 1.00f);
    c[ImGuiCol_Tab]              = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
    c[ImGuiCol_TabHovered]       = ImVec4(0.80f, 0.25f, 0.15f, 0.85f);
    c[ImGuiCol_TabSelected]      = ImVec4(0.75f, 0.22f, 0.14f, 1.00f);
    c[ImGuiCol_Header]           = ImVec4(0.20f, 0.44f, 0.80f, 0.45f);
    c[ImGuiCol_HeaderHovered]    = ImVec4(0.20f, 0.44f, 0.80f, 0.70f);
    c[ImGuiCol_HeaderActive]     = ImVec4(0.85f, 0.45f, 0.08f, 0.90f);
    c[ImGuiCol_CheckMark]        = ImVec4(0.96f, 0.62f, 0.04f, 1.00f);
    c[ImGuiCol_SliderGrab]       = ImVec4(0.75f, 0.77f, 0.80f, 1.00f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.96f, 0.62f, 0.04f, 1.00f);
}

void ImGuiHost::newOpenGLContextCreated() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;

    applyJV1010ImGuiTheme();
    ImGui_ImplOpenGL3_Init("#version 130");
    lastFrameTimeSec_ = juce::Time::getMillisecondCounterHiRes() * 0.001;
    contextReady_ = true;
}

void ImGuiHost::renderOpenGL() {
    if (!contextReady_) return;

    const float scale = static_cast<float>(openGLContext_.getRenderingScale());
    const int w = getWidth();
    const int h = getHeight();
    if (w <= 0 || h <= 0) return;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
    io.DisplayFramebufferScale = ImVec2(scale, scale);

    double nowSec = juce::Time::getMillisecondCounterHiRes() * 0.001;
    io.DeltaTime = static_cast<float>(juce::jmax(0.001, nowSec - lastFrameTimeSec_));
    lastFrameTimeSec_ = nowSec;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (onRender_) {
        onRender_();
    }

    ImGui::Render();

    juce::gl::glViewport(0, 0, static_cast<int>(w * scale), static_cast<int>(h * scale));
    juce::gl::glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
    juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiHost::openGLContextClosing() {
    if (!contextReady_) return;
    contextReady_ = false;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiHost::mouseMove(const juce::MouseEvent& e) {
    if (!contextReady_) return;
    ImGui::GetIO().AddMousePosEvent(e.position.x, e.position.y);
}

void ImGuiHost::mouseEnter(const juce::MouseEvent& e) {
    if (!contextReady_) return;
    ImGui::GetIO().AddMousePosEvent(e.position.x, e.position.y);
}

void ImGuiHost::mouseExit(const juce::MouseEvent&) {
    if (!contextReady_) return;
    ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
}

void ImGuiHost::mouseDown(const juce::MouseEvent& e) {
    grabKeyboardFocus();
    if (!contextReady_) return;
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(e.position.x, e.position.y);
    if (e.mods.isLeftButtonDown() && !leftButtonDown_) {
        leftButtonDown_ = true;
        io.AddMouseButtonEvent(0, true);
    }
    if (e.mods.isRightButtonDown() && !rightButtonDown_) {
        rightButtonDown_ = true;
        io.AddMouseButtonEvent(1, true);
    }
    if (e.mods.isMiddleButtonDown() && !middleButtonDown_) {
        middleButtonDown_ = true;
        io.AddMouseButtonEvent(2, true);
    }
}

void ImGuiHost::mouseDrag(const juce::MouseEvent& e) {
    if (!contextReady_) return;
    ImGui::GetIO().AddMousePosEvent(e.position.x, e.position.y);
}

void ImGuiHost::mouseUp(const juce::MouseEvent& e) {
    if (!contextReady_) return;
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(e.position.x, e.position.y);
    if (leftButtonDown_) {
        leftButtonDown_ = false;
        io.AddMouseButtonEvent(0, false);
    }
    if (rightButtonDown_) {
        rightButtonDown_ = false;
        io.AddMouseButtonEvent(1, false);
    }
    if (middleButtonDown_) {
        middleButtonDown_ = false;
        io.AddMouseButtonEvent(2, false);
    }
}

void ImGuiHost::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) {
    if (!contextReady_) return;
    ImGui::GetIO().AddMouseWheelEvent(wheel.deltaX * 2.5f, wheel.deltaY * 2.5f);
}

bool ImGuiHost::keyPressed(const juce::KeyPress& key) {
    if (!contextReady_) return false;
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl,  key.getModifiers().isCtrlDown());
    io.AddKeyEvent(ImGuiMod_Shift, key.getModifiers().isShiftDown());
    io.AddKeyEvent(ImGuiMod_Alt,   key.getModifiers().isAltDown());

    int imKey = mapJuceKeyToImGuiKey(key.getKeyCode());
    if (imKey != ImGuiKey_None) {
        io.AddKeyEvent(static_cast<ImGuiKey>(imKey), true);
        io.AddKeyEvent(static_cast<ImGuiKey>(imKey), false);
    }

    auto ch = key.getTextCharacter();
    if (ch >= 32 && ch != 127) {
        io.AddInputCharacter(static_cast<unsigned int>(ch));
    }
    return true;
}

bool ImGuiHost::keyStateChanged(bool) {
    if (!contextReady_) return false;
    auto mods = juce::ModifierKeys::getCurrentModifiers();
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl,  mods.isCtrlDown());
    io.AddKeyEvent(ImGuiMod_Shift, mods.isShiftDown());
    io.AddKeyEvent(ImGuiMod_Alt,   mods.isAltDown());
    return false;
}

} // namespace jv1010
