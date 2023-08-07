#ifndef __LUA_PCRE2_H__
#define __LUA_PCRE2_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State;

typedef struct lpcre2_match_data
{
    /**
     * @brief Match result.
     * The value have following meanings:
     * + 0: Match success.
     * + >0: Match success, and the value is the number of captured groups.
     * + <0: Match failure.
     */
    int rc;
} lpcre2_match_data_t;

/**
 * @defgroup LUA_PCRE2_LUAOPEN entrypoint
 *
 * This is the entrypoint of lpcre2.
 *
 * @{
 */

typedef enum lpcre2_option
{
    /**
     * @brief Allow empty classes
     */
    LPCRE2_ALLOW_EMPTY_CLASS            = 0x00000001u,

    /**
     * @brief An empty string is not a valid match.
     */
    LPCRE2_NOTEMPTY                     = 0x00000004u,

    /**
     * @brief An empty string at the start of the subject is not a valid match.
     */
    LPCRE2_NOTEMPTY_ATSTART             = 0x00000008u,

    /**
     * @brief `.` matches anything including NL.
     */
    LPCRE2_DOTALL                       = 0x00000020u,

    /**
     * @brief Ignore white space and # comments.
     */
    LPCRE2_EXTENDED                     = 0x00000080u,

    /**
     * @brief `^` and `$` match newlines within data.
     */
    LPCRE2_MULTILINE                    = 0x00000400u,

    /**
     * @brief Pattern can match only at end of subject.
     */
    LPCRE2_ENDANCHORED                  = 0x20000000u,

    /**
     * @brief Do not check the subject for UTF validity (only relevant if
     *   `PCRE2_UTF` was set at compile time).
     */
    LPCRE2_NO_UTF_CHECK                 = 0x40000000u,

    /**
     * @brief Match only at the first position.
     */
    LPCRE2_ANCHORED                     = 0x80000000u,

    /**
     * @brief Replace all occurrences in the subject.
     */
    LPCRE2_SUBSTITUTE_GLOBAL            = 0x00000100u,

    /**
     * @brief Do extended replacement processing.
     */
    LPCRE2_SUBSTITUTE_EXTENDED          = 0x00000200u,

    /**
     * @brief Simple unset insert = empty string.
     */
    LPCRE2_SUBSTITUTE_UNSET_EMPTY       = 0x00000400u,

    /**
     * @brief Treat unknown group as unset.
     */
    LPCRE2_SUBSTITUTE_UNKNOWN_UNSET     = 0x00000800u,

    /**
     * @brief Return only replacement string(s).
     */
    LPCRE2_SUBSTITUTE_REPLACEMENT_ONLY  = 0x00020000u,
} lpcre2_option_t;

/**
 * @brief Load pcre2 package.
 * 
 * + Shared library. If you compile lpcre2 as shared library, generally you
 *   don't need to call this function. Use syntax like
 *   `lpcre2 = require('lpcre2')` and Lua VM will do the reset for you.
 * + Static library. If you compile lpcre2 as static library, you need to
 *   manually load lpcre2. A classic usage is:
 *
 *   ```c
 *   luaopen_lpcre2(L)
 *   lua_setglobal(L, "lpcre2")
 *   ```
 *
 *   This will load lpcre2 into global namespace, and you can safe to use
 *   `lcpre2.compile()`.
 *
 * @param[in] L     Lua Stack.
 * @return          Always 1.
 */
int luaopen_lpcre2(struct lua_State* L);

/**
 * @}
 */

/**
 * @defgroup LUA_PCRE2_COMPILE compile
 * @{
 */

typedef struct lpcre2_code lpcre2_code_t;

/**
 * @brief Compile a regular expression pattern and push it on top of \p L.
 * @param[in] L         Lua Stack.
 * @param[in] pattern   A string containing expression to be compiled.
 * @param[in] length    The length of the string.
 * @param[in] options   Option bits. Support following option:
 *                      + #LPCRE2_ALLOW_EMPTY_CLASS
 *                      + #LPCRE2_DOTALL
 *                      + #LPCRE2_EXTENDED
 *                      + #LPCRE2_MULTILINE
 * @return The compiled regular expression pattern. If failed, an
 *   error string is pushed on top of stack, and function does not return.
 * @see https://www.pcre.org/current/doc/html/pcre2_compile.html
 */
lpcre2_code_t* lpcre2_compile(struct lua_State* L, const char* pattern,
    size_t length, uint32_t options);

/**
 * @}
 */

/**
 * @defgroup LUA_PCRE2_SUBSTITUTE substitute
 * @{
 */

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
 * @param[in] options       Option bits. Support following option:
 *                          + #LPCRE2_SUBSTITUTE_GLOBAL
 *                          + #LPCRE2_SUBSTITUTE_EXTENDED
 *                          + #LPCRE2_SUBSTITUTE_UNSET_EMPTY
 *                          + #LPCRE2_SUBSTITUTE_UNKNOWN_UNSET
 *                          + #LPCRE2_SUBSTITUTE_REPLACEMENT_ONLY
 * @param[out] len          The size of replaced string (not including NULL terminator).
 * @return Points to the replaced string. If error occur, an
 *   error string is pushed on top of stack, and function does not return.
 */
const char* lpcre2_substitute(struct lua_State* L, lpcre2_code_t* code,
    const char* subject, size_t length, const char* replacement, size_t rlength,
    uint32_t options, size_t* len);

/**
 * @}
 */

/**
 * @defgroup LUA_PCRE2_MATCH match
 * @{
 */

/**
 * @brief Matches a compiled regular expression against a given subject string,
 *   and push match result on top of Lua stack \p L.
 * @param[in] L         Lua Stack.
 * @param[in] code      The compiled regular expression pattern.
 * @param[in] subject   The subject string.
 * @param[in] length    Length of the subject string.
 * @param[in] offset    Offset in the subject at which to start matching.
 * @param[in] options   Option bits. Support following option:
 *                      + #LPCRE2_ENDANCHORED
 *                      + #LPCRE2_ANCHORED
 *                      + #LPCRE2_NOTEMPTY
 *                      + #LPCRE2_NOTEMPTY_ATSTART
 *                      + #LPCRE2_NO_UTF_CHECK
 * @return              Match result.
 */
lpcre2_match_data_t* lpcre2_match(struct lua_State* L, lpcre2_code_t* code,
    const char* subject, size_t length, size_t offset, uint32_t options);

/**
 * @brief Get offset of captured group.
 * @param[in] L             Lua Stack.
 * @param[in] match_data    Match result.
 * @param[in] idx           Group index. 0 is the whole match, 1 is the first match.
 * @param[out] len          The length of captured group.
 * @return The start position of captured group.
 */
size_t lpcre2_match_data_ovector(struct lua_State* L,
    lpcre2_match_data_t* match_data, size_t idx, size_t* len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
