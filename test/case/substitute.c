#include "test.h"

typedef struct test_substitute
{
	lua_State* L;
} test_substitute_t;

static test_substitute_t g_test_substitute;

TEST_FIXTURE_SETUP(code)
{
	memset(&g_test_substitute, 0, sizeof(g_test_substitute));

	g_test_substitute.L = luaL_newstate();
	ASSERT_NE_PTR(g_test_substitute.L, NULL);

	ASSERT_EQ_INT(luaopen_lpcre2(g_test_substitute.L), 1);
}

TEST_FIXTURE_TEAREDOWN(code)
{
	lua_close(g_test_substitute.L);
	g_test_substitute.L = NULL;
}

TEST_F(code, substitute_c)
{
	int sp = lua_gettop(g_test_substitute.L);

	/* pcre2.compile(pattern) */
	{
#if LUA_VERSION_NUM >= 503
		ASSERT_EQ_INT(lua_getfield(g_test_substitute.L, -1, "compile"), LUA_TFUNCTION);
#else
		lua_getfield(g_test_substitute.L, -1, "compile");
		ASSERT_EQ_INT(lua_type(g_test_substitute.L, -1), LUA_TFUNCTION);
#endif

		const char* pattern = "^(\\s*#\\s*include\\s+\"[-.\\w/]+\")";
		lua_pushstring(g_test_substitute.L, pattern);

		ASSERT_EQ_INT(lua_pcall(g_test_substitute.L, 1, 1, 0), LUA_OK);
	}

	/* code::substitute(content, replacement) */
	{
#if LUA_VERSION_NUM >= 503
		ASSERT_EQ_INT(lua_getfield(g_test_substitute.L, -1, "substitute"), LUA_TFUNCTION);
#else
		lua_getfield(g_test_substitute.L, -1, "substitute");
		ASSERT_EQ_INT(lua_type(g_test_substitute.L, -1), LUA_TFUNCTION);
#endif

		lua_pushvalue(g_test_substitute.L, sp + 1);

		const char* content =
			"#include \"test.h\"";
		lua_pushstring(g_test_substitute.L, content);

		const char* replacement =
			"/* AMALGAMATE: $1 */";
		lua_pushstring(g_test_substitute.L, replacement);

		ASSERT_EQ_INT(lua_pcall(g_test_substitute.L, 3, 1, 0), LUA_OK);
		ASSERT_EQ_INT(lua_type(g_test_substitute.L, -1), LUA_TSTRING);
	}

	ASSERT_EQ_STR(lua_tostring(g_test_substitute.L, -1), "/* AMALGAMATE: #include \"test.h\" */");
}

TEST_F(code, substitute_lua)
{
	const char* lua_code =
"local pattern = \"^(\\\\s*#\\\\s*include\\\\s+\\\"[-.\\\\w/]+\\\")\"\n"
"code = lpcre2.compile(pattern)\n"
"assert(code ~= nil)\n"
"\n"
"local content = \"#include \\\"test.h\\\"\"\n"
"local replace = \"/* AMALGAMATE: $1 */\"\n"
"local ret = code:substitute(content, replace)\n"
"assert(ret == \"/* AMALGAMATE: #include \\\"test.h\\\" */\")\n";

	lua_setglobal(g_test_substitute.L, "lpcre2");
	luaL_openlibs(g_test_substitute.L);

	ASSERT_EQ_INT(luaL_dostring(g_test_substitute.L, lua_code), LUA_OK,
		"%s", lua_tostring(g_test_substitute.L, -1));
}
