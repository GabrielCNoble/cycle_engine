#include "log.h"

#include <stdio.h>	/* fopen, fclose, etc... */
#include <time.h>	/* for mingw_timeofday, etc... */

char *log_path = "cache\\engine.log";


PEWAPI void log_Init()
{
	
}

PEWAPI void log_finish()
{
		
}


PEWAPI void log_LogMessage(char *format, ...)
{
	FILE *log = fopen(log_path, "a+");
	char log_msg[1024];
	char *date;
	struct tm *p;
	int i = 0;
	time_t t;
	t = time(NULL);
	p = localtime(&t);
	date = asctime(p);
	while(date[i] != '\n' && date[i] != '\0')
	{
		i++;
	}
	date[i] = ' ';
	
	//printf("%s\n", asctime(p));
	sprintf(log_msg, "#################################\n[%s]\n--\t%s\n[END OF LOG]\n#################################\n\n", date, format);
	
	fprintf(log, "%s", log_msg);
	fclose(log);
}











