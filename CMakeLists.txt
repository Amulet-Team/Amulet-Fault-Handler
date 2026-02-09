cmake_minimum_required(VERSION 3.13)

project(amulet_faulthandler LANGUAGES CXX)

set(amulet_faulthandler_DIR ${CMAKE_CURRENT_LIST_DIR}/src/amulet_faulthandler CACHE PATH "")

# Set C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set platform variables
if (WIN32)
	# set windows 7 as the minimum version
	add_definitions(-D_WIN32_WINNT=0x0602)
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
elseif(UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
    message( FATAL_ERROR "Unsupported platform. Please submit a pull request to support this platform." )
endif()

if (MSVC)
    add_definitions("/MP")
endif()

# Find libraries
if (NOT TARGET pybind11::module)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# Find C++ files
file(REAL_PATH src SOURCE_PATH)
file(GLOB_RECURSE EXTENSION_SOURCES LIST_DIRECTORIES false ${SOURCE_PATH}/amulet/*.py.cpp)

# Add python extension
pybind11_add_module(_faulthandler)
set_target_properties(_faulthandler PROPERTIES FOLDER "Python")
target_compile_definitions(_faulthandler PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
target_sources(_faulthandler PRIVATE ${EXTENSION_SOURCES})

if(NOT DEFINED AMULET_FAULTHANDLER_EXT_DIR)
    set(AMULET_FAULTHANDLER_EXT_DIR ${amulet_faulthandler_DIR})
endif()

# Install
install(TARGETS _faulthandler DESTINATION ${AMULET_FAULTHANDLER_EXT_DIR})
