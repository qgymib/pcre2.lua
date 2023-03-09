cmake_minimum_required(VERSION 3.12)
project(lpcre2)

###############################################################################
# Functions
###############################################################################

# Enable all warning for target
function(setup_target_wall name)
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${name} PRIVATE /W4 /WX)
    else ()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    endif ()
endfunction()

###############################################################################
# Setup executable
###############################################################################

add_library(${PROJECT_NAME}
    "src/lua_pcre2.c")

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

setup_target_wall(${PROJECT_NAME})

###############################################################################
# Dependency
###############################################################################

# Lua
find_package(Lua REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})

# PCRE2
find_package(PCRE2 CONFIG COMPONENTS 8BIT)
if (PCRE2_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE PCRE2::8BIT)
else ()
    find_package(PkgConfig)
    pkg_search_module(PCRE2DEP REQUIRED libpcre2-8)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${PCRE2DEP_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${PCRE2DEP_INCLUDE_DIRS})
endif ()

###############################################################################
# Test
###############################################################################
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(third_party/cutest)
    add_subdirectory(test)
endif()
