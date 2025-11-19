#ifndef DISPLAYMODE_FORMAT_H
#define DISPLAYMODE_FORMAT_H
#include <stddef.h>

struct DisplayModeInfo {
    size_t width;
    size_t height;
    double refresh_rate;
    int aspect_w;
    int aspect_h;
    char pixelEncodingStr[64];
    int mode_id;
    int isHiDPI;
    char displayName[128];
    char resCategory[16];
    int usable_for_desktop;
};

void FormatDisplayModeInfo(const struct DisplayModeInfo *info, char *out, size_t out_size);

#endif // DISPLAYMODE_FORMAT_H
