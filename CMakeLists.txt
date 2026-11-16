cmake_minimum_required(VERSION 3.22)

project(Otodecks VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Locate JUCE -------------------------------------------------------------
# Preferred: a checkout at libs/JUCE (submodule) or a sibling ../JUCE folder.
# Override either by configuring with -DJUCE_PATH=/path/to/JUCE.
# If nothing is found, JUCE is fetched from GitHub at configure time.
if(NOT DEFINED JUCE_PATH)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
        set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE/CMakeLists.txt")
        set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
    endif()
endif()

if(JUCE_PATH)
    message(STATUS "Using JUCE from ${JUCE_PATH}")
    add_subdirectory("${JUCE_PATH}" JUCE)
else()
    message(STATUS "No local JUCE found - fetching JUCE 8.0.4 from GitHub")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# --- Application target ------------------------------------------------------
juce_add_gui_app(Otodecks
    PRODUCT_NAME "Otodecks"
    COMPANY_NAME "Balasubramanian_Manish"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Otodecks records from the microphone when you press RECORD MIC.")

juce_generate_juce_header(Otodecks)

target_sources(Otodecks
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/DeckGUI.cpp
        Source/DJAudioPlayer.cpp
        Source/WaveformDisplay.cpp
        Source/Track.cpp
        Source/PersistenceManager.cpp
        Source/MusicLibrary.cpp
        Source/PlaylistComponent.cpp
        Source/HotCueComponent.cpp
        Source/AudioRecorder.cpp
        Source/SamplePlayer.cpp
        Source/LoopSampler.cpp
        Source/BPMAnalyser.cpp)

target_compile_definitions(Otodecks
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Otodecks,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Otodecks,JUCE_VERSION>")

target_link_libraries(Otodecks
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
