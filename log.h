#ifndef _LOG_FILE_H
#define _LOG_FILE_H
#include <stdio.h>
#include <stdlib.h>


FILE* log_open(void);
void log_message(const char* format,...);
void log_close();
#endif

