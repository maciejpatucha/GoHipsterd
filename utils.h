#include <linux/limits.h>

int CheckLink(char *interface);
unsigned long GetMaxRecordingTime(void);
int TerminateRecording(void);
char *OutputFileName(void);
char *GetFileToConvert(void);
char *ConvertOutputFileName(char *filename);
