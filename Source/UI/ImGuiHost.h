#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <functional>

namespace jv1010 {

class ImGuiHost : public juce::Component,
                  public juce::OpenGLRenderer {
public:
    ImGuiHost();
    ~ImGuiHost() override;

    void setRenderCallback(std::function<void()> cb) { onRender_ = std::move(cb); }

    void paint(juce::Graphics&) override {}
    void resized() override;

    // Mouse & Keyboard forwarding to ImGui
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

    // OpenGLRenderer
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

private:
    static void applyJV1010ImGuiTheme();

    juce::OpenGLContext openGLContext_;
    std::function<void()> onRender_;
    double lastFrameTimeSec_{ 0.0 };
    bool contextReady_{ false };
    bool leftButtonDown_{ false };
    bool rightButtonDown_{ false };
    bool middleButtonDown_{ false };
};

} // namespace jv1010
