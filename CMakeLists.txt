cmake_minimum_required(VERSION 3.20)

project(MelatoninInspector VERSION 1.3.0 LANGUAGES CXX
        DESCRIPTION "JUCE module for inspecting Components"
        HOMEPAGE_URL "https://github.com/sudara/melatonin_inspector")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include(FetchContent)
if (MelatoninInspector_IS_TOP_LEVEL)
    message(STATUS "Cloning JUCE...")

    FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG origin/master
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            FIND_PACKAGE_ARGS 7.0.6)

    FetchContent_MakeAvailable(JUCE)

    juce_add_console_app(dummy VERSION 1.0.0)
    target_sources(dummy PRIVATE tests/dummy.cpp)
endif ()

if (NOT COMMAND juce_add_module)
    message(FATAL_ERROR "JUCE must be added to your project before melatonin_inspector!")
endif ()

# this makes the assumption the current directory is named melatonin_inspector
juce_add_module("${CMAKE_CURRENT_LIST_DIR}")
add_library(Melatonin::Inspector ALIAS melatonin_inspector)

if (MelatoninInspector_IS_TOP_LEVEL)
    target_link_libraries(dummy PRIVATE melatonin_inspector
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
endif ()

# Assets are precompiled in the module to make it Projucer friendly
#
# TODO: We need to automate this.
#
# To work with CMake Assets:
#  * Uncomment the following lines to generate the assets again
#  * Delete everything in "LatestCompliedAssets" folder so there's no conflict
#  * Comment out the manual includes in melatonin_inspector.cpp

# When ready to commit:
#  * Comment out these lines again
#  * Move the new binary .cpp from your build folder to LatestCompiledAssets
#  * Move the latest InspectorBinaryData.h to the main module folder
#    (for me the files are in cmake-build-debug/modules/melatonin_inspector/juce_binarydata_MelatoninInspector
#  * Add the .cpp includes manually to melatonin_inspector.cpp

#file(GLOB_RECURSE MelatoninInspectorAssetFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*")
#juce_add_binary_data(MelatoninInspectorAssets SOURCES ${MelatoninInspectorAssetFiles} HEADER_NAME InspectorBinaryData.h NAMESPACE InspectorBinaryData)
#set_target_properties(MelatoninInspectorAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
#target_link_libraries(melatonin_inspector INTERFACE MelatoninInspectorAssets)
