#include <gccore.h>
#include <stdio.h>
#include <lua.hpp>
#include "system.h"
#include "main.h"
#include "wiired.h"

//Config key storing which menu the Home icon on the C-stick wheel boots
//0 = stock Wii Menu, 1 = custom home menu (Wiired)
#define HOME_TARGET_KEY     "HomeTarget"
#define HOME_TARGET_DEFAULT 0

static int lua_Sys_debug(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    const char* str = luaL_checkstring(L, 1);

    printf(str);

    return 0;
}

static int lua_Sys_bootSysMenu(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    bootSysMenu();

    return 0;
}

static int lua_Sys_bootPriiloader(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    bootPriiloader();

    return 0;
}

static int lua_Sys_bootInstaller(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    bootDOL("/apps/RVLoader/installer.dol", "", false);

    return 0;
}

// Sys.bootDOL(path [, patchMX])
// Boots a DOL from the mounted drive (e.g. "/apps/wiired/boot.dol").
// Returns false if the file can't be opened; on success it never returns.
static int lua_Sys_bootDOL(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc < 1 || argc > 2) {
        return luaL_error(L, "wrong number of arguments");
    }

    const char* path = luaL_checkstring(L, 1);
    bool patchMX = (argc == 2) ? lua_toboolean(L, 2) : false;

    //Check the file exists first so the caller can fall back to something else
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        lua_pushboolean(L, false);
        return 1;
    }
    fclose(fp);

    bootDOL(path, "", patchMX);

    //Only reached if bootDOL failed
    lua_pushboolean(L, false);
    return 1;
}

// Sys.getHomeTarget() -> 0 = Wii Menu, 1 = custom home menu
static int lua_Sys_getHomeTarget(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    int target;
    if (!mainConfig.getValue(HOME_TARGET_KEY, &target))
        target = HOME_TARGET_DEFAULT;

    lua_pushinteger(L, target);

    return 1;
}

// Sys.setHomeTarget(target) - saves the choice to /rvloader/config.cfg
static int lua_Sys_setHomeTarget(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    mainConfig.setValue(HOME_TARGET_KEY, (int)luaL_checkinteger(L, 1));
    mainConfig.save(MAINCONFIG_PATH);

    return 0;
}

// Sys.launchWiired() - write the game library and start Wiired.
// Returns false if Wiired isn't installed (never returns on success).
static int lua_Sys_launchWiired(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, wiiredLaunch());
    return 1;
}

// Sys.isWiiredInstalled()
static int lua_Sys_isWiiredInstalled(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, wiiredInstalled());
    return 1;
}

// Sys.getStartupMenu() -> 0 = RVLoader, 1 = Wiired
static int lua_Sys_getStartupMenu(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    int value;
    if (!mainConfig.getValue(STARTUP_MENU_KEY, &value))
        value = STARTUP_MENU_DEFAULT;

    lua_pushinteger(L, value);
    return 1;
}

// Sys.setStartupMenu(value) - saves the choice to /rvloader/config.cfg
static int lua_Sys_setStartupMenu(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    mainConfig.setValue(STARTUP_MENU_KEY, (int)luaL_checkinteger(L, 1));
    mainConfig.save(MAINCONFIG_PATH);
    return 0;
}

static int lua_Sys_reboot(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    reboot();

    return 0;
}

static int lua_Sys_getVersion(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, (float)VER_MAJOR + VER_MINOR / 10.0f);

    return 1;
}

static const luaL_Reg Sys_functions[] = {
    {"debug", lua_Sys_debug},
    {"bootSysMenu", lua_Sys_bootSysMenu},
    {"bootPriiloader", lua_Sys_bootPriiloader},
    {"bootInstaller", lua_Sys_bootInstaller},
    {"bootDOL", lua_Sys_bootDOL},
    {"getHomeTarget", lua_Sys_getHomeTarget},
    {"setHomeTarget", lua_Sys_setHomeTarget},
    {"launchWiired", lua_Sys_launchWiired},
    {"isWiiredInstalled", lua_Sys_isWiiredInstalled},
    {"getStartupMenu", lua_Sys_getStartupMenu},
    {"setStartupMenu", lua_Sys_setStartupMenu},
    {"reboot", lua_Sys_reboot},
    {"getVersion", lua_Sys_getVersion},
    {NULL, NULL}
};

void luaRegisterSysLib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, Sys_functions, 0);
    lua_setglobal(L, "Sys");
}
