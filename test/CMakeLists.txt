
add_executable(lpcre2_test
    "case/compile.c"
    "case/luaopen.c"
    "case/substitute.c"
    "test.c")

target_include_directories(lpcre2_test
    PRIVATE
        ${LUA_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lpcre2_test
    PRIVATE
        lpcre2
        cutest
        ${LUA_LIBRARIES})

setup_target_wall(lpcre2_test)

add_test(NAME lpcre2_test COMMAND $<TARGET_FILE:lpcre2_test>)
