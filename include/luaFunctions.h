#ifndef LUAFUNCTIONS_H
#define LUAFUNCTIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lua.h"

    /*
     * Used to register custom C function that will be accessible in the lua
     * config
     */
    void registerCustomFunctions(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // LUAFUNCTIONS_H