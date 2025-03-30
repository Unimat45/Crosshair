#include "luaFunctions.h"

int cRGB(lua_State *L)
{
    int r, g, b;

    // Get the arguments from the lua state,
    // and mask them as 8 bit numbers.
    r = (int)lua_tointeger(L, 1) & 0xFF;
    g = (int)lua_tointeger(L, 2) & 0xFF;
    b = (int)lua_tointeger(L, 3) & 0xFF;

    // Combine them into a single value.
    int result = (r << 16) | (g << 8) | b;

    // Pushes it to the stack as the return value.
    lua_pushinteger(L, result);

    // Only return 1 value.
    return 1;
}

void registerCustomFunctions(lua_State *L) { lua_register(L, "RGB", cRGB); }