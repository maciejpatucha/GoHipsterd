#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "configuration.h"

Configuration *ReadConfiguration(const char *config_path)
{
    Configuration *config = calloc(1,sizeof(Configuration));
    printf("Reading configuration....\n");

    if (config == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for Configuration structure: %s\n", strerror(errno));
        return NULL;
    }

    FILE *config_file = fopen(config_path, "r+");

    if (config_file == NULL)
    {
        fprintf(stderr, "Failed to open config.dat file: %s\n", strerror(errno));
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
        fprintf(stderr, "No configuration specified!!!\n");
        return 1;
    }

    FILE *config_file = fopen(config_path, "w");

    if (config_file == NULL)
    {
        fprintf(stderr, "Unable to open config.dat to write configuration!!!\n");
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
