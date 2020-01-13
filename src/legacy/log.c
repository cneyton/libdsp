/**
 * @file log.c
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include "log.h"

#define RESET     "\e[0m"
#define BLACK     "\e[30m"
#define RED       "\e[31m"
#define GREEN     "\e[32m"
#define YELLOW    "\e[33m"
#define BLUE      "\e[34m"
#define PURPLE    "\e[35m"
#define CYAN      "\e[36m"
#define WHITE     "\e[37m"

#define BOLD      "\e[1m"


/**
 * @brief Print colored logs on stdout.
 */
void _log_add(const char *c_filename, const char *function_name, int line_number, const char *format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf(""BOLD"%s:%d:%s"RESET" %s\n", c_filename, line_number, function_name, buffer);

    return;
}

