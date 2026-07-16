cmake_minimum_required(VERSION 3.22)

project(fl_humbucker VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE if not provided via -DJUCE_PATH=...
if(NOT DEFINED JUCE_PATH)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
else()
    add_subdirectory(${JUCE_PATH} JUCE EXCLUDE_FROM_ALL)
endif()

juce_add_plugin(fl_humbucker
    COMPANY_NAME "Humbucker"
    PLUGIN_MANUFACTURER_CODE Hmbk
    PLUGIN_CODE Hmbx
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "FL Humbucker"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx
)

target_sources(fl_humbucker
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/UI/PerformanceTimeline.cpp
        Source/Analysis/PitchDetector.cpp
        Source/Analysis/OnsetDetector.cpp
        Source/Analysis/PerformanceCapture.cpp
        Source/Analysis/SampleSlot.cpp
        Source/Analysis/PatternRenderer.cpp
        Source/Analysis/PerformanceSampler.cpp
)

target_compile_definitions(fl_humbucker
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(fl_humbucker
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_include_directories(fl_humbucker
    PRIVATE
        Source
)
