#include <gccore.h>
#include <wiiuse/wpad.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "wiired.h"
#include "main.h"
#include "system.h"
#include "titles.h"
#include "guigamesview.h"

typedef enum {
    REQ_NONE = 0,
    REQ_BOOT,
    REQ_UI,
    REQ_REFRESH
} RequestKind;

static RequestKind reqKind = REQ_NONE;
static std::string reqType;     //wii | gc | vc | chan | hb | settings
static std::string reqPath;

void wiiredParseArgs(int argc, char** argv) {
    reqKind = REQ_NONE;
    if (argc < 3 || argv == NULL || argv[1] == NULL || argv[2] == NULL)
        return;
    if (strcmp(argv[1], "wiired") != 0)
        return;

    //Copy everything now: the command line lives in memory that gets reused later
    std::string cmd = argv[2];
    if (argc >= 4 && argv[3] != NULL) reqType = argv[3];
    if (argc >= 5 && argv[4] != NULL) reqPath = argv[4];

    if (cmd == "boot" && !reqType.empty() && !reqPath.empty())
        reqKind = REQ_BOOT;
    else if (cmd == "ui")
        reqKind = REQ_UI;
    else if (cmd == "refresh")
        reqKind = REQ_REFRESH;

    printf("Wiired request: %s %s %s\n", cmd.c_str(), reqType.c_str(), reqPath.c_str());
}

bool wiiredHasRequest() {
    return reqKind != REQ_NONE;
}

bool wiiredInstalled() {
    FILE* fp = fopen(WIIRED_DOL_PATH, "rb");
    if (fp == NULL)
        return false;
    fclose(fp);
    return true;
}

//Names come from WiiTDB or the disc header: keep the file format intact
static std::string cleanField(const std::string& in) {
    std::string out = in;
    for (auto& c : out) {
        if (c == '\t' || c == '\r' || c == '\n')
            c = ' ';
    }
    return out;
}

static void writeList(FILE* fp, const char* type, const std::vector<GameContainer>& list) {
    for (const auto& gc : list) {
        fprintf(fp, "%s\t%s\t%s\t%s\t%s\n", type,
            cleanField(gc.gameIDString).c_str(),
            cleanField(gc.name).c_str(),
            cleanField(gc.path).c_str(),
            cleanField(gc.coverPath).c_str());
    }
}

void wiiredWriteLibrary() {
    //Cover paths are filled in by background threads; wait so they are complete
    waitWiiCovers();
    waitGCCovers();
    waitVCCovers();
    waitWiiChannelsCovers();

    const char* tmpPath = WIIRED_LIBRARY_PATH ".tmp";
    FILE* fp = fopen(tmpPath, "wb");
    if (fp == NULL) {
        printf("Couldn't write %s\n", tmpPath);
        return;
    }
    fprintf(fp, "#RVLOADER-LIBRARY 1\n");
    writeList(fp, "wii", wiiGames);
    writeList(fp, "gc", gcGames);
    writeList(fp, "vc", vcGames);
    writeList(fp, "chan", wiiChannels);
    bool ok = !ferror(fp);
    fclose(fp);

    if (ok) {
        remove(WIIRED_LIBRARY_PATH);
        rename(tmpPath, WIIRED_LIBRARY_PATH);
    } else {
        remove(tmpPath);
    }
}

bool wiiredLaunch() {
    if (!wiiredInstalled())
        return false;
    wiiredWriteLibrary();
    bootDOL(WIIRED_DOL_PATH, "", false);
    return false; //Only reached if booting failed
}

//Hold B on any controller while RVLoader loads to stay in RVLoader
static bool stayInRVLoaderHeld() {
    PAD_ScanPads();
    WPAD_ScanPads();
    for (int i = 0; i < 4; i++) {
        if (PAD_ButtonsHeld(i) & PAD_BUTTON_B)
            return true;
        if (WPAD_ButtonsHeld(i) & WPAD_BUTTON_B)
            return true;
    }
    return false;
}

void wiiredStartupHandoff() {
    if (reqKind == REQ_REFRESH) {
        wiiredLaunch();
        return; //Wiired missing: carry on in RVLoader
    }

    if (reqKind != REQ_NONE)
        return; //boot / ui requests are handled after the theme is loaded

    int startup = STARTUP_MENU_DEFAULT;
    mainConfig.getValue(STARTUP_MENU_KEY, &startup);
    if (startup != 1)
        return;

    if (stayInRVLoaderHeld())
        return;

    wiiredLaunch();
}

static bool typeFromString(const std::string& s, TitleType* type) {
    if (s == "gc")   { *type = GC_GAME;     return true; }
    if (s == "wii")  { *type = WII_GAME;    return true; }
    if (s == "vc")   { *type = WII_VC;      return true; }
    if (s == "chan") { *type = WII_CHANNEL; return true; }
    return false;
}

//Element ids from theme.xml
static const char* viewIdFromString(const std::string& s) {
    if (s == "gc")       return "GCGamesView";
    if (s == "wii")      return "WiiGamesView";
    if (s == "vc")       return "VCGamesView";
    if (s == "chan")     return "WiiChannelsView";
    if (s == "hb")       return "WiiHBView";
    if (s == "settings") return "SettingsView";
    return NULL;
}

void wiiredHandleRequestAfterTheme() {
    if (reqKind == REQ_UI) {
        const char* viewId = viewIdFromString(reqType);
        if (viewId != NULL)
            mainWindowSwitchElement(viewId);
        reqKind = REQ_NONE;
        return;
    }

    if (reqKind != REQ_BOOT)
        return;
    reqKind = REQ_NONE;

    TitleType type;
    if (!typeFromString(reqType, &type))
        return;

    GuiGamesView* view = GuiGamesView::getView(type);
    if (view == NULL)
        return;

    int idx = view->findTitleByPath(reqPath);
    if (idx < 0) {
        //The title isn't there any more: just show that list
        const char* viewId = viewIdFromString(reqType);
        if (viewId != NULL)
            mainWindowSwitchElement(viewId);
        return;
    }

    //GameCube titles never return from here. Wii titles start Hiidra's boot thread
    //and return; the main loop then shows the boot screen until the game starts.
    view->bootTitle((u32)idx, false);
}
