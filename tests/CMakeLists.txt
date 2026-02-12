cmake_minimum_required(VERSION 4.1)

project(amulet_faulthandler_tests LANGUAGES CXX)

# Set C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set platform variables
if (WIN32)
	# set windows 7 as the minimum version
	add_definitions(-D_WIN32_WINNT=0x0601)
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
elseif(UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
    message( FATAL_ERROR "Unsupported platform. Please submit a pull request to support this platform." )
endif()

# Find dependencies
if (NOT TARGET pybind11::module)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# Find sources
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false "${CMAKE_CURRENT_LIST_DIR}/*.py.cpp")

pybind11_add_module(_test_amulet_faulthandler)
set_target_properties(_test_amulet_faulthandler PROPERTIES FOLDER "Tests")
target_compile_definitions(_test_amulet_faulthandler PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
target_sources(_test_amulet_faulthandler PRIVATE ${SOURCES})
foreach(FILE ${SOURCES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_LIST_DIR} ${FILE})
    get_filename_component(GROUP ${REL_PATH} DIRECTORY)
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    source_group(${GROUP} FILES ${FILE})
endforeach()

# Install
install(TARGETS _test_amulet_faulthandler DESTINATION "${CMAKE_CURRENT_LIST_DIR}")
