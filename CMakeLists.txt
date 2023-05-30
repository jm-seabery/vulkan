# Template Application - build using cmake
# MACOS requires to be built with clang++ (/usr/bin/clang++)
# Windows MSVC should use x64 and host=x64 when generating

cmake_minimum_required(VERSION 3.8)
project(vulkan)
message("Template Application build started")

# Add google test suite (should create in the build/_deps folder)
include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_CXX_CLANG_TIDY
# clang-tidy;
# -header-filter=.; # checks header files
# -warnings-as-errors=*; # warnings as errors
# -checks=*;)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT CMAKE_BUILD_TYPE)
	if(NOT MSVC)
		# CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up differentiation between debug and release builds.
		set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
	else()
	endif()
endif()

# library name postfix
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_RELWITHDEBINFO_POSTFIX "-r")
set(CMAKE_RELEASE_POSTFIX "")

# Set the c++17 standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

# Determine 64 bit or 32 bit (only affects Windows builds generally)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BUILD_64BIT TRUE)
else()
	set(BUILD_64BIT FALSE)
endif()

# per platform compile flags and preprocessor
if(UNIX AND NOT APPLE)
	message(${CMAKE_BUILD_TYPE})

	add_compile_options(-Wall
		-fPIC)

	if(CMAKE_BUILD_TYPE MATCHES Debug)
		add_definitions(-DSEABERY_BUILD_DEBUG)
		add_compile_options(-ggdb)
		message("Is DEBUG")
	else()
		add_definitions(-DSEABERY_BUILD_RELEASE)
		add_definitions(-DNDEBUG)
		add_compile_options(-O3
			-fopenmp
			-march=haswell
			-mavx)
		message("Is RELEASE")
	endif()

# profile options
# add_compile_options(-g -pg)
elseif(APPLE)
elseif(WIN32)
	if(MSVC)
	else() # MinGW
	endif()
endif()

# make the bin folder
file(MAKE_DIRECTORY bin)

# Common SDK include directories
include_directories(submods/GSL/include)
include_directories(submods/glm)
include_directories(submods/Vulkan-Hpp)
include_directories(submods/sdlpp)

# app includes
include_directories(src)

set(SOURCES_EXT

	${PROJECT_SOURCE_DIR}/submods/Vulkan-Hpp/samples/utils/utils.cpp
)

set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(submods/SDL)

add_subdirectory(submods/Vulkan-Hpp/RAII_Samples/utils)

# Root source code
add_subdirectory(src)
