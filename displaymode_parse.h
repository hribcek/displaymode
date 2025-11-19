#include <stdbool.h>
#ifndef DISPLAYMODE_PARSE_H
#define DISPLAYMODE_PARSE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// States for the main invocation "option".
// The enum value of the alphabetical options matches the letter that should
// be used on the command line.
// Option values for command-line parsing
enum Option {
    kOptionMissing = 0,
    kOptionInvalid = 1,
    kOptionInvalidMode = 2,
    kOptionSupportedModes = 'd',
    kOptionHelp = 'h',
    kOptionConfigureMode = 't',
    kOptionVersion = 'v',
    kOptionLongHelp,      // --help
    kOptionLongVersion,   // --version
    kOptionLongVerbose,   // --verbose
};

// Positions in argv of various expected parameters.
enum {
    kArgvOptionIndex = 1,
    kArgvWidthIndex = 2,
    kArgvHeightIndex = 3,
    kArgvRefreshOrDisplayIndex = 4,
};

// Parsed command-line arguments
struct ParsedArgs {
    enum Option option;
    const char * literal_option;
    unsigned long width;
    unsigned long height;
    double refresh_rate;  // 0.0 for any
    uint32_t display_index;
    int verbose; // 0 = false, 1 = true
};

// Returns non-zero if "actual" is acceptable for the given specification.
int MatchesRefreshRate(double specified, double actual);

// Parses the command-line arguments and returns them.
// Handles both legacy single-letter options and long flags (e.g., --help).
struct ParsedArgs ParseArgs(int argc, const char * argv[]);

#ifdef __cplusplus
}
#endif

#endif // DISPLAYMODE_PARSE_H
