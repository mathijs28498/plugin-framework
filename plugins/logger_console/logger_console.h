#pragma once

#define LOGGER_CONSOLE_LOG_LEVEL_MAX 4

struct LoggerContext;
enum LoggerInterfaceLogLevel;

void logger_console_log(const struct LoggerContext *context, enum LoggerInterfaceLogLevel log_level, enum LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *message, ...);
void logger_console_set_level(struct LoggerContext *context, enum LoggerInterfaceLogLevel log_level);
void logger_console_set_colors(struct LoggerContext *context, const char *new_colors[LOGGER_CONSOLE_LOG_LEVEL_MAX]);
void logger_console_on_program_exit(struct LoggerContext *context, int exit_code);