#include "dssembly.h"

bool argHelp = false;
bool argRaw  = false;
bool argKeep = false;
bool argObj  = false;

char *argRawInputFile  = NULL;
char *argRawOutputFile = NULL;
char *argConfigFile    = NULL;

static void printHelp()
{
    printf("Usage: dssembly -r inputFile.dsb outputFile.bin\n");
    printf("       dssembly configFile.dfg\n");
    printf("\n");
    printf("Options:\n");
    printf("    -r      Raw mode, assembles a single file into binary without the dinker,\n");
    printf("            else run in linked mode\n");
    printf("    -k      Keep the temporary directory\n");
    printf("    -o      Stop at object files, do not link. Output in temporary directory. -k\n");
    printf("            option is implied\n");
    printf("    -h      Display this help and exit\n");
    printf("\n");
    printf("For more information, see <https://github.com/LexiRomano/Dssembly>\n");
}

static bool createTmpDirectory()
{
    DIR *dirp;

    dirp = opendir(TEMP_DIR_NAME);

    if (NULL == dirp)
    {
        if (0 == mkdir(TEMP_DIR_NAME,
                       (S_IRWXU | S_IRWXG | S_IROTH))) // 774 permission
        {
            return true;
        }

        return false;
    }

    closedir(dirp);

    return true;
}

static void removeTmpDirectory()
{
    char   nameBuf[512] = {0};
    struct dirent *file = NULL;

    DIR *dirp;

    dirp = opendir(TEMP_DIR_NAME);

    if (NULL == dirp)
    {
        return;
    }

    for (file = readdir(dirp); file != NULL; file = readdir(dirp))
    {
        if (0 == strcmp(file->d_name, ".") ||
            0 == strcmp(file->d_name, ".."))
        {
            continue;
        }

        snprintf(nameBuf, 512, "%s/%s", TEMP_DIR_NAME, file->d_name);

        remove(nameBuf);
    }

    remove(TEMP_DIR_NAME);
}

static bool parseArgs(int argc, char *argv[])
{
    uint8_t fileCount = 0;

    if (1 == argc)
    {
        argHelp = true;
        return true;
    }

    for (uint8_t i = 1; i < argc; i++)
    {
        if ('-' == argv[i][0])
        {
            for (uint8_t j = 1; argv[i][j] != '\0'; j++)
            {
                switch (argv[i][j])
                {
                    case 'r':
                    {
                        argRaw = true;
                        break;
                    }
                    case 'h':
                    {
                        argHelp = true;
                        break;
                    }
                    case 'k':
                    {
                        argKeep = true;
                        break;
                    }
                    case 'o':
                    {
                        argObj  = true;
                        argKeep = true;
                        break;
                    }
                }
            }

            if (argHelp)
            {
                return true;
            }

            continue;
        }

        if (0 == fileCount)
        {
            argRawInputFile  = argv[i];
        }
        else if (1 == fileCount)
        {
            argRawOutputFile = argv[i];
        }

        fileCount++;
    }

    if (argRaw)
    {
        if (fileCount > 2)
        {
            printf("Too many arguments for raw mode\n");
            return false;
        }
        if (fileCount < 2)
        {
            printf("Too few arguments for raw mode\n");
            return false;
        }
    }
    else
    {
        if (fileCount > 1)
        {
            printf("Too many arguments for linked mode\n");
            return false;
        }
        if (fileCount < 0)
        {
            printf("Too few arguments for linked mode\n");
            return false;
        }

        argConfigFile = argRawInputFile;
        argRawInputFile = NULL;
    }

    return true;
}

static char *convertToObjectName(char *sourceName, uint32_t lineNumber)
{
    uint32_t rawNameStart = 0;
    uint32_t rawNameStop  = 0;
    bool     stopFound    = false;
    char     rawName[256] = {0};
    char     buf[512]     = {0};

    if (NULL == sourceName)
    {
        printf("%s:%u unknown error\n", argConfigFile, lineNumber);
        return NULL;
    }

    for (uint32_t i = strlen(sourceName) - 1; i != 0; i--)
    {
        if ('/' == sourceName[i])
        {
            rawNameStart = i + 1;
            break;
        }

        if (false == stopFound &&
            '.'   == sourceName[i])
        {
            rawNameStop = i;
            stopFound = true;
        }
    }

    if (false == stopFound)
    {
        rawNameStop = strlen(sourceName);
    }

    if (rawNameStart == rawNameStop)
    {
        printf("%s:%u invalid file name\n", argConfigFile, lineNumber);
        return NULL;
    }

    snprintf(rawName, rawNameStop - rawNameStart + 1, "%s", &(sourceName[rawNameStart]));

    snprintf(buf, 512, "%s/%s%s", TEMP_DIR_NAME, rawName, DINKER_OBJECT_EXT);

    return strcpy(calloc(strlen(buf) + 1, sizeof(char)), buf);
}

static bool parseConfigFile(dinkerConfig_t **config)
{
    FILE           *configFile   = NULL;
    char            buf[1024]    = {0};
    dinkerConfig_t *newConfig    = NULL;
    bool            hasOutput    = false;
    tokens_t        tokens       = {0};
    uint32_t        lineNumber   = 0;
    uint8_t         sourceIndex  = 0;
    uint8_t         sectionIndex = 0;

    if (NULL == argConfigFile ||
        NULL == config)
    {
        INTERNAL_ERROR;
        return false;
    }

    configFile = fopen(argConfigFile, "r");

    if (NULL == configFile)
    {
        printf("Could not open dinker config file \"%s\"\n", argConfigFile);
        return false;
    }

    newConfig = calloc(1, sizeof(*newConfig));

    // First pass, get counts of everything and throw errors
    while (fgets(buf, 1024, configFile))
    {
        lineNumber++;
        if (false == parseTokens(buf, &tokens))
        {
            INTERNAL_ERROR;
            free(newConfig);
            fclose(configFile);
        }

        if (0 == tokens.tokenCount)
        {
            continue;
        }

        if (0 == strncmp(tokens.tokens[0], DINKER_CONFIG_SOURCE, strlen(DINKER_CONFIG_SOURCE)) ||
            0 == strncmp(tokens.tokens[0], DINKER_CONFIG_OBJECT, strlen(DINKER_CONFIG_OBJECT)))
        {
            if (2 != tokens.tokenCount)
            {
                printf("%s:%u: source/object requires one file path argument\n", argConfigFile, lineNumber);
                free(newConfig);
                freeTokensContents(&tokens);
                fclose(configFile);
                return false;
            }
            newConfig->inputFileCount++;
        }
        else if (0 == strncmp(buf, DINKER_CONFIG_OUT, strlen(DINKER_CONFIG_OUT)))
        {
            if (2 != tokens.tokenCount)
            {
                printf("%s:%u: out requires one file path argument\n", argConfigFile, lineNumber);
                return false;
            }

            if (hasOutput)
            {
                printf("%s:%u: only one output path is allowed in dinker config\n",
                       argConfigFile, lineNumber);
                free(newConfig);
                freeTokensContents(&tokens);
                fclose(configFile);
                return false;
            }

            hasOutput = true;
        }
        else
        {
            if (1 != tokens.tokenCount)
            {
                printf("%s:%u: only one section per line allowed\n", argConfigFile, lineNumber);
                free(newConfig);
                freeTokensContents(&tokens);
                fclose(configFile);
                return false;
            }

            newConfig->sectionCount++;
        }
    }

    freeTokensContents(&tokens);

    if (0 == newConfig->inputFileCount)
    {
        printf("%s: at least one source file required\n", argConfigFile);
        free(newConfig);
        fclose(configFile);
        return false;
    }

    if (0 == newConfig->sectionCount)
    {
        printf("%s: at least one section required\n", argConfigFile);
        free(newConfig);
        fclose(configFile);
        return false;
    }

    newConfig->inputFiles = calloc(newConfig->inputFileCount, sizeof(inputFile_t));
    newConfig->sections   = calloc(newConfig->sectionCount,   sizeof(char*));

    if (false == hasOutput)
    {
        newConfig->outputFile = strcpy(calloc(strlen(DINKER_DEFAULT_OUT) + 1, sizeof(char*)),
                                              DINKER_DEFAULT_OUT);
    }

    rewind(configFile);

    // Second pass, parse everything
    while (fgets(buf, 1024, configFile))
    {
        if (false == parseTokens(buf, &tokens))
        {
            INTERNAL_ERROR;
            free(newConfig);
            fclose(configFile);
        }

        if (0 == tokens.tokenCount)
        {
            continue;
        }

        if (0 == strncmp(tokens.tokens[0], DINKER_CONFIG_SOURCE, strlen(DINKER_CONFIG_SOURCE)))
        {
            newConfig->inputFiles[sourceIndex].sourceName = strdup(tokens.tokens[1]);

            newConfig->inputFiles[sourceIndex].objectName =
                    convertToObjectName(newConfig->inputFiles[sourceIndex].sourceName,
                                        lineNumber);

            sourceIndex++;
        }
        else if (0 == strncmp(tokens.tokens[0], DINKER_CONFIG_OBJECT, strlen(DINKER_CONFIG_OBJECT)))
        {
            newConfig->inputFiles[sourceIndex].objectName = strdup(tokens.tokens[1]);

            sourceIndex++;
        }
        else if (0 == strncmp(buf, DINKER_CONFIG_OUT, strlen(DINKER_CONFIG_OUT)))
        {
            newConfig->outputFile = strcpy(calloc(strlen(tokens.tokens[1]) + 1, sizeof(char*)),
                                           tokens.tokens[1]);
        }
        else
        {
            newConfig->sections[sectionIndex++] = strcpy(calloc(strlen(tokens.tokens[0]) + 1, sizeof(char*)),
                                                         tokens.tokens[0]);
        }
    }

    freeTokensContents(&tokens);

    fclose(configFile);
    *config = newConfig;
    return true;
}

int main(int argc, char *argv[])
{
    dinkerConfig_t *config  = NULL;
    bool            success = false;

    if (false == parseArgs(argc, argv))
    {
        return -1;
    }

    if (argHelp)
    {
        printHelp();
        return 0;
    }

    if (argRaw)
    {
        return dssembler(argRawInputFile, argRawOutputFile, false) ? 0 : 1;
    }

    if (false == parseConfigFile(&config) ||
        NULL  == config)
    {
        return false;
    }

    if (false == createTmpDirectory())
    {
        printf("Failed to create temporary directories\n");
        freeDinkerConfigContents(config);
        free(config);
        return -1;
    }

    success = true;
    for (uint8_t i = 0; i < config->inputFileCount; i++)
    {
        // Don't dssemble object files
        if (NULL != config->inputFiles[i].sourceName)
        {
            success &= dssembler(config->inputFiles[i].sourceName,
                                 config->inputFiles[i].objectName,
                                 true);
        }
    }

    if (false == success)
    {
        if (false == argKeep)
        {
            removeTmpDirectory();
        }

        freeDinkerConfigContents(config);
        free(config);
        return -1;
    }

    if (false == argObj)
    {
        success = dinker(config);
    }

    if (false == argKeep)
    {
        removeTmpDirectory();
    }

    freeDinkerConfigContents(config);
    free(config);
    return success ? 0 : -1;
}
