#ifndef EXAM_PREP_SIMPLELOGGER_H
#define EXAM_PREP_SIMPLELOGGER_H

#define LOG_LEVEL 2

void SimpleLog(unsigned long ulErrorType, int iLine, const char *szFile, const char *szFunctionName, const char *pszFormat, ...);

#define LogTrace(...) SimpleLog(0, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LogDebug(...) SimpleLog(1, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LogInfo(...) SimpleLog(2, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LogWarn(...) SimpleLog(3, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define LogError(...) SimpleLog(4, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#endif //EXAM_PREP_SIMPLELOGGER_H
