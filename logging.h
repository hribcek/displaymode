#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

// Set the current log level
void setLogLevel(LogLevel level);

// Log a message
void logMessage(LogLevel level, const char *format, ...);

#endif // LOGGING_H