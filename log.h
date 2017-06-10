#ifndef LOG_H
#define LOG_H

#include "includes.h"
#include "conf.h"

PEWAPI void log_Init();

PEWAPI void log_finish();

PEWAPI void log_LogMessage(char *format, ...);


#endif /* LOG_H */
