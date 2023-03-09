#ifndef __LUA_PCRE2_H__
#define __LUA_PCRE2_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State;
typedef struct lpcre2_code lpcre2_code_t;

typedef enum lpcre2_option
{
    LPCRE2_ALLOW_EMPTY_CLASS            = 0x00000001u,
    LPCRE2_DOTALL                       = 0x00000020u,
    LPCRE2_EXTENDED                     = 0x00000080u,
    LPCRE2_MULTILINE                    = 0x00000400u,
    

    LPCRE2_SUBSTITUTE_GLOBAL            = 0x00000100u,  /**< For #lpcre2_substitute() */
    LPCRE2_SUBSTITUTE_EXTENDED          = 0x00000200u,  /**< For #lpcre2_substitute() */
    LPCRE2_SUBSTITUTE_UNSET_EMPTY       = 0x00000400u,  /**< For #lpcre2_substitute() */
    LPCRE2_SUBSTITUTE_UNKNOWN_UNSET     = 0x00000800u,  /**< For #lpcre2_substitute() */
    LPCRE2_SUBSTITUTE_REPLACEMENT_ONLY  = 0x00020000u,  /**< For #lpcre2_substitute() */
} lpcre2_option_t;

/**
 * @brief Load pcre2 package.
 * @param[in] L     Lua Stack.
 * @return          Always 1.
 */
int luaopen_lpcre2(struct lua_State* L);

/**
 * @brief Compile a regular expression pattern and push it on top of \p L.
 * @param[in] L         Lua Stack.
 * @param[in] pattern   A string containing expression to be compiled.
 * @param[in] length    The length of the string.
 * @param[in] options   Option bits. Checkout #lpcre2_option_t.
 * @return              The compiled regular expression pattern. If failed, an
 *   error string is pushed on top of stack, and function does not return.
 * @see https://www.pcre.org/current/doc/html/pcre2_compile.html
 */
lpcre2_code_t* lpcre2_compile(struct lua_State* L, const char* pattern,
    size_t length, uint32_t options);

/**
 * @brief Matches a compiled regular expression against a given \p subject,
 *   makes a copy of the \p subject, substituting a \p replacement string for
 *   what was matched, and push on top of Lua stack \p L.
 * @param[in] L             Lua Stack.
 * @param[in] code          The compiled regular expression pattern.
 * @param[in] subject       The subject string.
 * @param[in] length        Length of the subject string.
 * @param[in] replacement   Points to the replacement string.
 * @param[in] rlength       Length of the replacement string.
 * @param[out] len          The size of replaced string (not including NULL terminator).
 * @return                  Points to the replaced string. If error occur, an
 *   error string is pushed on top of stack, and function does not return.
 */
const char* lpcre2_substitute(struct lua_State* L, lpcre2_code_t* code,
    const char* subject, size_t length, const char* replacement, size_t rlength,
    uint32_t options, size_t* len);

#ifdef __cplusplus
}
#endif
#endif
