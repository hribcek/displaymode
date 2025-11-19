#include "displaymode_parse.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    struct ParsedArgs parsed_args;
    parsed_args.option = kOptionMissing;
    parsed_args.literal_option = NULL;
    parsed_args.width = 0;
    parsed_args.height = 0;
    parsed_args.refresh_rate = 0.0;
    parsed_args.display_index = 0;
    parsed_args.verbose = 0;

    if (argc <= 1) {
        return parsed_args;
    }

    // Check for long options (flags) and build filtered positional args
    const char *positional[argc];
    int pos_count = 0;
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == NULL) continue;
        if (strcmp(argv[i], "--help") == 0) {
            parsed_args.option = kOptionLongHelp;
            return parsed_args;
        }
        if (strcmp(argv[i], "--version") == 0) {
            parsed_args.option = kOptionLongVersion;
            return parsed_args;
        }
        if (strcmp(argv[i], "--verbose") == 0) {
            parsed_args.verbose = 1;
            continue;
        }
        positional[pos_count++] = argv[i];
    }

    // Legacy single-letter options
    if (pos_count > kArgvOptionIndex && 1 == (int)strlen(positional[kArgvOptionIndex])) {
        parsed_args.literal_option = positional[kArgvOptionIndex];
        const char option = positional[kArgvOptionIndex][0];
        switch (option) {
            case kOptionSupportedModes:
                parsed_args.option = kOptionSupportedModes;
                break;
            case kOptionHelp:
                parsed_args.option = kOptionHelp;
                break;
            case kOptionConfigureMode:
                parsed_args.option = kOptionConfigureMode;
                break;
            case kOptionVersion:
                parsed_args.option = kOptionVersion;
                break;
            default:
                // Leave parsed_args.option as kOptionMissing (0) to match original behavior.
                break;
        }
        if (option == kOptionConfigureMode) {
            ParseModeInternal(pos_count, positional, &parsed_args);
        }
    }
    return parsed_args;
}
