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

#define LPCRE2_CODE_NAME    "_lpcre2_code"

#define LPCRE2_OPTION_MAP(xx)   \
    xx(LPCRE2_ALLOW_EMPTY_CLASS,            PCRE2_ALLOW_EMPTY_CLASS)        \
    xx(LPCRE2_DOTALL,                       PCRE2_DOTALL)                   \
    xx(LPCRE2_EXTENDED,                     PCRE2_EXTENDED)                 \
    xx(LPCRE2_MULTILINE,                    PCRE2_MULTILINE)                \
                                                                            \
    xx(LPCRE2_SUBSTITUTE_GLOBAL,            PCRE2_SUBSTITUTE_GLOBAL)        \
    xx(LPCRE2_SUBSTITUTE_EXTENDED,          PCRE2_SUBSTITUTE_EXTENDED)      \
    xx(LPCRE2_SUBSTITUTE_UNSET_EMPTY,       PCRE2_SUBSTITUTE_UNSET_EMPTY)   \
    xx(LPCRE2_SUBSTITUTE_UNKNOWN_UNSET,     PCRE2_SUBSTITUTE_UNKNOWN_UNSET) \
    xx(LPCRE2_SUBSTITUTE_REPLACEMENT_ONLY,  PCRE2_SUBSTITUTE_REPLACEMENT_ONLY)

/*
 * For compatibility.
 */
#if LUA_VERSION_NUM == 501
#define luaL_newlib(L,l)  \
  (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))
#define luaL_newlibtable(L,l)	\
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

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lpcre2_code_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
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
