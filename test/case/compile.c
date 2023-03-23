#include "test.h"

typedef struct test_compile
{
	lua_State* L;
} test_compile_t;

static test_compile_t g_test_compile;

TEST_FIXTURE_SETUP(lpcre2)
{
	memset(&g_test_compile, 0, sizeof(g_test_compile));

	g_test_compile.L = luaL_newstate();
	ASSERT_NE_PTR(g_test_compile.L, NULL);

	ASSERT_EQ_INT(luaopen_lpcre2(g_test_compile.L), 1);
}

TEST_FIXTURE_TEARDOWN(lpcre2)
{
	lua_close(g_test_compile.L);
	g_test_compile.L = NULL;
}

TEST_F(lpcre2, compile)
{
#if LUA_VERSION_NUM >= 503
	ASSERT_EQ_INT(lua_getfield(g_test_compile.L, -1, "compile"), LUA_TFUNCTION);
#else
	lua_getfield(g_test_compile.L, -1, "compile");
	ASSERT_EQ_INT(lua_type(g_test_compile.L, -1), LUA_TFUNCTION);
#endif

	/* parameter 1 */
	const char* pattern = "asdf";
	lua_pushstring(g_test_compile.L, pattern);

	/* call pcre2.compile() */
	ASSERT_EQ_INT(lua_pcall(g_test_compile.L, 1, 1, 0), LUA_OK);

	/* Verify result */
	ASSERT_EQ_INT(lua_type(g_test_compile.L, -1), LUA_TUSERDATA);
}
