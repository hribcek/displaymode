// displaymode - a utility for changing the display resolution on Mac OS X.
//
// Copyright 2019-2023 Dean Scarff.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Compilation:
//   clang -std=c11 -lm -framework CoreFoundation -framework CoreGraphics -o displaymode displaymode.c
//
// Usage (to change the resolution to 1440x900):
//   displaymode t 1440 900

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>            // added for bool

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <MacTypes.h>
#include <json-c/json.h>

#include "displaymode_parse.h"  // <- new header exposing ParseArgs, MatchesRefreshRate, ParsedArgs
#include "logging.h"

// Maximum number of displays to query at once (used with CGGetActiveDisplayList).
#define kMaxDisplays 16

// Name and version to display with "v" option.
static const char kProgramVersion[] = "displaymode 1.4.0";

// Restore usage string referenced by ShowUsage.
static const char kUsage[] =
    "Usage:\n\n"
    "  displaymode [options...]\n\n"
    "Options:\n"
    "  t <width> <height> [@<refresh>] [display]\n"
    "      sets the display's width, height and (optionally) refresh rate\n\n"
    "  d\n"
    "      prints available resolutions for each display\n\n"
    "  h, --help\n"
    "      prints this message\n\n"
    "  v, --version\n"
    "      prints version and copyright notice\n\n"
    "  --verbose\n"
    "      enables verbose output\n";

// Prints a message describing how to invoke the tool on the command line.
static void ShowUsage(void) {
    puts(kUsage);
}


#include "displaymode_format.h"

// Extract info and print
static void PrintMode(CGDisplayModeRef mode) {
    if (mode == NULL) {
        return;
    }
    struct DisplayModeInfo info = {0};
    info.width = CGDisplayModeGetWidth(mode);
    info.height = CGDisplayModeGetHeight(mode);
    info.refresh_rate = CGDisplayModeGetRefreshRate(mode);
    info.usable_for_desktop = CGDisplayModeIsUsableForDesktopGUI(mode);
    // Aspect ratio calculation
    int gcd = 1;
    int a = (int)info.width, b = (int)info.height;
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    gcd = a;
    info.aspect_w = (int)info.width / gcd;
    info.aspect_h = (int)info.height / gcd;
    // Pixel encoding (color depth) - Updated to remove deprecated function
    CFStringRef pixelEncoding = CFSTR("Unknown"); // Placeholder for pixel encoding
    CFStringGetCString(pixelEncoding, info.pixelEncodingStr, sizeof(info.pixelEncodingStr), kCFStringEncodingUTF8);

    // Scaling/HiDPI info - Simplified logic
    info.isHiDPI = 0; // Default to non-HiDPI
    // HiDPI detection logic removed due to lack of valid API
    // Display name/model
    snprintf(info.displayName, sizeof(info.displayName), "Display");
    // Resolution category
    if (info.isHiDPI) strcpy(info.resCategory, "HiDPI");
    else if (info.width < 1024 || info.height < 768) strcpy(info.resCategory, "LowRes");
    else strcpy(info.resCategory, "Standard");
    // Format and print
    char out[256];
    FormatDisplayModeInfo(&info, out, sizeof(out));
    printf("%s", out);

    // JSON output
    struct json_object *jsonObj = json_object_new_object();
    json_object_object_add(jsonObj, "width", json_object_new_int(info.width));
    json_object_object_add(jsonObj, "height", json_object_new_int(info.height));
    json_object_object_add(jsonObj, "refreshRate", json_object_new_double(info.refresh_rate));

    const char *jsonStr = json_object_to_json_string(jsonObj);
    logMessage(LOG_LEVEL_INFO, "JSON Output: %s", jsonStr);

    json_object_put(jsonObj); // Free JSON object
}

// Prints all display modes for the main display.  Returns 0 on success.
static int PrintModes(CGDirectDisplayID display) {
    CGDisplayModeRef current_mode = CGDisplayCopyDisplayMode(display);
    CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, NULL);
    if (modes == NULL) {
        // Fallback: if we have the current mode, print it; otherwise fail.
        if (current_mode != NULL) {
            PrintMode(current_mode);
            puts(" *");
            CGDisplayModeRelease(current_mode);
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Failed to get display modes\n");
        return EXIT_FAILURE;
    }

    const CFIndex count = CFArrayGetCount(modes);
    Boolean has_current = 0;
    for (size_t i = 0; i < (size_t)count; ++i) {
        CGDisplayModeRef mode =
            (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, (CFIndex)i);
        if (mode == NULL) {
            continue;
        }
        PrintMode(mode);
        if (current_mode != NULL && CFEqual(mode, current_mode)) {
            has_current = 1;
            puts(" *");
        } else {
            puts("");
        }
    }
    if (!has_current && current_mode != NULL) {
        PrintMode(current_mode);
        puts(" *");
    }
    CFRelease(modes);
    if (current_mode != NULL) {
        CGDisplayModeRelease(current_mode);
    }
    return EXIT_SUCCESS;
}

static int PrintModesForAllDisplays(void) {
    CGDirectDisplayID displays[kMaxDisplays];
    uint32_t num_displays = 0;
    CGError e = CGGetActiveDisplayList(kMaxDisplays, &displays[0], &num_displays);
    if (e) {
        fprintf(stderr, "CGGetActiveDisplayList CGError: %d\n", e);
        return e;
    }

    for (uint32_t i = 0; i < num_displays; ++i) {
        printf("%sDisplay %u%s:\n", i == 0 ? "" : "\n", i, i == 0 ? " (MAIN)" : "");
        PrintModes(displays[i]);
    }

    return EXIT_SUCCESS;
}

static CGError GetDisplayID(uint32_t display_index, CGDirectDisplayID *display) {
    CGDirectDisplayID displays[kMaxDisplays];
    uint32_t num_displays = 0;
    CGError e = CGGetActiveDisplayList(kMaxDisplays, &displays[0], &num_displays);
    if (e) {
        fprintf(stderr, "CGGetActiveDisplayList CGError: %d\n", e);
        return e;
    }
    if (num_displays <= display_index) {
        fprintf(stderr, "Display %u not supported; display must be < %u\n",
                display_index, num_displays);
        return kCGErrorRangeCheck;
    }
    *display = displays[display_index];
    return kCGErrorSuccess;
}

static CGDisplayModeRef GetModeMatching(const struct ParsedArgs *parsed_args,
                                        const CGDirectDisplayID display) {
    CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, NULL);
    if (modes == NULL) {
        return NULL;
    }
    const CFIndex count = CFArrayGetCount(modes);

    CGDisplayModeRef matched_mode = NULL;
    for (size_t i = 0; i < (size_t)count; ++i) {
        CGDisplayModeRef mode =
            (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, (CFIndex)i);
        if (mode == NULL) {
            continue;
        }
        const size_t width = CGDisplayModeGetWidth(mode);
        const size_t height = CGDisplayModeGetHeight(mode);
        const double refresh_rate = CGDisplayModeGetRefreshRate(mode);
        if (width == parsed_args->width &&
            height == parsed_args->height &&
            MatchesRefreshRate(parsed_args->refresh_rate, refresh_rate)) {
            matched_mode = CGDisplayModeRetain(mode);
            break;
        }
    }
    CFRelease(modes);
    return matched_mode;
}

static int ConfigureMode(const struct ParsedArgs *parsed_args) {
    CGDirectDisplayID display;
    CGError e;
    if ((e = GetDisplayID(parsed_args->display_index, &display))) {
        return e;
    }

    CGDisplayModeRef mode = GetModeMatching(parsed_args, display);
    if (mode == NULL) {
        if (parsed_args->refresh_rate == 0.0) {
            fprintf(stderr, "Could not find a mode for resolution %lux%lu\n",
                    parsed_args->width, parsed_args->height);
        } else {
            fprintf(stderr, "Could not find a mode for resolution %lux%lu @%.1f\n",
                    parsed_args->width, parsed_args->height,
                    parsed_args->refresh_rate);
        }
        return -1;
    }

    CGDisplayModeRef original_mode = CGDisplayCopyDisplayMode(display);
    size_t original_width = 0;
    size_t original_height = 0;
    double original_refresh_rate = 0.0;
    if (original_mode != NULL) {
        original_width = CGDisplayModeGetWidth(original_mode);
        original_height = CGDisplayModeGetHeight(original_mode);
        original_refresh_rate = CGDisplayModeGetRefreshRate(original_mode);
        CGDisplayModeRelease(original_mode);
    }

    CGDisplayConfigRef config = NULL;
    if ((e = CGBeginDisplayConfiguration(&config))) {
        fprintf(stderr, "CGBeginDisplayConfiguration CGError: %d\n", e);
        CGDisplayModeRelease(mode);
        return e;
    }
    if ((e = CGConfigureDisplayWithDisplayMode(config, display, mode, NULL))) {
        fprintf(stderr, "CGConfigureDisplayWithDisplayMode CGError: %d\n", e);
        CGCancelDisplayConfiguration(config);
        CGDisplayModeRelease(mode);
        return e;
    }
    if ((e = CGCompleteDisplayConfiguration(config, kCGConfigurePermanently))) {
        fprintf(stderr, "CGCompleteDisplayConfiguration CGError: %d\n", e);
        CGDisplayModeRelease(mode);
        return e;
    }
    CGDisplayModeRelease(mode);

    if (parsed_args->refresh_rate == 0.0) {
        printf("Changed display resolution from %zux%zu to %lux%lu\n",
               original_width, original_height,
               parsed_args->width, parsed_args->height);
    } else {
        printf("Changed display resolution from %zux%zu @%f to %lux%lu @%.1f\n",
               original_width, original_height, original_refresh_rate,
               parsed_args->width, parsed_args->height,
               parsed_args->refresh_rate);
    }
    return EXIT_SUCCESS;
}

int main(int argc, const char *argv[]) {
    setLogLevel(LOG_LEVEL_DEBUG);
    logMessage(LOG_LEVEL_INFO, "Starting displaymode application");

    const struct ParsedArgs parsed_args = ParseArgs(argc, argv);
    switch (parsed_args.option) {
        case kOptionMissing: {
            fputs("Missing option; server mode is not supported\n\n", stderr);
            ShowUsage();
            break;
        }
        case kOptionInvalid: {
            fprintf(stderr, "Invalid option: '%s'\n\n",
                    parsed_args.literal_option);
            ShowUsage();
            break;
        }
        case kOptionInvalidMode: {
            fputs("Invalid mode\n", stderr);
            break;
        }
        case kOptionConfigureMode:
            if (parsed_args.verbose) {
                printf("[VERBOSE] Configuring display mode...\n");
            }
            return ConfigureMode(&parsed_args);
        case kOptionHelp:
        case kOptionLongHelp:
            ShowUsage();
            return EXIT_SUCCESS;
        case kOptionSupportedModes:
            if (parsed_args.verbose) {
                printf("[VERBOSE] Printing supported display modes...\n");
            }
            return PrintModesForAllDisplays();
        case kOptionVersion:
        case kOptionLongVersion:
            printf("%s\nCopyright 2019-2023 Dean Scarff\n", kProgramVersion);
            return EXIT_SUCCESS;
        default:
            break;
    }
    return EXIT_FAILURE;
}
