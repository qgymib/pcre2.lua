#include "test.h"

typedef struct test_match
{
	lua_State* L;
} test_match_t;

static test_match_t g_test_match;

TEST_FIXTURE_SETUP(code)
{
	memset(&g_test_match, 0, sizeof(g_test_match));

	g_test_match.L = luaL_newstate();
	ASSERT_NE_PTR(g_test_match.L, NULL);

	ASSERT_EQ_INT(luaopen_lpcre2(g_test_match.L), 1);
}

TEST_FIXTURE_TEARDOWN(code)
{
	lua_close(g_test_match.L);
	g_test_match.L = NULL;
}

TEST_F(code, match_lua)
{
	const char* lua_code =
"local pattern = \"(\\\\w+) (\\\\w+)\"\n"
"local code = lpcre2.compile(pattern)\n"
"local content = \"hello world!\"\n"
"local match = code:match(content)\n"
"local res = {\n"
"    [0]= \"hello world\",\n"
"    [1]= \"hello\",\n"
"    [2]= \"world\",\n"
"}\n"
"assert(match ~= nil)\n"
"for i,v in match:iter(content) do\n"
"    assert(res[i] == v)\n"
"end\n";

	lua_setglobal(g_test_match.L, "lpcre2");
	luaL_openlibs(g_test_match.L);

	ASSERT_EQ_INT(luaL_dostring(g_test_match.L, lua_code), LUA_OK,
		"%s", lua_tostring(g_test_match.L, -1));
}
