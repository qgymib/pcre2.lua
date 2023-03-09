# lua_pcre2
A Lua wrapper for pcre2

## Integration

### Manually

Copy `lua_pcre2.h` and `lua_pcre2.c` to your build tree, and you are done.

### CMake

Add following code to your CMakeLists.txt:

```cmake
add_subdirectory(lua_pcre2)
target_link_libraries(${PROJECT_NAME} PRIVATE lua_pcre2)
```

## Trouble shooting

### Chould NOT find Lua

It means cmake cannot find Lua on your system. If you are using a custom build Lua, please set cmake variable `$LUA_LIBRARIES` and `$LUA_INCLUDE_DIR` manually.

### Chould not find a package configuration file provided by "PCRE2"

It means cmake cannot find pcre2 on your system. If you are using a custom build pcre2, please set cmake variable `$PCRE2_DIR` / `$PCRE2_INCLUDE_DIR` / `$PCRE2_POSIX_LIBRARY` / `$PCRE2_8BIT_LIBRARY` manually.
