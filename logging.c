#include "logging.h"
#include <stdarg.h>
#include <time.h>

static LogLevel currentLogLevel = LOG_LEVEL_INFO;

void setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

void logMessage(LogLevel level, const char *format, ...) {
    if (level < currentLogLevel) {
        return;
    }

    const char *levelStr;
    switch (level) {
        case LOG_LEVEL_DEBUG: levelStr = "DEBUG"; break;
        case LOG_LEVEL_INFO:  levelStr = "INFO";  break;
        case LOG_LEVEL_WARN:  levelStr = "WARN";  break;
        case LOG_LEVEL_ERROR: levelStr = "ERROR"; break;
        default:              levelStr = "UNKNOWN"; break;
    }

    time_t now = time(NULL);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(stderr, "%s [%s] ", timeStr, levelStr);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}