cmake_minimum_required (VERSION 3.20)

project (Melatonin::Inspector VERSION 1.2.0 LANGUAGES CXX
		 DESCRIPTION "JUCE module for inspecting Components"
		 HOMEPAGE_URL "https://github.com/sudara/melatonin_inspector")

if (NOT COMMAND juce_add_module)
    message(FATAL_ERROR "JUCE must be added to your project before melatonin_inspector!")
endif ()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

juce_add_module ("${CMAKE_CURRENT_LIST_DIR}")

add_library(Melatonin::Inspector ALIAS melatonin_inspector)

# Assets are precompiled in the module to make it Projucer friendly
# set MELATONIN_COMPILE_BINARY_DATA to ON to generate the assets again (then copy over from the build folder)
option(MELATONIN_COMPILE_BINARY_DATA "Compile the Binary Data as part of the melatonin_inspector target" OFF)

if (MELATONIN_COMPILE_BINARY_DATA)
	file(GLOB_RECURSE MelatoninInspectorAssetFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*")
	juce_add_binary_data(MelatoninInspectorAssets SOURCES ${MelatoninInspectorAssetFiles} HEADER_NAME InspectorBinaryData.h NAMESPACE InspectorBinaryData)
	set_target_properties(MelatoninInspectorAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
	target_link_libraries(melatonin_inspector INTERFACE MelatoninInspectorAssets)
	target_compile_definitions(melatonin_inspector INTERFACE MELATONIN_USE_COMPILED_BINARY_DATA=1)
else()
	target_include_directories(melatonin_inspector INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/LatestCompiledAssets)
	target_compile_definitions(melatonin_inspector INTERFACE MELATONIN_USE_COMPILED_BINARY_DATA=0)
endif()

