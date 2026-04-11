#include "dssembly.h"

static FILE *inputFile          = NULL;
static char *outputFileLocation = NULL;

static instructionList_t instructionList = {0};

static instructionDescriptor_t instructionDescriptors[] =
{
    {"MOVE", 0x06, form_2, true,  0x07, form_4, false, 0, false},
    {"TERM", 0xFF, form_6, false, 0,    0,      false, 0, false}
};

static char regselDescriptors[][3] = 
{
    "G0",
    "G1",
    "G2",
    "G3",
    "G4",
    "G5",
    "G6",
    "G7",
    "OA",
    "OB",
    "SB",
    "SS",
    "IL"
};

static uint8_t formArgCount[] = 
{
    3, // form_1
    2, // form_2
    3, // form_3
    1, // form_4
    2, // form_5
    0, // form_6
    1  // form_7
};

static uint8_t formRegselCount[] = 
{
    3, // form_1
    2, // form_2
    2, // form_3
    1, // form_4
    1, // form_5
    0, // form_6
    0  // form_7
};

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

static uint8_t getRegsel(char *str)
{
    for (uint8_t i = 0; i < 0x10; i++)
    {
        if (0 == strncmp(str, regselDescriptors[i], 3))
        {
            return i;
        }
    }

    return -1;
}

static bool isValidRegsel(char *str)
{
    for (uint8_t i = 0; i < 0x10; i++)
    {
        if (0 == strncmp(str, regselDescriptors[i], 3))
        {
            return true;
        }
    }

    return false;
}

static bool parseFormAndRegsel(tokens_t *tokens, instruction_t *instruction, instructionDescriptor_t *descriptor, uint32_t lineNumber)
{
    uint8_t minimumRegselCount = 0;
    bool    isAlternateForm    = false;

    if (NULL == tokens ||
        NULL == descriptor)
    {
        printf("Internal error\n");
        return false;
    }
    

    if (tokens->tokenCount - 1 != formArgCount[descriptor->primaryForm])
    {
        if (tokens->tokenCount - 1 > formArgCount[descriptor->primaryForm])
        {
            printf("Error on line %u: Too many arguments for instruction \"%s\"\n", lineNumber, tokens->tokens[0]);
        }
        else
        {
            printf("Error on line %u: Too few arguments for instruction \"%s\"\n", lineNumber, tokens->tokens[0]);
        }
        return false;
    }

    if (true == descriptor->hasAlternateForm)
    {
        minimumRegselCount = formRegselCount[descriptor->alternateForm];
    }
    else
    {
        minimumRegselCount = formRegselCount[descriptor->primaryForm];
    }

    switch (minimumRegselCount)
    {
        case 3:
            if (false == isValidRegsel(tokens->tokens[3]))
            {
                printf("Error on line %u: Argument #3 for \"%s\" must be a regsel\n", lineNumber, tokens->tokens[0]);
                return false;
            }

            instruction->regsel3 = getRegsel(tokens->tokens[3]);

        case 2:
            if (false == isValidRegsel(tokens->tokens[2]))
            {
                printf("Error on line %u: Argument #2 for \"%s\" must be a regsel\n", lineNumber, tokens->tokens[0]);
                return false;
            }

            instruction->regsel2 = getRegsel(tokens->tokens[2]);

        case 1:
            if (false == isValidRegsel(tokens->tokens[1]))
            {
                printf("Error on line %u: Argument #1 for \"%s\" must be a regsel\n", lineNumber, tokens->tokens[0]);
                return false;
            }

            instruction->regsel1 = getRegsel(tokens->tokens[1]);

        default:
            break;
    }

    if (true == descriptor->hasAlternateForm)
    {
        isAlternateForm = true;
        switch (formRegselCount[descriptor->primaryForm])
        {
            case 3:
                if (true == isValidRegsel(tokens->tokens[3]))
                {
                    instruction->regsel3 = getRegsel(tokens->tokens[3]);
                    isAlternateForm = false;
                }
                break;
            
            case 2:
                if (true == isValidRegsel(tokens->tokens[2]))
                {
                    instruction->regsel2 = getRegsel(tokens->tokens[2]);
                    isAlternateForm = false;
                }
                break;

            case 1:
                if (true == isValidRegsel(tokens->tokens[1]))
                {
                    instruction->regsel1 = getRegsel(tokens->tokens[1]);
                    isAlternateForm = false;
                }
                break;
        }
    }

    if (isAlternateForm)
    {
        instruction->opCode = descriptor->opCodePrimaryVal;
    }
    else
    {
        instruction->opCode = descriptor->opCodeAlternateVal;
    }

    return true;
}

static bool parseInstruction(tokens_t *tokens, instruction_t *instruction, uint32_t lineNumber)
{
    instructionDescriptor_t *descriptorInstance = NULL;

    if (NULL == tokens             ||
        0    == tokens->tokenCount ||
        NULL == instruction)
    {
        printf("Internal error\n");
        return false;
    }

    for (uint16_t i = 0; i < sizeof(instructionDescriptors) / sizeof(instructionDescriptor_t); i++)
    {
        descriptorInstance = &(instructionDescriptors[i]);

        if (0 == strcmp(tokens->tokens[0], descriptorInstance->opCodeStr))
        {
            if (false == parseFormAndRegsel(tokens, instruction, descriptorInstance, lineNumber))
            {
                return false;
            }
            return true;
        }
    }

    printf("Error on line %u: Invalid instruction \"%s\"\n", lineNumber, tokens->tokens[0]);

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

        if (0 == tokens.tokenCount)
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

        if (false == parseInstruction(&tokens, currentInstruction, lineNumber))
        {
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