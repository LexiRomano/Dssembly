#include <dssembly.h>

static void printHelp()
{
    printf("Usage: dssembly -r inputFile.dsb outputFile.bin\n");
    printf("       dssembly configFile.dfg\n");
    printf("\n");
    printf("Options:\n");
    printf("    -r      Raw mode, assembles a single file into binary without the dinker\n");
    printf("    -h      Display this help and exit\n");
    printf("\n");
    printf("For more information, see <https://github.com/LexiRomano/Dssembly>\n");
}

static bool shouldPrintHelp(int argc, char *argv[])
{
    if (argc < 2)
    {
        return true;
    }

    if ('-' == argv[1][0])
    {
        for (uint8_t i = 1; argv[1][i] != '\0'; i++)
        {
            if ('h' == argv[1][i])
            {
                return true;
            }
        }
    }

    return false;
}

static bool isRawMode(int argc, char *argv[])
{
    if (argc < 2)
    {
        return false;
    }

    if ('-' == argv[1][0])
    {
        for (uint8_t i = 1; argv[1][i] != '\0'; i++)
        {
            if ('r' == argv[1][i])
            {
                return true;
            }
        }
    }

    return false;
}


int main(int argc, char *argv[])
{
    if (shouldPrintHelp(argc, argv))
    {
        printHelp();
        return 0;
    }

    if (isRawMode(argc, argv))
    {
        if (argc > 4)
        {
            printf("Too many arguments for raw mode\n");
            return -1;
        }
        if (argc < 4)
        {
            printf("Too few arguments for raw mode\n");
            return -1;
        }
        if (false == dssembler(argv[2], argv[3], false))
        {
            return -1;
        }

        return 0;
    }
    
    printf("Linked mode currently unsupported\n");
    return -1;
}