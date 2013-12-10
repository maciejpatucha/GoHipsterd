#include "common_headers.h"

static bool IsRawVideoFile(char *file);

char *GetFileToConvert(void)
{
    DIR *dir = opendir("/home/pi/Recordings");

    if (dir == NULL)
    {
        WriteToLog(1, "Cannot open directory");
        return NULL;
    }

    struct dirent *currdir;

    while ((currdir = readdir(dir)) != NULL)
    {
        if ((currdir->d_type == DT_DIR) || (!strncmp(currdir->d_name, ".", 1)) || (!strncmp(currdir->d_name, "..", 2)))
        {
            continue;
        }

        if (!IsRawVideoFile(currdir->d_name))
        {
            continue;
        }

        char *fileToConvert = calloc(PATH_MAX, sizeof(char));

        if (fileToConvert == NULL)
        {
            WriteToLog(1, "Cannot allocate memory for fileToConvert");
            closedir(dir);
            return NULL;
        }

        snprintf(fileToConvert, PATH_MAX, "/home/pi/Recordings/%s", currdir->d_name);
        closedir(dir);
        return fileToConvert;
    }
    closedir(dir);

    return NULL;
}

char *ConvertOutputFileName(char *filename)
{
    int length = strlen(filename);

    char *convFileName = calloc(length, sizeof(char));

    if (convFileName == NULL)
    {
        WriteToLog(0, "Unable to allocate memory for convFileName");
        return NULL;
    }


    int amount = strstr(filename, "h264") - filename;

    strncpy(convFileName, filename, amount);
    strncat(convFileName, "mp4", 3);

    return convFileName;
}

static bool IsRawVideoFile(char *file)
{
    if (strstr(file, "h264") == NULL)
    {
        return false;
    }

    return true;
}

