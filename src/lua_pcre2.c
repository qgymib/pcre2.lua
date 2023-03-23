/**
 * This macro must be defined before including pcre2.h. For a program that uses
 * only one code unit width, it makes it possible to use generic function names
 * such as pcre2_compile().
 */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "lua.h"
#include "lauxlib.h"
#include "lua_pcre2.h"

#define LPCRE2_CODE_NAME            "_lpcre2_code"
#define LPCRE2_MATCH_DATA_NAME      "_lpcre2_match_data"
#define LPCRE2_MATCH_DATA_ITER_NAME "_lpcre2_match_data_iter"

#define LPCRE2_OPTION_MAP(xx)   \
    xx(LPCRE2_ALLOW_EMPTY_CLASS,            PCRE2_ALLOW_EMPTY_CLASS)            \
    xx(LPCRE2_NOTEMPTY,                     PCRE2_NOTEMPTY)                     \
    xx(LPCRE2_NOTEMPTY_ATSTART,             PCRE2_NOTEMPTY_ATSTART)             \
    xx(LPCRE2_DOTALL,                       PCRE2_DOTALL)                       \
    xx(LPCRE2_EXTENDED,                     PCRE2_EXTENDED)                     \
    xx(LPCRE2_MULTILINE,                    PCRE2_MULTILINE)                    \
    xx(LPCRE2_ENDANCHORED,                  PCRE2_ENDANCHORED)                  \
    xx(LPCRE2_NO_UTF_CHECK,                 PCRE2_NO_UTF_CHECK)                 \
    xx(LPCRE2_ANCHORED,                     PCRE2_ANCHORED)                     \
                                                                                \
    xx(LPCRE2_SUBSTITUTE_GLOBAL,            PCRE2_SUBSTITUTE_GLOBAL)            \
    xx(LPCRE2_SUBSTITUTE_EXTENDED,          PCRE2_SUBSTITUTE_EXTENDED)          \
    xx(LPCRE2_SUBSTITUTE_UNSET_EMPTY,       PCRE2_SUBSTITUTE_UNSET_EMPTY)       \
    xx(LPCRE2_SUBSTITUTE_UNKNOWN_UNSET,     PCRE2_SUBSTITUTE_UNKNOWN_UNSET)     \
    xx(LPCRE2_SUBSTITUTE_REPLACEMENT_ONLY,  PCRE2_SUBSTITUTE_REPLACEMENT_ONLY)

#define container_of(ptr, TYPE, member) \
    ((TYPE*)((char*)(ptr) - (char*)&((TYPE*)0)->member))

/*
 * For compatibility.
 */
#if LUA_VERSION_NUM == 501
#define luaL_newlib(L,l)  \
  (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))
#define luaL_newlibtable(L,l)   \
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

static void luaL_setfuncs(lua_State* L, const luaL_Reg* l, int nup)
{
    luaL_checkstack(L, nup, "too many upvalues");
    for (; l->name != NULL; l++)
    {
        if (l->func == NULL)
        {
            lua_pushboolean(L, 0);
        }
        else
        {
            int i;
            for (i = 0; i < nup; i++)
            {
                lua_pushvalue(L, -nup);
            }
            lua_pushcclosure(L, l->func, nup);
        }
        lua_setfield(L, -(nup + 2), l->name);
    }
    lua_pop(L, nup);
}

#endif

struct lpcre2_code
{
    pcre2_code* code;
    PCRE2_UCHAR message[256];
};

typedef struct lpcre2_match_data_impl
{
    lpcre2_match_data_t base;
    pcre2_match_data*   data;
} lpcre2_match_data_impl_t;

typedef struct lpcre2_match_data_iter
{
#define LPCRE2_MATCH_DATA_ITER_UV_STR   1
#define LPCRE2_MATCH_DATA_ITER_UV_MD    2

    lpcre2_match_data_impl_t* match_data;

    const char* content;
    size_t      content_sz;
} lpcre2_match_data_iter_t;

static int _lpcre2_code_gc(lua_State* L)
{
    lpcre2_code_t* code = lua_touserdata(L, 1);

    if (code->code != NULL)
    {
        pcre2_code_free(code->code);
        code->code = NULL;
    }

    return 0;
}

static int _lpcre2_match(lua_State* L)
{
    lpcre2_code_t* code = luaL_checkudata(L, 1, LPCRE2_CODE_NAME);

    size_t subject_sz = 0;
    const char* subject = luaL_checklstring(L, 2, &subject_sz);

    size_t offset = lua_tointeger(L, 3);
    uint32_t options = (uint32_t)lua_tointeger(L, 4);

    lpcre2_match(L, code, subject, subject_sz, offset, options);
    return 1;
}

static int _lpcre2_substitute(lua_State* L)
{
    lpcre2_code_t* code = luaL_checkudata(L, 1, LPCRE2_CODE_NAME);

    size_t content_sz = 0;
    const char* content = luaL_checklstring(L, 2, &content_sz);

    size_t replace_sz = 0;
    const char* replace = luaL_checklstring(L, 3, &replace_sz);

    uint32_t options = (uint32_t)lua_tointeger(L, 4);

    lpcre2_substitute(L, code, content, content_sz, replace, replace_sz,
        options, NULL);

    return 1;
}

static int _lpcre2_match_data_gc(lua_State* L)
{
    lpcre2_match_data_impl_t* data = lua_touserdata(L, 1);

    if (data->data != NULL)
    {
        pcre2_match_data_free(data->data);
        data->data = NULL;
    }

    return 0;
}

static int _lpcre2_compile(lua_State* L)
{
    size_t pattern_sz = 0;
    const char* pattern = luaL_checklstring(L, 1, &pattern_sz);

    uint32_t options = (uint32_t)lua_tointeger(L, 2);

    lpcre2_compile(L, pattern, pattern_sz, options);

    return 1;
}

static void _lpcre2_check_options(lua_State* L)
{
#define LPCRE2_CHECK_OPTION(OPT_LPCRE2, OPT_PCRE2)  \
    do {\
        int lpcre2_opt_val = OPT_LPCRE2;\
        int pcre2_opt_val = OPT_PCRE2;\
        if (lpcre2_opt_val != pcre2_opt_val) {\
            luaL_error(L, "except %s == %s, actual %I vs %I",\
                #OPT_LPCRE2, #OPT_PCRE2,\
                (lua_Integer)lpcre2_opt_val, (lua_Integer)pcre2_opt_val);\
        }\
    } while (0);

    LPCRE2_OPTION_MAP(LPCRE2_CHECK_OPTION);

#undef LPCRE2_CHECK_OPTION
}

static void _lpcre2_set_options(lua_State* L)
{
#define LLCRE2_SET_OPTION(OPT_LPCRE2, OPT_PCRE2)    \
    lua_pushinteger(L, OPT_LPCRE2);\
    lua_setfield(L, -2, #OPT_LPCRE2);

    LPCRE2_OPTION_MAP(LLCRE2_SET_OPTION);

#undef LLCRE2_SET_OPTION
}

static int _lpcre2_match_data_iter_next(lua_State* L)
{
	size_t len = 0;
	size_t beg_off = 0;

	lpcre2_match_data_iter_t* iter = lua_touserdata(L, 1);
	lpcre2_match_data_impl_t* match_data = iter->match_data;

	if (match_data->base.rc <= 0)
	{
		return 0;
	}

	int idx = 0;
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		idx = (int)lua_tointeger(L, 2) + 1;
	}

	if (idx > match_data->base.rc)
	{
		return 0;
	}

	beg_off = lpcre2_match_data_ovector(L, &match_data->base, idx, &len);
	lua_pushinteger(L, idx);
	lua_pushlstring(L, iter->content + beg_off, len);

	return 2;
}

static int _lpcre2_match_data_iter(lua_State* L)
{
	lpcre2_match_data_impl_t* match_data = luaL_checkudata(L, 1, LPCRE2_MATCH_DATA_NAME);

	size_t content_sz;
	const char* content = luaL_checklstring(L, 2, &content_sz);

	lua_pushcfunction(L, _lpcre2_match_data_iter_next);

	lpcre2_match_data_iter_t* iter = lua_newuserdata(L, sizeof(lpcre2_match_data_iter_t));
	iter->match_data = match_data;
	iter->content = content;
	iter->content_sz = content_sz;

	lua_pushnil(L);

	return 3;
}

int luaopen_lpcre2(lua_State* L)
{
#if LUA_VERSION_NUM >= 502
    luaL_checkversion(L);
#endif

    _lpcre2_check_options(L);

    static const luaL_Reg pcre2_apis[] = {
        { "compile",    _lpcre2_compile },
        { NULL,         NULL }
    };
    luaL_newlib(L, pcre2_apis);

    _lpcre2_set_options(L);

    return 1;
}

lpcre2_code_t* lpcre2_compile(lua_State* L, const char* pattern,
    size_t length, uint32_t options)
{
    lpcre2_code_t* code = lua_newuserdata(L, sizeof(lpcre2_code_t));
    code->code = NULL;

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lpcre2_code_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
        { "match",      _lpcre2_match },
        { "substitute", _lpcre2_substitute },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LPCRE2_CODE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    int errcode;
    PCRE2_SIZE erroffset;
    code->code = pcre2_compile((PCRE2_SPTR)pattern,
        length,
        options,
        &errcode,
        &erroffset,
        NULL);
    if (code->code == NULL)
    {
        pcre2_get_error_message(errcode, code->message,
            sizeof(code->message) / sizeof(PCRE2_UCHAR));
        luaL_error(L, "%s", code->message);
        return NULL;
    }

    return code;
}

const char* lpcre2_substitute(lua_State* L, lpcre2_code_t* code,
    const char* subject, size_t length, const char* replacement, size_t rlength,
    uint32_t options, size_t* len)
{
    int ret;
    char* addr;

    PCRE2_SIZE outlength = 0;
    ret = pcre2_substitute(code->code,
        (PCRE2_SPTR)subject,
        length,
        0,
        options | PCRE2_SUBSTITUTE_OVERFLOW_LENGTH,
        NULL,
        NULL,
        (PCRE2_SPTR)replacement,
        rlength,
        NULL,
        &outlength);
    if (ret != PCRE2_ERROR_NOMEMORY)
    {
        goto error;
    }

    luaL_Buffer buf;

#if LUA_VERSION_NUM >= 502
    addr = luaL_buffinitsize(L, &buf, outlength);
#else
    luaL_buffinit(L, &buf);
    if ((addr = malloc(outlength)) == NULL)
    {
        luaL_error(L, "not enough memory");
        return NULL;
    }
#endif

    ret = pcre2_substitute(code->code,
        (PCRE2_SPTR)subject,
        length,
        0,
        options,
        NULL,
        NULL,
        (PCRE2_SPTR)replacement,
        rlength,
        (PCRE2_UCHAR*)addr,
        &outlength);
    if (ret < 0)
    {
        goto error;
    }

#if LUA_VERSION_NUM >= 502
    luaL_addsize(&buf, outlength);
#else
    luaL_addlstring(&buf, addr, outlength);
    free(addr); addr = NULL;
#endif

    luaL_pushresult(&buf);

    return lua_tolstring(L, -1, len);

error:
    pcre2_get_error_message(ret, code->message,
        sizeof(code->message) / sizeof(PCRE2_UCHAR));
    luaL_error(L, "%s", code->message);
    return NULL;
}

lpcre2_match_data_t* lpcre2_match(lua_State* L, lpcre2_code_t* code,
    const char* subject, size_t length, size_t offset, uint32_t options)
{
    lpcre2_match_data_impl_t* data = lua_newuserdata(L, sizeof(lpcre2_match_data_impl_t));
    data->data = NULL;

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lpcre2_match_data_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
        { "iter",   _lpcre2_match_data_iter },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, LPCRE2_MATCH_DATA_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    if ((data->data = pcre2_match_data_create_from_pattern(code->code, NULL)) == NULL)
    {
        luaL_error(L, "out of memory");
        return NULL;
    }

    data->base.rc = pcre2_match(code->code,
        (PCRE2_SPTR)subject,
        length,
        offset,
        options,
        data->data,
        NULL);
    if (data->base.rc < 0)
    {
        if (data->base.rc == PCRE2_ERROR_NOMATCH)
        {
            data->base.rc = -1;
            return &data->base;
        }

        pcre2_get_error_message(data->base.rc, code->message,
            sizeof(code->message) / sizeof(PCRE2_UCHAR));
        luaL_error(L, "%s", code->message);
        return NULL;
    }

    data->base.rc--;

    return &data->base;
}

size_t lpcre2_match_data_ovector(lua_State* L, lpcre2_match_data_t* match_data,
    size_t idx, size_t* len)
{
    lpcre2_match_data_impl_t* real_match_data = container_of(match_data, lpcre2_match_data_impl_t, base);
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(real_match_data->data);

    if (idx >= INT_MAX || (int)idx > match_data->rc)
    {
        luaL_error(L, "out of bound");
        return (size_t) -1;
    }

    size_t beg_off = ovector[2 * idx];
    size_t end_off = ovector[2 * idx + 1];

    if (len != NULL)
    {
        *len = end_off - beg_off;
    }

    return beg_off;
}
