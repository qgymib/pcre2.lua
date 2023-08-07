# lua_pcre2
A Lua wrapper for pcre2.

Support Lua version 5.1 / 5.2 / 5.3 / 5.4.

## Integration

### Manually

Copy `lua_pcre2.h` and `lua_pcre2.c` to your build tree, and you are done.

### CMake

Add following code to your CMakeLists.txt:

```cmake
add_subdirectory(lua_pcre2)
target_link_libraries(${PROJECT_NAME} PRIVATE lua_pcre2)
```

## Usage

### Lua API

#### compile()

```lua
code = lpcre2.compile(pattern[, OPTIONS])
```

Compile a regular expression pattern.

The second parameter is optional, which is Bit-OR of following flags:
+ lpcre2.`LPCRE2_ALLOW_EMPTY_CLASS`: Allow empty classes
+ lpcre2.`LPCRE2_DOTALL`: `.` matches anything including NL
+ lpcre2.`LPCRE2_EXTENDED`: Ignore white space and # comments
+ lpcre2.`LPCRE2_MULTILINE`: `^` and `$` match newlines within data.

#### match()

```lua
matchdata = code:match(subject[, OFFSET[, OPTIONS]])
```

Matches a compiled regular expression against a given subject. A matchdata object is returned if match found, or nil if not found.

#### all_groups()

```lua
table = matchdata:all_groups(subject)
```

Return a list of all captured groups. `[0]` is the whole group, `[1]` is group 1, `[2]` is group 2, and so on.

#### group()

```lua
string = matchdata:group(subject, index)
```

Return the content of captured group.

### group_count()

```lua
integer = matchdata:group_count()
```

Get the number of captured groups.

#### group_offset()

```lua
beg,len = matchdata:group_offset(index)
```

Get the offset and length of captured group.

#### substitute()

```lua
string = code:substitute(subject, replacement[, OPTIONS])
```

Matches a compiled regular expression against a given subject, makes a copy of the subject, substituting a replacement string for what was matched.

The third parameter is optional, which is Bit-OR of following flags:
+ lpcre2.`LPCRE2_SUBSTITUTE_GLOBAL`: Replace all occurrences in the subject.
+ lpcre2.`LPCRE2_SUBSTITUTE_EXTENDED`: Do extended replacement processing.
+ lpcre2.`LPCRE2_SUBSTITUTE_UNSET_EMPTY`: Simple unset insert = empty string
+ lpcre2.`LPCRE2_SUBSTITUTE_UNKNOWN_UNSET`: Treat unknown group as unset.
+ lpcre2.`LPCRE2_SUBSTITUTE_REPLACEMENT_ONLY`: Return only replacement string(s).


### C API

Checkout documents in header.

## Trouble shooting

### Chould NOT find Lua

It means cmake cannot find Lua on your system. If you are using a custom build Lua, please set cmake variable `LUA_LIBRARIES` and `LUA_INCLUDE_DIR` manually.

eg.

```
cmake -DLUA_LIBRARIES=/path/to/lua/lib -DLUA_INCLUDE_DIR=/path/to/lua/include ..
```

### Chould not find a package configuration file provided by "PCRE2"

It means cmake cannot find pcre2 on your system.

+ If you are using a custom build pcre2, please set cmake variable `PCRE2_ROOT` / `PCRE2_USE_STATIC_LIBS` manually.

eg.

```
cmake -DPCRE2_ROOT=/path/to/pcre2 -DPCRE2_USE_STATIC_LIBS=ON ..
```

+ If you are using system installed pcre2, it is likely the packager does not offer `pcre2-config.cmake`.
