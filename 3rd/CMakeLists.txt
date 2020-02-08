get_directory_property(m_hasParent PARENT_DIRECTORY)
if(NOT m_hasParent)
    message(FATAL_ERROR "Use the top-level CMake script!")
endif()
unset(m_hasParent)

# disable all warnings while compile sources in 3rd
if(MSVC)
    string(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
else()
    add_compile_options("-w")
endif()

find_package(RapidJSON 1.1 QUIET CONFIG)
if(NOT RapidJSON_FOUND)
    find_package(RapidJSON 1.1 QUIET)
endif()
if(NOT RapidJSON_FOUND)
    set(RAPIDJSON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include")
endif()
message(STATUS "Found RapidJSON: ${RAPIDJSON_INCLUDE_DIRS}")
add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON SYSTEM INTERFACE
    $<BUILD_INTERFACE:${RAPIDJSON_INCLUDE_DIRS}>)

set_target_properties(RapidJSON PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RAPIDJSON_INCLUDE_DIRS}")

target_include_directories(RapidJSON SYSTEM INTERFACE ${RAPIDJSON_INCLUDE_DIRS})

set(RapidJSON_VERSION ${RapidJSON_VERSION} PARENT_SCOPE)
message(STATUS "--------------------------- 8< --------------------------")
