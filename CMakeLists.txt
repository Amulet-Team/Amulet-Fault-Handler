cmake_minimum_required(VERSION 4.1)

project(amulet_faulthandler LANGUAGES CXX)

set(amulet_faulthandler_DIR ${CMAKE_CURRENT_LIST_DIR}/src/amulet_faulthandler CACHE PATH "")

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

if (MSVC)
    add_definitions("/MP")
endif()

find_package(Python3 COMPONENTS Interpreter Development)

# Find libraries
if (NOT TARGET pybind11::module)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# Find C++ files
file(REAL_PATH src SOURCE_PATH)
file(GLOB_RECURSE EXTENSION_SOURCES LIST_DIRECTORIES false ${SOURCE_PATH}/amulet_faulthandler/*.py.cpp)
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false ${SOURCE_PATH}/amulet_faulthandler/*.cpp)
file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false ${SOURCE_PATH}/amulet_faulthandler/*.hpp)
list(REMOVE_ITEM SOURCES ${EXTENSION_SOURCES})


# Add implementation
add_library(faulthandler STATIC)
set_target_properties(faulthandler PROPERTIES FOLDER "CPP")
if (WIN32)
    target_link_libraries(faulthandler PRIVATE dbghelp)
endif()
target_include_directories(faulthandler PUBLIC ${SOURCE_PATH})
target_sources(faulthandler PRIVATE ${SOURCES} ${HEADERS})
foreach(FILE ${SOURCES} ${HEADERS})
    file(RELATIVE_PATH REL_PATH ${SOURCE_PATH} ${FILE})
    get_filename_component(GROUP ${REL_PATH} DIRECTORY)
    string(REPLACE "/" "\\" GROUP ${GROUP})
    source_group(${GROUP} FILES ${FILE})
endforeach()


# Add python extension
pybind11_add_module(_faulthandler)
set_target_properties(_faulthandler PROPERTIES FOLDER "Python")
target_link_libraries(_faulthandler PRIVATE faulthandler)
target_compile_definitions(_faulthandler PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
target_sources(_faulthandler PRIVATE ${EXTENSION_SOURCES})
if (WIN32)
    string(REPLACE ".pyd" "" PY_EXT_NO_PYD "${PYTHON_MODULE_EXTENSION}")
    set_target_properties(_faulthandler PROPERTIES
        PDB_NAME "_faulthandler${PY_EXT_NO_PYD}"
    )
endif()

if(NOT DEFINED AMULET_FAULTHANDLER_EXT_DIR)
    set(AMULET_FAULTHANDLER_EXT_DIR ${amulet_faulthandler_DIR})
endif()

# Install
install(TARGETS _faulthandler DESTINATION ${AMULET_FAULTHANDLER_EXT_DIR})
if (WIN32)
    install(
        FILES $<TARGET_PDB_FILE:_faulthandler>
        DESTINATION ${AMULET_FAULTHANDLER_EXT_DIR}
        OPTIONAL
    )
endif()
