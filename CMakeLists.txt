cmake_minimum_required(VERSION 3.22)

project(GlorpyBass VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("/home/gfc/JUCE" JUCE)

juce_add_plugin(GlorpyBass
    COMPANY_NAME "OpenCode"
    BUNDLE_ID com.opencode.glorpybass
    PLUGIN_MANUFACTURER_CODE OpCd
    PLUGIN_CODE Glrb
    FORMATS Standalone VST3
    PRODUCT_NAME "Glorpy Bass"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(GlorpyBass)

target_sources(GlorpyBass
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
)

target_compile_definitions(GlorpyBass
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(GlorpyBass
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
