#include "dssembly.h"

static FILE *inputFile          = NULL;
static char *outputFileLocation = NULL;

static instructionList_t instructionList = {0};

static bool parseArgs(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Insufficient arguments!\n");
        return false;
    }

    inputFile = fopen(argv[1], "r");

    if (NULL == inputFile)
    {
        printf("Failed to open input file \"%s\"\n", argv[1]);
        return false;
    }

    if (argc >= 3)
    {
        outputFileLocation = argv[2];
    }

    return true;
}

static bool getOpCode(char *strIn, uint8_t *codeOut)
{
    if (0 == strcmp(strIn, "ABC"))
    {
        *codeOut = 0xFF;
        return true;
    }

    return false;
}

static bool parseInputFile()
{
    char           inputBuffer[2048]  = {0};
    instruction_t *currentInstruction = NULL;
    tokens_t       tokens             = {0};
    uint32_t       lineNumber         = 0;

    while (fgets(inputBuffer, sizeof(inputBuffer), inputFile))
    {
        lineNumber++;

        if (false == parseTokens(inputBuffer, &tokens))
        {
            printf("Internal error\n");
            freeInstructionListContents(&instructionList);
            return false;
        }

        if (NULL == tokens.tokens[0] ||
            0    == strncmp(tokens.tokens[0], COMMENT_PREFIX, strlen(COMMENT_PREFIX)))
        {
            continue;
        }

        currentInstruction = addNewInstruction(&instructionList);

        if (NULL == currentInstruction)
        {
            printf("Internal error\n");
            freeTokensContents(&tokens);
            freeInstructionListContents(&instructionList);
            return false;
        }

        if (false == getOpCode(tokens.tokens[0], &(currentInstruction->opCode)))
        {
            printf("Error on line %u: Invalid instruction \"%s\"\n", lineNumber, tokens.tokens[0]);
            freeTokensContents(&tokens);
            freeInstructionListContents(&instructionList);
            return false;
        }

    }

    freeTokensContents(&tokens);
    return true;
}

int main(int argc, char* argv[])
{
    printf("Dssembly\n");

    if (false == parseArgs(argc, argv))
    {
        return -1;
    }

    if (false == parseInputFile())
    {
        return -1;
    }

    freeInstructionListContents(&instructionList);

    printf("Success!\n");

    return 0;
}