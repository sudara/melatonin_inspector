cmake_minimum_required(VERSION 3.20)

project(MelatoninInspector VERSION 1.3.0 LANGUAGES C CXX
    DESCRIPTION "JUCE module for inspecting Components"
    HOMEPAGE_URL "https://github.com/sudara/melatonin_inspector")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include(FetchContent)
if (MelatoninInspector_IS_TOP_LEVEL)
    option(JUCE7 "Run tests on JUCE 7" OFF)

    message(STATUS "Cloning JUCE...")
    if (JUCE7)
        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 7.0.12
            GIT_PROGRESS TRUE
        )
    else ()

        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG develop
            GIT_PROGRESS TRUE
        )
    endif ()

    FetchContent_MakeAvailable(JUCE)

    SET(TARGET_NAME "unique_ptr_disabled" CACHE STRING "Name of cpp and target to compile")

    juce_add_gui_app("${TARGET_NAME}" VERSION 1.0.0)
    target_sources("${TARGET_NAME}" PRIVATE "tests/${TARGET_NAME}.cpp")

    target_compile_definitions("${TARGET_NAME}" PUBLIC
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
    )

    # Windows errors due to sccache issues
    # see https://github.com/mozilla/sccache/pull/963/files
    if (MSVC)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
            string(REPLACE "/MP" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
            string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
            string(REPLACE "/MP" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        endif ()
    endif ()

    set_target_properties("${TARGET_NAME}" PROPERTIES COMPILE_WARNING_AS_ERROR ON)

endif ()

if (NOT COMMAND juce_add_module)
    message(FATAL_ERROR "JUCE must be added to your project before melatonin_inspector!")
endif ()

# this makes the assumption the current directory is named melatonin_inspector
juce_add_module("${CMAKE_CURRENT_LIST_DIR}")
add_library(Melatonin::Inspector ALIAS melatonin_inspector)

if (MelatoninInspector_IS_TOP_LEVEL)
    target_link_libraries("${TARGET_NAME}" PRIVATE melatonin_inspector
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
endif ()

# Assets are precompiled in the module to make it Projucer friendly
#
# To add or modify images:
#  * Uncomment the following CMake lines to generate the MelatoninInspectorAssets binary
#  * uncomment #include "InspectorBinaryData.h in misc.h and comment out the LatestCompiledAssets include
#  * build as you would normally via CMake
#  * run ./modules/melatonin_inspector/copy_cmake_assets.rb from your root PROJECT folder (not the melatonin_inspector folder)
#  * comment these lines back out and swap the misc.h include again

#file(GLOB_RECURSE MelatoninInspectorAssetFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*")
#juce_add_binary_data(MelatoninInspectorAssets SOURCES ${MelatoninInspectorAssetFiles} HEADER_NAME InspectorBinaryData.h NAMESPACE InspectorBinaryData)
#set_target_properties(MelatoninInspectorAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
#target_link_libraries(melatonin_inspector INTERFACE MelatoninInspectorAssets)
