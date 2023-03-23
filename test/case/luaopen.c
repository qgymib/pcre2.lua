#include "test.h"

typedef struct test_luaopen
{
	lua_State*	L;
} test_luaopen_t;

static test_luaopen_t g_test_luaopen;

TEST_FIXTURE_SETUP(lpcre2)
{
	memset(&g_test_luaopen, 0, sizeof(g_test_luaopen));

	g_test_luaopen.L = luaL_newstate();
	ASSERT_NE_PTR(g_test_luaopen.L, NULL);
}

TEST_FIXTURE_TEARDOWN(lpcre2)
{
	lua_close(g_test_luaopen.L);
	g_test_luaopen.L = NULL;
}

TEST_F(lpcre2, luaopen)
{
	ASSERT_EQ_INT(luaopen_lpcre2(g_test_luaopen.L), 1);
	ASSERT_EQ_INT(lua_gettop(g_test_luaopen.L), 1);
}
