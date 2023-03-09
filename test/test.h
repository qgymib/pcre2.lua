#ifndef __TEST_H__
#define __TEST_H__

#include "cutest.h"
#include "lua_pcre2.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/*
 * For compatibility.
 */
#if LUA_VERSION_NUM == 501
#	define LUA_OK	0
#endif

#ifdef __cplusplus
}
#endif

#endif
