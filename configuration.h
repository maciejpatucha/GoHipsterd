#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdbool.h>

typedef struct
{
    int width;
    int height;
} Resolution;

enum Quality
{
    high,
    medium,
    low
};

enum DefaultAction
{
    photo,
    video
};

typedef struct
{
    Resolution res;
    enum Quality quality;
    int fps;
    int rotation;
    bool autostart;
    enum DefaultAction defaultAction;
} Configuration;

Configuration *ReadConfiguration(const char *config_path);
int UpdateConfiguration(const char *config_path, Configuration *config);
void FreeConfiguration(Configuration *config);
void RestoreDefaultConfiguration(void);

#endif // CONFIGURATION_H
