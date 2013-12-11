#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "configuration.h"
#include "logging.h"

Configuration *ReadConfiguration(const char *config_path)
{
    Configuration *config = calloc(1,sizeof(Configuration));
    printf("Reading configuration....\n");

    if (config == NULL)
    {
        WriteToLog(1, "Unable to allocate memeory for configuration struct.");
        return NULL;
    }

    FILE *config_file = fopen(config_path, "r+");

    if (config_file == NULL)
    {
        WriteToLog(1, "Unable to open configuration file.");
        return NULL;
    }

    fscanf(config_file, "Resolution: %dx%d\n"
                        "Quality: %d\n"
                        "fps: %d\n"
                        "Rotation: %d\n"
                        "Autostart: %d\n"
                        "DefaultAction: %d\n",
                        &config->res.width, &config->res.height,
                        &config->quality, &config->fps,
                        &config->rotation, &config->autostart,
                        &config->defaultAction);
    fclose(config_file);

    return config;
}

int UpdateConfiguration(const char *config_path, Configuration *config)
{
    if (config == NULL)
    {
        return 1;
    }

    FILE *config_file = fopen(config_path, "w");

    if (config_file == NULL)
    {
        return 1;
    }

    fprintf(config_file, "Resolution: %dx%d\n"
                "Quality: %d\n"
                "fps: %d\n"
                "Rotation: %d\n"
                "Autostart: %d\n"
                "DefaultAction: %d\n",
                config->res.width, config->res.height,
                config->quality, config->fps, config->rotation,
                config->autostart, config->defaultAction);
    fclose(config_file);

    return 0;
}

void FreeConfiguration(Configuration *config)
{
    free(config);
}

void RestoreDefaultConfiguration(void)
{
    Configuration *config = calloc(1, sizeof(Configuration));

    if (config == NULL)
    {
        WriteToLog(1, "Unable to allocate memory for Configuration struct.");
        return;
    }

    config->res.width = 1280;
    config->res.height = 720;
    config->quality = medium;
    config->fps = 25;
    config->autostart = true;
    config->rotation = 270;
    config->defaultAction = video;

    UpdateConfiguration("/etc/goraspberry/config.dat", config);

    FreeConfiguration(config);
}
