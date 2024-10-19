cmake_minimum_required(VERSION 3.30)
set(CMAKE_C_STANDARD 11)
string(FIND "${CMAKE_GENERATOR}" "Xcode" is_xcode)
string(FIND "${CMAKE_GENERATOR}" "Makefile" is_makefile)
if(is_xcode GREATER_EQUAL 0)
    set(is_xcode TRUE)
else()
    set(is_xcode FALSE)
endif()
if(is_makefile GREATER_EQUAL 0)
    set(is_makefile TRUE)
else()
    set(is_makefile FALSE)
endif()

message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

set(ENVSDK $ENV{PLAYDATE_SDK_PATH})

if (NOT ${ENVSDK} STREQUAL "")
	# Convert path from Windows
	file(TO_CMAKE_PATH ${ENVSDK} SDK)
else()
	execute_process(
			COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
			COMMAND head -n 1
			COMMAND cut -c9-
			OUTPUT_VARIABLE SDK
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if (NOT EXISTS ${SDK})
	message(FATAL_ERROR "SDK Path not found; set ENV value PLAYDATE_SDK_PATH")
	return()
endif()

function(generate_compile_flags TARGET_NAME DST_DIR)
    message(STATUS "Generate compile_flags.txt: ${DST_DIR}")
    set(COMPILE_FLAGS "-I${SDK}/C_API\n")
    get_target_property(include_dirs ${TARGET_NAME} INCLUDE_DIRECTORIES)
    foreach(dir IN LISTS include_dirs)
        string(APPEND COMPILE_FLAGS "-I${dir}\n")
    endforeach()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../compile_flags.txt.in" "${DST_DIR}/compile_flags.txt" @ONLY)
endfunction()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

file(GLOB_RECURSE SOURCES *.c)
list(FILTER SOURCES EXCLUDE REGEX "_build")

message("Sources: ${SOURCES}")

cmake_path(GET CMAKE_SOURCE_DIR PARENT_PATH PARENT_DIR)
cmake_path(GET PARENT_DIR PARENT_PATH PARENT_DIR)
get_filename_component(LIB_NAME ${PARENT_DIR} NAME)

# Game Name Customization
get_filename_component(PROJECT_NAME ${CMAKE_SOURCE_DIR} NAME)
message("Project Name: ${PROJECT_NAME}")
message("Binary Dir: ${CMAKE_BINARY_DIR}")

project(${PROJECT_NAME} C ASM)

if(TOOLCHAIN STREQUAL "armgcc")
    set(PLAYDATE_GAME_DEVICE ${PROJECT_NAME}_DEVICE)
    add_executable(${PLAYDATE_GAME_DEVICE} ${SOURCES})
    set(TARGET_NAME ${PLAYDATE_GAME_DEVICE})
else()
    set(PLAYDATE_GAME_NAME ${PROJECT_NAME})
    add_library(${PLAYDATE_GAME_NAME} SHARED ${SOURCES})
    set(TARGET_NAME ${PLAYDATE_GAME_NAME})
endif()

target_include_directories(${TARGET_NAME} PUBLIC ../../src ../../include)
set(LIBRARY_PATH "")
if(is_xcode)
    target_link_libraries(${TARGET_NAME}
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../../_output/lib${LIB_NAME}.simulator.debug.a>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/../../_output/lib${LIB_NAME}.simulator.release.a>
    )
else()
    get_filename_component(LIBRARY_TYPE ${CMAKE_BINARY_DIR} NAME)
    set(LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../_output/lib${LIB_NAME}.${LIBRARY_TYPE}.a)
    if(TOOLCHAIN STREQUAL "armgcc")
        target_link_libraries(${TARGET_NAME} -Wl,--start-group "${LIBRARY_PATH}" -Wl,--end-group)
    else()
        target_link_libraries(${TARGET_NAME} "${LIBRARY_PATH}")
    endif()
    message("Library Path: ${LIBRARY_PATH}")
endif()


generate_compile_flags(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

include(${SDK}/C_API/buildsupport/playdate_game.cmake)

