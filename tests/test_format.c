#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../displaymode_format.h"

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT(expr, msg) do { \
    tests_run++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL: %s (test %d)\n", msg, tests_run); \
        tests_failed++; \
    } \
} while (0)

static void test_format_standard_mode(void) {
    struct DisplayModeInfo info = {
        .width = 1920,
        .height = 1080,
        .refresh_rate = 60.0,
        .aspect_w = 16,
        .aspect_h = 9,
        .pixelEncodingStr = "RGB",
        .mode_id = 42,
        .isHiDPI = 0,
        .displayName = "Display",
        .resCategory = "Standard",
        .usable_for_desktop = 1
    };
    char out[256];
    FormatDisplayModeInfo(&info, out, sizeof(out));
    ASSERT(strstr(out, "1920 x 1080") != NULL, "Resolution present");
    ASSERT(strstr(out, "AR:16:9") != NULL, "Aspect ratio present");
    ASSERT(strstr(out, "Enc:RGB") != NULL, "Pixel encoding present");
    ASSERT(strstr(out, "ModeID:42") != NULL, "Mode ID present");
    ASSERT(strstr(out, "Cat:Standard") != NULL, "Category present");
}

static void test_format_hidpi_mode(void) {
    struct DisplayModeInfo info = {
        .width = 2560,
        .height = 1440,
        .refresh_rate = 60.0,
        .aspect_w = 16,
        .aspect_h = 9,
        .pixelEncodingStr = "ARGB",
        .mode_id = 99,
        .isHiDPI = 1,
        .displayName = "Display",
        .resCategory = "HiDPI",
        .usable_for_desktop = 1
    };
    char out[256];
    FormatDisplayModeInfo(&info, out, sizeof(out));
    ASSERT(strstr(out, "HiDPI") != NULL, "HiDPI info present");
    ASSERT(strstr(out, "Cat:HiDPI") != NULL, "Category present");
}

static void test_format_lowres_mode(void) {
    struct DisplayModeInfo info = {
        .width = 800,
        .height = 600,
        .refresh_rate = 75.0,
        .aspect_w = 4,
        .aspect_h = 3,
        .pixelEncodingStr = "YUV",
        .mode_id = 7,
        .isHiDPI = 0,
        .displayName = "Display",
        .resCategory = "LowRes",
        .usable_for_desktop = 0
    };
    char out[256];
    FormatDisplayModeInfo(&info, out, sizeof(out));
    ASSERT(strstr(out, "800 x 600") != NULL, "Resolution present");
    ASSERT(strstr(out, "AR:4:3") != NULL, "Aspect ratio present");
    ASSERT(strstr(out, "Enc:YUV") != NULL, "Pixel encoding present");
    ASSERT(strstr(out, "ModeID:7") != NULL, "Mode ID present");
    ASSERT(strstr(out, "Cat:LowRes") != NULL, "Category present");
    ASSERT(strstr(out, "!") != NULL, "Not usable for desktop indicated");
}

int main(void) {
    test_format_standard_mode();
    test_format_hidpi_mode();
    test_format_lowres_mode();
    if (tests_failed == 0) {
        printf("All %d format tests passed.\n", tests_run);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "%d of %d format tests failed.\n", tests_failed, tests_run);
        return EXIT_FAILURE;
    }
}
