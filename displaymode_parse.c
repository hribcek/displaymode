#include "displaymode_parse.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Returns non-zero if "actual" is acceptable for the given specification.
int MatchesRefreshRate(double specified, double actual) {
    static const double kRefreshTolerance = 0.005;
    return specified == 0.0 || fabs(specified - actual) < kRefreshTolerance;
}

// Parses the "width height [display]" mode specification.
static void ParseModeInternal(const int argc, const char * argv[],
                              struct ParsedArgs * parsed_args) {
    if (argc <= kArgvHeightIndex) {
        parsed_args->option = kOptionInvalidMode;
        return;
    }

    // Parse width.
    errno = 0;
    char *endptr = NULL;
    const unsigned long width =
        strtoul(argv[kArgvWidthIndex], &endptr, 10);
    if (endptr == argv[kArgvWidthIndex] || errno != 0) {
        parsed_args->option = kOptionInvalidMode;
        errno = 0;
        return;
    }

    // Parse height.
    errno = 0;
    endptr = NULL;
    const unsigned long height =
        strtoul(argv[kArgvHeightIndex], &endptr, 10);
    if (endptr == argv[kArgvHeightIndex] || errno != 0) {
        parsed_args->option = kOptionInvalidMode;
        errno = 0;
        return;
    }

    size_t next_index = kArgvRefreshOrDisplayIndex;
    parsed_args->refresh_rate = 0.0;

    // Optional refresh rate in form "@<value>"
    if (next_index < (size_t)argc && argv[next_index][0] == '@') {
        const char *s = argv[next_index] + 1;
        char *end = NULL;
        errno = 0;
        const double refresh = strtod(s, &end);
        if (end == s || errno != 0 || refresh < 0.0) {
            parsed_args->option = kOptionInvalidMode;
            errno = 0;
            return;
        }
        parsed_args->refresh_rate = refresh;
        ++next_index;
    }

    // Optional display index
    if (next_index < (size_t)argc) {
        errno = 0;
        char *end = NULL;
        unsigned long di = strtoul(argv[next_index], &end, 10);
        if (end == argv[next_index] || errno != 0 || di > UINT32_MAX) {
            parsed_args->option = kOptionInvalidMode;
            errno = 0;
            return;
        }
        parsed_args->display_index = (uint32_t)di;
    } else {
        parsed_args->display_index = 0;
    }

    if (width == 0 || height == 0) {
        parsed_args->option = kOptionInvalidMode;
        return;
    }

    parsed_args->width = width;
    parsed_args->height = height;
}

// Parses the command-line arguments and returns them.
struct ParsedArgs ParseArgs(int argc, const char * argv[]) {
    struct ParsedArgs parsed_args = { 0 };

    if (argc <= 1) {
        return parsed_args;
    }
    if (1 != (int)strlen(argv[kArgvOptionIndex])) {
        return parsed_args;
    }

    parsed_args.literal_option = argv[kArgvOptionIndex];
    const char option = argv[kArgvOptionIndex][0];
    switch (option) {
        case kOptionSupportedModes:
        case kOptionHelp:
        case kOptionConfigureMode:
        case kOptionVersion:
            parsed_args.option = (enum Option)option;
            break;
        default:
            // Leave parsed_args.option as kOptionMissing (0) to match original behavior.
            break;
    }

    if (option == kOptionConfigureMode) {
        ParseModeInternal(argc, argv, &parsed_args);
    }
    return parsed_args;
}
