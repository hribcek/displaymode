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

#include "displaymode_parse.h"  // <- new header exposing ParseArgs, MatchesRefreshRate, ParsedArgs

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
    "  h\n"
    "      prints this message\n\n"
    "  v\n"
    "      prints version and copyright notice\n";

// Prints a message describing how to invoke the tool on the command line.
static void ShowUsage(void) {
    puts(kUsage);
}

// Prints the resolution and refresh rate for a display mode.
static void PrintMode(CGDisplayModeRef mode) {
    if (mode == NULL) {
        return;
    }
    const size_t width = CGDisplayModeGetWidth(mode);
    const size_t height = CGDisplayModeGetHeight(mode);
    const double refresh_rate = CGDisplayModeGetRefreshRate(mode);
    const bool usable_for_desktop =
        CGDisplayModeIsUsableForDesktopGUI(mode);
    printf("%zu x %zu @%.1fHz%s", width, height, refresh_rate,
           usable_for_desktop ? "" : " !");
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
    for (CFIndex i = 0; i < count; ++i) {
        CGDisplayModeRef mode =
            (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
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
    uint32_t num_displays;
    CGError e =
        CGGetActiveDisplayList(kMaxDisplays, &displays[0], &num_displays);
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

// Returns the display ID (arbitrary integers) corresponding to the given
// display index (0-indexed).
static CGError GetDisplayID(uint32_t display_index,
                            CGDirectDisplayID * display) {
    CGDirectDisplayID displays[kMaxDisplays];
    uint32_t num_displays;
    CGError e =
        CGGetActiveDisplayList(kMaxDisplays, &displays[0], &num_displays);
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

// Returns the first mode whose resolution matches the width and height
// specified in `parsed_args'.  Returns NULL if no modes matched.
// The caller owns the returned mode.
static CGDisplayModeRef GetModeMatching(const struct ParsedArgs * parsed_args,
                                        const CGDirectDisplayID display) {
    CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, NULL);
    if (modes == NULL) {
        return NULL;
    }
    const CFIndex count = CFArrayGetCount(modes);

    CGDisplayModeRef matched_mode = NULL;
    // Set matched_mode to the first display mode matching the requested
    // resolution.
    for (CFIndex i = 0; i < count; ++i) {
        CGDisplayModeRef const mode =
            (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
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

// Changes the resolution permanently for the user.
static int ConfigureMode(const struct ParsedArgs * parsed_args) {
    CGDirectDisplayID display;
    CGError e;
    if ((e = GetDisplayID(parsed_args->display_index, &display))) {
        return e;
    }

    CGDisplayModeRef mode = GetModeMatching(parsed_args, display);
    if (NULL == mode) {
        if (parsed_args->refresh_rate == 0.0) {
            fprintf(stderr, "Could not find a mode for resolution %lux%lu\n",
                    parsed_args->width, parsed_args->height);
        } else {
            fprintf(stderr, "Could not find a mode for resolution %lux%lu"
                    " @%.1f\n",
                    parsed_args->width, parsed_args->height,
                    parsed_args->refresh_rate);
        }
        return -1;
    }

    // Save the original resolution.
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

    // Change the resolution.
    CGDisplayConfigRef config = NULL;
    if ((e = CGBeginDisplayConfiguration(&config))) {
        fprintf(stderr, "CGBeginDisplayConfiguration CGError: %d\n", e);
        CGDisplayModeRelease(mode);
        return e;
    }
    if ((e = CGConfigureDisplayWithDisplayMode(config, display, mode, NULL))) {
        fprintf(stderr, "CGConfigureDisplayWithDisplayMode CGError: %d\n", e);
        // Best-effort cancel and cleanup.
        CGCancelDisplayConfiguration(config);
        CGDisplayModeRelease(mode);
        return e;
    }
    if ((e = CGCompleteDisplayConfiguration(config, kCGConfigurePermanently))) {
        fprintf(stderr, "CGCompleteDisplayConfiguration CGError: %d\n", e);
        // On failure, we can't be sure the configuration was applied; nothing more to do.
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

int main(int argc, const char * argv[]) {
    const struct ParsedArgs parsed_args = ParseArgs(argc, argv);
    switch (parsed_args.option) {
        case kOptionMissing:
            fputs("Missing option; server mode is not supported\n\n", stderr);
            ShowUsage();
            break;

        case kOptionInvalid:
            fprintf(stderr, "Invalid option: '%s'\n\n",
                    parsed_args.literal_option);
            ShowUsage();
            break;

        case kOptionInvalidMode:
            fputs("Invalid mode\n", stderr);
            break;

        case kOptionConfigureMode:
            return ConfigureMode(&parsed_args);

        case kOptionHelp:
            ShowUsage();
            return EXIT_SUCCESS;

        case kOptionSupportedModes:
            return PrintModesForAllDisplays();

        case kOptionVersion:
            printf("%s\nCopyright 2019-2023 Dean Scarff\n", kProgramVersion);
            return EXIT_SUCCESS;

        default:
            break;
    }
    return EXIT_FAILURE;
}
