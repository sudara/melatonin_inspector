cmake_minimum_required (VERSION 3.20)

project (Melatonin::Inspector VERSION 1.0.0 LANGUAGES CXX
		 DESCRIPTION "JUCE module for inspecting Components"
		 HOMEPAGE_URL "https://github.com/sudara/melatonin_inspector")

if (NOT COMMAND juce_add_module)
    message(FATAL_ERROR "JUCE must be added to your project before melatonin_inspector!")
endif ()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

file(GLOB_RECURSE MelatoninInspectorAssetFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*")
juce_add_binary_data(MelatoninInspectorAssets SOURCES ${MelatoninInspectorAssetFiles})
set_target_properties(MelatoninInspectorAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

juce_add_module ("${CMAKE_CURRENT_LIST_DIR}")

add_library(Melatonin::Inspector ALIAS melatonin_inspector)
