#include <stdarg.h>
#include "log.h"
FILE *logfile;
FILE* log_open(void){

    logfile = fopen("filesys.log", "w");
    if (logfile == NULL) 
    {
	perror("logfile");
	exit(EXIT_FAILURE);
    }
    // set logfile to line buffering
    setvbuf(logfile, NULL, _IOLBF, 0);
    
    return logfile;
}

void log_message(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(logfile, format, ap);
}

void log_close()
{
    fclose(logfile);
}