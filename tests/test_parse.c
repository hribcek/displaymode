#include "../displaymode_parse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT(expr, msg) do { \
    tests_run++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL: %s (test %d)\n", msg, tests_run); \
        tests_failed++; \
    } \
} while (0)

static void test_matches_refresh_rate(void) {
    ASSERT(MatchesRefreshRate(0.0, 30.0), "specified 0.0 accepts any rate");
    ASSERT(MatchesRefreshRate(60.0, 60.0), "exact match");
    ASSERT(MatchesRefreshRate(60.0, 60.003), "within tolerance");
    ASSERT(!MatchesRefreshRate(60.0, 59.9), "outside tolerance");
}

static void test_parse_args_simple(void) {
    const char *argv1[] = { "prog", "t", "1440", "900", NULL };
    struct ParsedArgs p1 = ParseArgs(4, argv1);
    ASSERT(p1.option == kOptionConfigureMode, "option == t");
    ASSERT(p1.width == 1440UL, "width parsed");
    ASSERT(p1.height == 900UL, "height parsed");
    ASSERT(p1.refresh_rate == 0.0, "no refresh parsed -> 0.0");
    ASSERT(p1.display_index == 0U, "default display index 0");
}

static void test_parse_args_with_refresh_and_display(void) {
    const char *argv2[] = { "prog", "t", "800", "600", "@75.0", "2", NULL };
    struct ParsedArgs p2 = ParseArgs(6, argv2);
    ASSERT(p2.option == kOptionConfigureMode, "option == t (with refresh)");
    ASSERT(p2.width == 800UL, "width parsed 800");
    ASSERT(p2.height == 600UL, "height parsed 600");
    ASSERT(p2.refresh_rate > 74.99 && p2.refresh_rate < 75.01, "refresh 75");
    ASSERT(p2.display_index == 2U, "display index parsed 2");
}

static void test_parse_args_invalid_mode(void) {
    const char *argv3[] = { "prog", "t", "x", "600", NULL };
    struct ParsedArgs p3 = ParseArgs(4, argv3);
    ASSERT(p3.option == kOptionInvalidMode, "invalid width -> invalid mode");
}

static void test_parse_args_help_flag(void) {
    const char *argv[] = { "prog", "--help", NULL };
    struct ParsedArgs p = ParseArgs(2, argv);
    ASSERT(p.option == kOptionHelp || p.option == kOptionLongHelp, "--help flag parsed as help option");
}

static void test_parse_args_version_flag(void) {
    const char *argv[] = { "prog", "--version", NULL };
    struct ParsedArgs p = ParseArgs(2, argv);
    ASSERT(p.option == kOptionVersion || p.option == kOptionLongVersion, "--version flag parsed as version option");
}

static void test_parse_args_verbose_flag(void) {
    const char *argv[] = { "prog", "t", "1024", "768", "--verbose", NULL };
    struct ParsedArgs p = ParseArgs(6, argv);
    ASSERT(p.option == kOptionConfigureMode, "option == t with --verbose");
    ASSERT(p.width == 1024UL, "width parsed 1024");
    ASSERT(p.height == 768UL, "height parsed 768");
}

static void test_parse_args_missing_args(void) {
    const char *argv[] = { "prog", "t", NULL };
    struct ParsedArgs p = ParseArgs(2, argv);
    ASSERT(p.option == kOptionInvalidMode, "missing width/height -> invalid mode");
}

int main(void) {
    test_matches_refresh_rate();
    test_parse_args_simple();
    test_parse_args_with_refresh_and_display();
    test_parse_args_invalid_mode();
    test_parse_args_help_flag();
    test_parse_args_version_flag();
    test_parse_args_verbose_flag();
    test_parse_args_missing_args();

    if (tests_failed == 0) {
        printf("All %d tests passed.\n", tests_run);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "%d of %d tests failed.\n", tests_failed, tests_run);
        return EXIT_FAILURE;
    }
}
