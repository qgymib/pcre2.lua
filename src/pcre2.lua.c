/**
 * This macro must be defined before including pcre2.h. For a program that uses
 * only one code unit width, it makes it possible to use generic function names
 * such as pcre2_compile().
 */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "lua.h"
#include "lauxlib.h"
#include "pcre2.lua.h"

#define LPCRE2_CODE_NAME            "_lpcre2_code"
#define LPCRE2_MATCH_DATA_NAME      "_lpcre2_match_data"
#define LPCRE2_MATCH_DATA_ITER_NAME "_lpcre2_match_data_iter"

#define LPCRE2_OPTION_MAP(xx)   \
    xx(PCRE2_ALLOW_EMPTY_CLASS)            \
    xx(PCRE2_NOTEMPTY)                     \
    xx(PCRE2_NOTEMPTY_ATSTART)             \
    xx(PCRE2_DOTALL)                       \
    xx(PCRE2_EXTENDED)                     \
    xx(PCRE2_MULTILINE)                    \
    xx(PCRE2_ENDANCHORED)                  \
    xx(PCRE2_NO_UTF_CHECK)                 \
    xx(PCRE2_ANCHORED)                     \
                                           \
    xx(PCRE2_SUBSTITUTE_GLOBAL)            \
    xx(PCRE2_SUBSTITUTE_EXTENDED)          \
    xx(PCRE2_SUBSTITUTE_UNSET_EMPTY)       \
    xx(PCRE2_SUBSTITUTE_UNKNOWN_UNSET)     \
    xx(PCRE2_SUBSTITUTE_REPLACEMENT_ONLY)

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

#if LUA_VERSION_NUM <= 502

static void lua_seti(lua_State* L, int idx, lua_Integer n)
{
    int sp = lua_gettop(L); // This is where value at.
    int sp_table = idx > 0 ? idx : sp + idx + 1;

    lua_pushinteger(L, n); // sp+1: where key at.
    lua_insert(L, sp); // route

    lua_settable(L, sp_table);
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

    if (lpcre2_match(L, code, subject, subject_sz, offset, options) == NULL)
    {
        return 0;
    }
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

static void _lpcre2_set_options(lua_State* L)
{
#define LLCRE2_SET_OPTION(OPT_PCRE2)    \
    lua_pushinteger(L, OPT_PCRE2);\
    lua_setfield(L, -2, #OPT_PCRE2);

    LPCRE2_OPTION_MAP(LLCRE2_SET_OPTION);

#undef LLCRE2_SET_OPTION
}

int luaopen_lpcre2(lua_State* L)
{
#if LUA_VERSION_NUM >= 502
    luaL_checkversion(L);
#endif

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
        luaL_error(L, "compile pattern `%s` error at %d: %s",
            pattern, (int)erroffset, code->message);
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

static int _lpcre2_match_group(lua_State* L)
{
    lpcre2_match_data_impl_t* match_data = luaL_checkudata(L, 1, LPCRE2_MATCH_DATA_NAME);

    size_t content_sz;
    const char* content = luaL_checklstring(L, 2, &content_sz);

    int idx = (int)luaL_checkinteger(L, 3);

    if (idx > match_data->base.rc)
    {
        lua_pushnil(L);
        return 1;
    }

    size_t len = 0;
    size_t offset = lpcre2_match_data_ovector(L, &match_data->base, idx, &len);

    lua_pushlstring(L, content + offset, len);
    return 1;
}

static int _lpcre2_match_all_groups(lua_State* L)
{
    int idx;
    lua_settop(L, 2);

    lpcre2_match_data_impl_t* match_data = luaL_checkudata(L, 1, LPCRE2_MATCH_DATA_NAME);

    size_t content_sz;
    const char* content = luaL_checklstring(L, 2, &content_sz);

    lua_newtable(L); // sp:3
    for (idx = 0; idx <= match_data->base.rc; idx++)
    {
        size_t len = 0;
        size_t offset = lpcre2_match_data_ovector(L, &match_data->base, idx, &len);

        const char* data = content + offset;
        lua_pushlstring(L, data, len); // sp:4

        lua_seti(L, -2, idx);
    }

    return 1;
}

static int _lpcre2_match_group_count(lua_State* L)
{
    lpcre2_match_data_impl_t* match_data = luaL_checkudata(L, 1, LPCRE2_MATCH_DATA_NAME);

    lua_pushinteger(L, match_data->base.rc);
    return 1;
}

static int _lpcre2_match_group_offset(lua_State* L)
{
    lpcre2_match_data_impl_t* match_data = luaL_checkudata(L, 1, LPCRE2_MATCH_DATA_NAME);

    lua_Integer group_idx = luaL_checkinteger(L, 2);
    if (group_idx < 0 || group_idx > match_data->base.rc)
    {
        return luaL_error(L, "index out of range.");
    }

    size_t len = 0;
    size_t offset = lpcre2_match_data_ovector(L, &match_data->base, group_idx, &len);

    lua_pushinteger(L, offset + 1);
    lua_pushinteger(L, offset + 1 + len - 1);
    return 2;
}

lpcre2_match_data_t* lpcre2_match(lua_State* L, lpcre2_code_t* code,
    const char* subject, size_t length, size_t offset, uint32_t options)
{
    lpcre2_match_data_impl_t* data = lua_newuserdata(L, sizeof(lpcre2_match_data_impl_t));
    data->data = NULL;

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lpcre2_match_data_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "all_groups",     _lpcre2_match_all_groups },
        { "group",          _lpcre2_match_group },
        { "group_count",    _lpcre2_match_group_count },
        { "group_offset",   _lpcre2_match_group_offset },
        { NULL,             NULL },
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
            lua_pop(L, 1);
            return NULL;
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
