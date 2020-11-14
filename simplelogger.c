#include "./include/simplelogger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define LOG_FILE_PATH "./files/logs/log_%i.txt"
#define MAX_LINE_LENGTH 256

void
SimpleLog(unsigned long ulErrorType, int iLine, const char *szFile, const char *szFunctionName, const char *pszFormat,
          ...) {

    static FILE *fLogfile = NULL;
    static int iLogCounter = 0;

    va_list vaArgumentPointer;
    char szOutputString[MAX_LINE_LENGTH] = {0};
    char *pszType = NULL;
    switch (ulErrorType) {
        case 0:
            pszType = "TRACE";
            break;
        case 1:
            pszType = "DEBUG";
            break;
        case 2:
            pszType = "INFO ";
            break;
        case 3:
            pszType = "WARN ";
            break;
        case 4:
            pszType = "ERROR";
            break;
        default:
            pszType = "DEBUG";
            break;
    }

    // Increment log counter
    iLogCounter++;

    // Create new logfile if no file is open
    if (fLogfile == NULL) {
        char szFileName[256] = {0};
        time_t tTimeAndDate;
        tTimeAndDate = time(NULL);
        snprintf(szFileName, MAX_LINE_LENGTH - 1, LOG_FILE_PATH, (int) tTimeAndDate);
        fLogfile = fopen(szFileName, "w");
        if (fLogfile == NULL) {
            printf("ERROR: Failed to create new logfile\r\n");
        }
    }

    if (fLogfile != NULL) {
        va_start(vaArgumentPointer, pszFormat);
        vsnprintf(szOutputString, MAX_LINE_LENGTH - 1, pszFormat, vaArgumentPointer);
        va_end(vaArgumentPointer);

        // Set LOG_LEVEL in simplelogger.h
        if (ulErrorType >= LOG_LEVEL) {
            // Log to file
            fprintf(fLogfile, "[%04i] - [%s][%s][%s][%d] - %s\r\n", iLogCounter, pszType, szFile, szFunctionName, iLine,
                    szOutputString);
            fflush(fLogfile);

            // Log to stdout
            fprintf(stdout, "[%04i] - [%s][%s][%s][%d] - %s\r\n", iLogCounter, pszType, szFile, szFunctionName,
                    iLine,
                    szOutputString);
            fflush(stdout);
        }
    }
}