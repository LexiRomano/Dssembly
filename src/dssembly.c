#include "dssembly.h"

static FILE *inputFile          = NULL;
static char *outputFileLocation = NULL;

static instructionList_t instructionList = {0};
static labelList_t       labelList       = {0};

static instructionDescriptor_t instructionDescriptors[] =
{
    {"MOVE", 0x06, form_2, true,  0x07, form_4, false, 0, true,  false},
    {"TERM", 0xFF, form_6, false, 0,    0,      false, 0, false, false}
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

static uint32_t formMaxArgSize[] =
{
    0,         // form_1
    0,         // form_2
    0xFFFF,    // form_3
    0,         // form_4
    0xFFFFFF,  // form_5
    0,         // form_6
    0xFFFFFFF, // form_7
};

static bool parseArgs(int argc, char* argv[])
{
    if (argc != 3)
    {   
        if (argc > 3)
        {
            printf("Too many arguments!\n");
        }
        else
        {
            printf("Insufficient arguments!\n");
        }
        return false;
    }

    inputFile = fopen(argv[1], "r");

    if (NULL == inputFile)
    {
        printf("Failed to open input file \"%s\"\n", argv[1]);
        return false;
    }

    outputFileLocation = argv[2];
    

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
    if (NULL        == str ||
        strlen(str) != 2)
    {
        return false;
    }

    for (uint8_t i = 0; i < 0x10; i++)
    {
        if (0 == strncmp(str, regselDescriptors[i], 3))
        {
            return true;
        }
    }

    return false;
}

static bool parseFormAndRegsel(tokens_t                *tokens,
                               instruction_t           *instruction,
                               instructionDescriptor_t *descriptor)
{
    uint8_t minimumRegselCount = 0;
    bool    isAlternateForm    = false;

    if (NULL == tokens ||
        NULL == descriptor)
    {
        INTERNAL_ERROR;
        return false;
    }
    

    if (tokens->tokenCount - 1 != formArgCount[descriptor->primaryForm])
    {
        if (tokens->tokenCount - 1 > formArgCount[descriptor->primaryForm])
        {
            printf("Error on line %u: Too many arguments for instruction \"%s\"\n", instruction->lineNumber, tokens->tokens[0]);
        }
        else
        {
            printf("Error on line %u: Too few arguments for instruction \"%s\"\n", instruction->lineNumber, tokens->tokens[0]);
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
                printf("Error on line %u: Argument #3 for \"%s\" must be a regsel\n", instruction->lineNumber, tokens->tokens[0]);
                return false;
            }

            instruction->regsel3 = getRegsel(tokens->tokens[3]);

        case 2:
            if (false == isValidRegsel(tokens->tokens[2]))
            {
                printf("Error on line %u: Argument #2 for \"%s\" must be a regsel\n", instruction->lineNumber, tokens->tokens[0]);
                return false;
            }

            instruction->regsel2 = getRegsel(tokens->tokens[2]);

        case 1:
            if (false == isValidRegsel(tokens->tokens[1]))
            {
                printf("Error on line %u: Argument #1 for \"%s\" must be a regsel\n", instruction->lineNumber, tokens->tokens[0]);
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
        instruction->opCode = descriptor->opCodeAlternateVal;
    }
    else
    {
        instruction->opCode = descriptor->opCodePrimaryVal;
    }

    if (descriptor->hasInstructionAugment)
    {
        instruction->hasInstructionAugment = true;
        instruction->instructionAugment    = descriptor->instructionAugment;
    }

    return true;
}

static bool parseBinary(char *str, uint32_t *out)
{
    uint64_t sum = 0;
    char    *pointer = str;

    while (*pointer != '\0')
    {
        sum *= 0b10;

        switch (*pointer)
        {
            case '1':
                sum++;
            case '0':
                break;
            default:
                return false;
        }

        pointer++;
    }

    if (sum > 0xFFFFFFFF)
    {
        return false;
    }

    *out = (uint32_t) sum;

    return true;
}

static bool parseDecimal(char *str, uint32_t *out)
{
    uint64_t sum = 0;
    char    *pointer = str;

    while (*pointer != '\0')
    {
        sum *= 10;

        switch (*pointer)
        {
            case '9':
                sum++;
            case '8':
                sum++;
            case '7':
                sum++;
            case '6':
                sum++;
            case '5':
                sum++;
            case '4':
                sum++;
            case '3':
                sum++;
            case '2':
                sum++;
            case '1':
                sum++;
            case '0':
                break;
            default:
                return false;
        }

        pointer++;
    }

    if (sum > 0xFFFFFFFF)
    {
        return false;
    }

    *out = (uint32_t) sum;

    return true;
}

static bool parseHexadecimal(char *str, uint32_t *out)
{
    uint64_t sum = 0;
    char    *pointer = str;

    while (*pointer != '\0')
    {
        sum *= 0x10;

        switch (*pointer)
        {
            case 'F':
            case 'f':
                sum++;
            case 'E':
            case 'e':
                sum++;
            case 'D':
            case 'd':
                sum++;
            case 'C':
            case 'c':
                sum++;
            case 'B':
            case 'b':
                sum++;
            case 'A':
            case 'a':
                sum++;
            case '9':
                sum++;
            case '8':
                sum++;
            case '7':
                sum++;
            case '6':
                sum++;
            case '5':
                sum++;
            case '4':
                sum++;
            case '3':
                sum++;
            case '2':
                sum++;
            case '1':
                sum++;
            case '0':
                break;
            default:
                return false;
        }

        pointer++;
    }

    if (sum > 0xFFFFFFFF)
    {
        return false;
    }

    *out = (uint32_t) sum;

    return true;
}

static bool parseLiteral(char *literal, instruction_t *instruction)
{
    if (NULL == literal ||
        NULL == instruction)
    {
        return false;
    }

    if (strlen(literal) >= 3)
    {
        // Has a prefix
        if (0 == strncmp(literal, "0b", 2))
        {
            // Binary
            return parseBinary(&(literal[2]), &(instruction->immediate));

        }
        else if (0 == strncmp(literal, "0x", 2))
        {
            // Hexadecimal
            return parseHexadecimal(&(literal[2]), &(instruction->immediate));
        }
    }

    return  parseDecimal(literal, &(instruction->immediate));;
}

static bool validateLabel(char *str)
{
    if (NULL == str)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (true == isValidRegsel(str))
    {
        return false;
    }

    switch (str[0])
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case ':':
            return false;
    }

    return true;
}

static bool parseInstructionArguments(tokens_t                *tokens,
                                      instruction_t           *instruction,
                                      instructionDescriptor_t *descriptor)
{
    form_e form            = 0;
    bool   isAlternateForm = false;
    char  *argument        = NULL;

    if (NULL == tokens      ||
        NULL == instruction ||
        NULL == descriptor)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (instruction->opCode == descriptor->opCodePrimaryVal)
    {
        form = descriptor->primaryForm;
    }
    else
    {
        isAlternateForm = true;
        form = descriptor->alternateForm;
    }

    if (formArgCount[form] == formRegselCount[form] &&
        ((true  == isAlternateForm &&
          false == descriptor->alternateFormUsesArgAugment) ||
        false == isAlternateForm))
    {
        // All regsel or no argument
        return true;
    }

    argument = tokens->tokens[tokens->tokenCount - 1];

    if (false == parseLiteral(argument, instruction))
    {
        if (false == descriptor->takesAddress)
        {
            printf("Error on line %u: Malformed literal \"%s\"\n", instruction->lineNumber, argument);
            return false;
        }

        if (false == validateLabel(argument))
        {
            // No way this could be a label
            printf("Error on line %u: Invalid argument \"%s\"\n", instruction->lineNumber, argument);
            return false;
        }

        // Treat as a label for now.
        instruction->targetLabel = calloc(strlen(argument) + 1, sizeof(char));
        snprintf(instruction->targetLabel, strlen(argument) + 1, "%s", argument);
    }
    else if (instruction->immediate > formMaxArgSize[form] &&
             false == descriptor->alternateFormUsesArgAugment)
    {
        printf("Error on line %u: Literal too large for instruction \"%s\"\n", instruction->lineNumber, argument);
        return false;
    }

    return true;
}

static uint8_t getInstructionSize(instruction_t *instruction)
{
    uint8_t size = 4;

    if (NULL == instruction)
    {
        return size;
    }

    if (instruction->hasInstructionAugment)
    {
        size += 1;
    }

    if (instruction->hasArgAugment)
    {
        size += 4;
    }

    return size;
}

static bool parseLabel(char *token, char **label, uint32_t lineNumber)
{
    if (NULL == token ||
        NULL == label)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (strlen(token) == 0 ||
        false == validateLabel(token))
    {
        printf("Error on line %u: Invalid label \"%s\"\n", lineNumber, token);
        return false;
    }

    if (NULL != *label)
    {
        printf("Error on line %u: Cannot have more than one label in a row\n", lineNumber);
        return false;
    }

    *label = calloc(strlen(token) + 1, sizeof(char));
    snprintf(*label, strlen(token) + 1, "%s", token);

    return true;
}

static bool parseInstruction(tokens_t      *tokens,
                             instruction_t *instruction,
                             char          *label,
                             uint32_t       address)
{
    instructionDescriptor_t *descriptorInstance = NULL;
    label_t                 *tmpLabel           = NULL;

    if (NULL == tokens             ||
        0    == tokens->tokenCount ||
        NULL == instruction)
    {
        INTERNAL_ERROR;
        return false;
    }

    for (uint16_t i = 0; i < sizeof(instructionDescriptors) / sizeof(instructionDescriptor_t); i++)
    {
        descriptorInstance = &(instructionDescriptors[i]);

        if (0 == strcmp(tokens->tokens[0], descriptorInstance->instructionStr))
        {
            if (false == parseFormAndRegsel(tokens, instruction, descriptorInstance))
            {
                return false;
            }

            if (false == parseInstructionArguments(tokens, instruction, descriptorInstance))
            {
                return false;
            }

            if (NULL != label)
            {
                tmpLabel = addNewLabel(&labelList);

                if (NULL == tmpLabel)
                {
                    INTERNAL_ERROR;
                    return false;
                }

                tmpLabel->instruction = instruction;
                tmpLabel->label       = label;
            }

            instruction->address = address;
            
            return true;
        }
    }

    printf("Error on line %u: Invalid instruction \"%s\"\n", instruction->lineNumber, tokens->tokens[0]);

    return false;
}

static bool parseInputFile()
{
    char           inputBuffer[2048]   = {0};
    instruction_t *currentInstruction  = NULL;
    instruction_t *previousInstruction = NULL;
    tokens_t       tokens              = {0};
    uint32_t       lineNumber          = 0;
    char          *label               = NULL;
    bool           labelFound          = false;

    // First pass
    while (fgets(inputBuffer, sizeof(inputBuffer), inputFile))
    {
        lineNumber++;

        if (false == parseTokens(inputBuffer, &tokens))
        {
            INTERNAL_ERROR;
            freeInstructionListContents(&instructionList);
            freeLabelListContents(&labelList);
            return false;
        }

        if (0 == tokens.tokenCount)
        {
            continue;
        }

        if (tokens.tokens[0][0] == ':' )
        {
            if (false == parseLabel(&(tokens.tokens[0][1]), &label, lineNumber))
            {
                return false;
            }
        }
        else
        {

            currentInstruction = addNewInstruction(&instructionList);

            if (NULL == currentInstruction)
            {
                INTERNAL_ERROR;
                freeTokensContents(&tokens);
                freeInstructionListContents(&instructionList);
                freeLabelListContents(&labelList);
                fclose(inputFile);
                return false;
            }

            currentInstruction->lineNumber = lineNumber;

            if (false == parseInstruction(&tokens,
                                          currentInstruction,
                                          label,
                                          previousInstruction == NULL ?
                                              0 : previousInstruction->address + getInstructionSize(previousInstruction)))
            {
                if (NULL != label)
                {
                    free(label);
                }
                freeTokensContents(&tokens);
                freeInstructionListContents(&instructionList);
                freeLabelListContents(&labelList);
                fclose(inputFile);
                return false;
            }

            if (NULL != label)
            {
                label = NULL;
            }

            previousInstruction = currentInstruction;
        }
    }

    freeTokensContents(&tokens);
    fclose(inputFile);

    if (NULL != label)
    {
        printf("Error on line %u: Dangling label at end of file\n", lineNumber);
        free(label);
        freeInstructionListContents(&instructionList);
        freeLabelListContents(&labelList);
        return false;
    }

    // Second pass
    if (NULL == instructionList.first)
    {
        return true;
    }

    for (currentInstruction = instructionList.first; NULL != currentInstruction; currentInstruction = currentInstruction->next)
    {
        if (NULL == currentInstruction->targetLabel)
        {
            continue;
        }

        if (NULL != currentInstruction->targetLabel)
        {
            labelFound = false;
            for (label_t *labelInstance = labelList.first; labelInstance != NULL; labelInstance = labelInstance->next)
            {
                if (0 == strcmp(currentInstruction->targetLabel, labelInstance->label))
                {
                    labelFound = true;
                    currentInstruction->targetLabel_p = labelInstance->instruction;
                }
            }

            if (false == labelFound)
            {
                printf("Error on line %u: could not find label \"%s\"\n", currentInstruction->lineNumber, currentInstruction->targetLabel);
                freeInstructionListContents(&instructionList);
                freeLabelListContents(&labelList);
                return false;
            }
        }
    }

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