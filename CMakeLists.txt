cmake_minimum_required(VERSION 3.22)

project(JV1010EditorPro VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add JUCE and Dear ImGui (git submodule)
add_subdirectory(third_party/JUCE)

add_library(imgui_lib STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
)

# Common Core, Model, Database & Librarian sources
set(JV1010_CORE_SOURCES
    Source/Core/SysExProtocol.h
    Source/Core/SysExProtocol.cpp
    Source/Core/MidiManager.h
    Source/Core/MidiManager.cpp
    Source/Core/ExpansionDetector.h
    Source/Core/ExpansionDetector.cpp
    Source/Model/JV1010Types.h
    Source/Model/ParameterMaps.h
    Source/Model/JV1010State.h
    Source/Model/JV1010State.cpp
    Source/Model/EfxAlgorithms.h
    Source/Model/EfxAlgorithms.cpp
    Source/Database/FactoryPresetsDB.h
    Source/Database/FactoryPresetsDB.cpp
    Source/Database/WaveformsDB.h
    Source/Database/WaveformsDB.cpp
    Source/Database/SrJv80CatalogDB.h
    Source/Database/SrJv80CatalogDB.cpp
    Source/Librarian/SyxMidFileIO.h
    Source/Librarian/SyxMidFileIO.cpp
    Source/Librarian/PatchMorphRandom.h
    Source/Librarian/PatchMorphRandom.cpp
)

set(JV1010_IMGUI_UI_SOURCES
    Source/UI/ImGuiHost.h
    Source/UI/ImGuiHost.cpp
    Source/UI/ImGuiWidgets.h
    Source/UI/ImGuiWidgets.cpp
    Source/UI/Header/JV1010Silkscreen.h
    Source/UI/Header/JV1010Silkscreen.cpp
    Source/UI/Header/TopControlBar.h
    Source/UI/Header/TopControlBar.cpp
    Source/UI/Browser/SetLibrarianView.h
    Source/UI/Browser/SetLibrarianView.cpp
    Source/UI/Editors/PerformanceEditor.h
    Source/UI/Editors/PerformanceEditor.cpp
    Source/UI/Editors/PatchEditor.h
    Source/UI/Editors/PatchEditor.cpp
    Source/UI/Editors/PatchEffectsEditor.h
    Source/UI/Editors/PatchEffectsEditor.cpp
    Source/UI/Editors/DrumsEditor.h
    Source/UI/Editors/DrumsEditor.cpp
    Source/UI/Editors/SystemEditor.h
    Source/UI/Editors/SystemEditor.cpp
    Source/UI/Footer/VirtualKeyboardDock.h
    Source/UI/Footer/VirtualKeyboardDock.cpp
    Source/UI/ImGuiAppView.h
    Source/UI/ImGuiAppView.cpp
)

# Main JUCE Plugin + Standalone target
juce_add_plugin(JV1010EditorPro
    COMPANY_NAME "JV1010Tools"
    BUNDLE_ID "com.jv1010tools.jv1010editorpro"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE JvEd
    PLUGIN_CODE Jv10
    FORMATS VST3 Standalone
    PRODUCT_NAME "JV-1010 Editor Pro"
)

target_sources(JV1010EditorPro
    PRIVATE
        ${JV1010_CORE_SOURCES}
        ${JV1010_IMGUI_UI_SOURCES}
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp
)

target_include_directories(JV1010EditorPro
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(JV1010EditorPro
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(JV1010EditorPro
    PRIVATE
        imgui_lib
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Unit Tests executable
add_executable(JV1010Tests
    Tests/SysExProtocolTests.cpp
    Source/Core/SysExProtocol.cpp
    Source/Model/JV1010State.cpp
    Source/Model/EfxAlgorithms.cpp
    Source/Database/FactoryPresetsDB.cpp
    Source/Database/SrJv80CatalogDB.cpp
    Source/Librarian/SyxMidFileIO.cpp
    Source/Librarian/PatchMorphRandom.cpp
)

target_include_directories(JV1010Tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(JV1010Tests
    PRIVATE
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
)

enable_testing()
add_test(NAME SysExProtocolTests COMMAND JV1010Tests)
