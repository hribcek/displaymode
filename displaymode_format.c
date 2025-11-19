#include "displaymode_format.h"
#include <stdio.h>
#include <string.h>

void FormatDisplayModeInfo(const struct DisplayModeInfo *info, char *out, size_t out_size) {
    snprintf(out, out_size,
        "%zu x %zu @%.1fHz AR:%d:%d Enc:%s ModeID:%d %s %s Cat:%s%s",
        info->width, info->height, info->refresh_rate,
        info->aspect_w, info->aspect_h,
        info->pixelEncodingStr,
        info->mode_id,
        info->isHiDPI ? "HiDPI" : "Std",
        info->displayName,
        info->resCategory,
        info->usable_for_desktop ? "" : " !");
}
