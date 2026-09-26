#pragma once

/*
 * Wiired bridge.
 *
 * Wiired (usb:/apps/wiired/boot.dol) is used as the front end; RVLoader does the
 * actual game booting. The two talk through:
 *
 *   1. A library file, /rvloader/wiired_library.txt, written by RVLoader before it
 *      hands off to Wiired. One title per line, tab separated:
 *
 *          type <TAB> id <TAB> name <TAB> path <TAB> cover
 *
 *      type  = wii | gc | vc | chan
 *      path  = the title's path on the drive, exactly as RVLoader knows it
 *      cover = cover image path on the drive (may be empty or .../dummy.png)
 *
 *      The first line is a header: "#RVLOADER-LIBRARY 1".
 *
 *   2. Launch arguments, when Wiired starts RVLoader's boot.dol:
 *
 *          wiired boot <type> <path>   boot that title with its saved settings
 *          wiired ui [view]            open RVLoader's own interface (optionally on
 *                                      wii | gc | vc | chan | hb | settings)
 *          wiired refresh              rescan the drive, rewrite the library and
 *                                      go straight back to Wiired
 */

#include <string>

#define WIIRED_DOL_PATH      "/apps/wiired/boot.dol"
#define WIIRED_LIBRARY_PATH  "/rvloader/wiired_library.txt"

//Config key in /rvloader/config.cfg: 0 = start in RVLoader, 1 = start in Wiired
#define STARTUP_MENU_KEY     "StartupMenu"
#define STARTUP_MENU_DEFAULT 0

//Read RVLoader's launch arguments. Call once at the very start of main().
void wiiredParseArgs(int argc, char** argv);

//True if RVLoader was started by Wiired with a request (boot, ui or refresh)
bool wiiredHasRequest();

//True if Wiired's boot.dol is on the drive
bool wiiredInstalled();

//Wait for the cover threads, then write the library file for Wiired
void wiiredWriteLibrary();

//Write the library and start Wiired. Only returns if Wiired couldn't be started.
bool wiiredLaunch();

//Called after the game lists are scanned and the main config is loaded, before the
//theme. Starts Wiired when it should (startup setting or "refresh" request).
//Returns only if RVLoader should carry on normally.
void wiiredStartupHandoff();

//Called after the theme is loaded: carries out "boot" and "ui" requests.
void wiiredHandleRequestAfterTheme();
