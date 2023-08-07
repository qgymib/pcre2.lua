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
"local pattern = \"(\\\\w+) (\\\\w+)\"" LF
"local code = lpcre2.compile(pattern)" LF
"local content = \"hello world!\"" LF
"local match = code:match(content)" LF
"local res = {" LF
"    [0]= \"hello world\"," LF
"    [1]= \"hello\"," LF
"    [2]= \"world\"," LF
"}" LF
LF
"assert(match ~= nil)" LF
"assert(match:group_count() == 2)" LF
"for i,v in ipairs(match:all_groups(content)) do" LF
"    assert(res[i] == v)" LF
"end" LF
LF
"local off,len = match:group_offset(0)" LF
"assert(off == 1)" LF
"assert(len == 11)" LF
LF
"local off,len = match:group_offset(1)" LF
"assert(off == 1)" LF
"assert(len == 5)" LF
LF
"local off,len = match:group_offset(2)" LF
"assert(off == 7)" LF
"assert(len == 5)" LF
;

	lua_setglobal(g_test_match.L, "lpcre2");
	luaL_openlibs(g_test_match.L);

	ASSERT_EQ_INT(luaL_dostring(g_test_match.L, lua_code), LUA_OK,
		"%s", lua_tostring(g_test_match.L, -1));
}
