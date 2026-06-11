cmake_minimum_required(VERSION 3.22)

project(AcidLab303 VERSION 0.5.0 LANGUAGES C CXX)

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.13
    GIT_SHALLOW TRUE
)

if(EXISTS "${CMAKE_BINARY_DIR}/_deps/juce-src/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_JUCE "${CMAKE_BINARY_DIR}/_deps/juce-src")
endif()
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AcidLab303
    COMPANY_NAME "Zosia Audio"
    BUNDLE_ID "com.zosiaaudio.acidlab303"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE ZsAu
    PLUGIN_CODE Al33
    FORMATS VST3 Standalone
    PRODUCT_NAME "AcidLab303")

target_sources(AcidLab303
    PRIVATE
        src/dsp/AcidEngine.cpp
        src/model/MidiPatternExporter.cpp
        src/model/PatternBank.cpp
        src/plugin/PluginProcessor.cpp
        src/ui/AcidLookAndFeel.cpp
        src/ui/EnergyStrip.cpp
        src/ui/MidiDragButton.cpp
        src/ui/PluginEditor.cpp
        src/ui/StepGrid.cpp)

target_compile_features(AcidLab303 PRIVATE cxx_std_17)

target_compile_definitions(AcidLab303
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AcidLab303
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

enable_testing()

add_executable(AcidLab303Tests
    tests/DspSmokeTest.cpp
    src/dsp/AcidEngine.cpp
    src/model/MidiPatternExporter.cpp
    src/model/PatternBank.cpp)

target_compile_features(AcidLab303Tests PRIVATE cxx_std_17)
target_link_libraries(AcidLab303Tests
    PRIVATE
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

add_test(NAME AcidLab303Smoke COMMAND AcidLab303Tests)
