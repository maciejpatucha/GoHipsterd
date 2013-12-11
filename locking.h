#ifndef LOCKING_H
#define LOCKING_H

#include <stdbool.h>

void RecordingLock(void);
void ConversionLock(void);
void RecordingUnlock(void);
void ConversionUnlock(void);
bool TryRecordingLock(void);
bool TryConversionLock(void);

#endif // LOCKING_H
