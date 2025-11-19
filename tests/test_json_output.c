#include "displaymode_format.h" // Replace invalid header
#include <json-c/json.h> // Ensure JSON library is included
#include "logging.h"
#include <assert.h>

void test_json_output() {
    struct DisplayModeInfo mode = {
        .width = 1920,
        .height = 1080,
        .refresh_rate = 60.0,
        .aspect_w = 0,
        .aspect_h = 0,
        .isHiDPI = 0,
        .pixelEncodingStr = "",
        .displayName = "",
        .resCategory = "",
        .mode_id = 0
    };
    struct json_object *jsonObj = json_object_new_object();
    json_object_object_add(jsonObj, "width", json_object_new_int(mode.width));
    json_object_object_add(jsonObj, "height", json_object_new_int(mode.height));
    json_object_object_add(jsonObj, "refreshRate", json_object_new_double(mode.refresh_rate)); // Corrected field name

    const char *jsonStr = json_object_to_json_string(jsonObj);
    assert(jsonStr != NULL);
    logMessage(LOG_LEVEL_INFO, "Test JSON Output: %s", jsonStr);

    json_object_put(jsonObj);
}

int main() {
    setLogLevel(LOG_LEVEL_DEBUG);
    test_json_output();
    return 0;
}