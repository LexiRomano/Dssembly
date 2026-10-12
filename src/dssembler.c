#include "dssembly.h"

static FILE    *inputFile      = NULL;
static char    *inputFileName  = NULL;
static FILE    *outputFile     = NULL;
static char    *outputFileName = NULL;
static uint32_t lineNumber     = 0;

static bool link = false;

static sectionList_t         sectionList            = {0};
static section_t            *currentSection         = NULL;
static labelList_t          *currentLabelList       = NULL;
static labelList_t          *currentExportedLabels  = NULL;
static requiredLabelList_t  *currentRequiredLabels  = NULL;
static originRelativeList_t *currentOriginRelatives = NULL;
static aliasList_t           aliasList              = {0};

static instructionForm_e moveFormResolver(tokens_t *tokens);

static int moveOpCodeResolver(tokens_t         *tokens,
                              instructionForm_e instructionForm);

static instructionForm_e loadFormResolver(tokens_t *tokens);

static int loadOpCodeResolver(tokens_t         *tokens,
                              instructionForm_e instructionForm);

static instructionDescriptor_t instructionDescriptors[] =
{
    {"move",     false, 0, moveFormResolver, 0, moveOpCodeResolver, NULL, true},
    {"load.pc",  true,  0, loadFormResolver, 0, loadOpCodeResolver, NULL, true},
    {"load..pc", true,  0, loadFormResolver, 0, loadOpCodeResolver, NULL, true},
    {"load...",  true,  0, loadFormResolver, 0, loadOpCodeResolver, NULL, false},
    {"load....", true,  0, loadFormResolver, 0, loadOpCodeResolver, NULL, false},
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
    "OC",
    "SB",
    "SS",
    "SP",
    "IL",
    "FL"
};

static uint8_t formArgCount[] = 
{
    0, // form 1
    1, // form 1w
    1, // form 1h
    1, // form 1c
    1, // form 2
    2, // form 2w
    2, // form 2h
    2, // form 2c
    2, // form 3
    3, // form 3w
    3, // form 3h
    3, // form 3c
    3, // form 4
    4, // form 4w
    4, // form 4h
    4, // form 4c
    0, // form 5
    1, // form 5w
    1, // form 5h
    1, // form 5c
    1, // form 6
    2, // form 6w
    2, // form 6h
    2, // form 6c
    2, // form 7
    3, // form 7w
    3, // form 7h
    3, // form 7c
    3, // form 8
    4, // form 8w
    4, // form 8h
    4, // form 8c
};

static uint8_t formInstructionSize[] = 
{
    1, // form 1
    5, // form 1w
    3, // form 1h
    2, // form 1c
    2, // form 2
    6, // form 2w
    4, // form 2h
    3, // form 2c
    2, // form 3
    6, // form 3w
    4, // form 3h
    3, // form 3c
    3, // form 4
    7, // form 4w
    5, // form 4h
    4, // form 4c
    2, // form 5
    6, // form 5w
    4, // form 5h
    3, // form 5c
    2, // form 6
    6, // form 6w
    4, // form 6h
    3, // form 6c
    3, // form 7
    7, // form 7w
    5, // form 7h
    4, // form 7c
    3, // form 8
    7, // form 8w
    5, // form 8h
    4, // form 8c
};

static uint8_t formRegselCount[] = 
{
    0, // form 1
    0, // form 1w
    0, // form 1h
    0, // form 1c
    1, // form 2
    1, // form 2w
    1, // form 2h
    1, // form 2c
    2, // form 3
    2, // form 3w
    2, // form 3h
    2, // form 3c
    3, // form 4
    3, // form 4w
    3, // form 4h
    3, // form 4c
    0, // form 5
    0, // form 5w
    0, // form 5h
    0, // form 5c
    1, // form 6
    1, // form 6w
    1, // form 6h
    1, // form 6c
    2, // form 7
    2, // form 7w
    2, // form 7h
    2, // form 7c
    3, // form 8
    3, // form 8w
    3, // form 8h
    3, // form 8c
};

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

static bool parseFloat(char *literal, uint32_t *out)
{
    float    outf            = 0;
    bool     isNegative      = false;
    uint32_t intParse        = 0;
    char     charBuf[32]     = {0};
    char    *decimalLocation = NULL;
    float    divisor         = 0;

    if (NULL == literal ||
        '\0' == literal[0] ||
        NULL == strstr(literal, "."))
    {
        return false;
    }

    if ('-' == literal[0])
    {
        isNegative = true;
        literal++;

        if ('\0' == literal[0])
        {
            return false;
        }
    }

    decimalLocation = strstr(literal, ".");

    memcpy(charBuf, literal, decimalLocation - literal);
    charBuf[decimalLocation - literal] = '\0';

    if (false == parseDecimal(charBuf, &intParse))
    {
        return false;
    }

    outf = intParse;

    if (false == parseDecimal(decimalLocation + 1, &intParse))
    {
        return false;
    }

    divisor = 1.0;
    for (uint8_t i = strlen(decimalLocation + 1); i != 0; i--)
    {
        divisor *= 10.0;
    }

    outf += ((float)intParse) / divisor;

    if (isNegative)
    {
        outf *= -1.0;
    }

    memcpy(out, &outf, 4);

    return true;
}

static bool parseLiteral(char *literal, uint32_t *out, bool *isFloat)
{
    bool makeNegative = false;

    if (NULL == literal ||
        NULL == out     ||
        NULL == isFloat)
    {
        return false;
    }

    if (NULL != strstr(literal, "."))
    {
        if (parseFloat(literal, out))
        {
            *isFloat = true;
            return true;
        }

        return false;
    }

    if ('-' == literal[0])
    {
        makeNegative = true;
        literal++;
    }

    if (strlen(literal) >= 3)
    {
        // Has a prefix
        if (0 == strncmp(literal, "0b", 2))
        {
            // Binary
            if (false == parseBinary(&(literal[2]), out))
            {
                return false;
            }

        }
        else if (0 == strncmp(literal, "0x", 2))
        {
            // Hexadecimal
            if (false == parseHexadecimal(&(literal[2]), out))
            {
                return false;
            }
        }
        else if (false == parseDecimal(literal, out))
        {
            return false;
        }
    }
    else if (false == parseDecimal(literal, out))
    {
        return false;
    }

    if (makeNegative)
    {
        *out = ~(*out) + 1;
    }

    return true;
}

static instructionForm_e moveFormResolver(tokens_t *tokens)
{
    bool     isFloat       = false;
    uint32_t parsedLiteral = 0;

    if (NULL == tokens             ||
        1    >  tokens->tokenCount ||
        0    != strcmp("move", tokens->tokens[0]))
    {
        INTERNAL_ERROR;
        return INS_FORM_ERR;
    }

    if (3 != tokens->tokenCount)
    {
        printf("%s:%u: \"move\" requires exactly 2 arguments\n", inputFileName, lineNumber);
        return INS_FORM_ERR;
    }

    if (false == isValidRegsel(tokens->tokens[1]))
    {
        printf("%s:%u: \"move\" requires the first argument to be a regsel\n", inputFileName, lineNumber);
        return INS_FORM_ERR;
    }

    if (isValidRegsel(tokens->tokens[2]))
    {
        return INS_FORM_3;
    }

    if (parseLiteral(tokens->tokens[2], &parsedLiteral, &isFloat))
    {
        if (isFloat)
        {
            return INS_FORM_2W;
        }

        if (parsedLiteral <= 0xFF ||
            parsedLiteral >= 0xFFFFFF80)
        {
            return INS_FORM_2C;
        }

        if (parsedLiteral <= 0xFFFF ||
            parsedLiteral >= 0xFFFF8000)
        {
            return INS_FORM_2H;
        }
    }

    return INS_FORM_2W;
}

static int moveOpCodeResolver(tokens_t         *tokens,
                              instructionForm_e instructionForm)
{
    bool     isFloat       = false;
    uint32_t parsedLiteral = 0;

    if (NULL == tokens             ||
        1    >  tokens->tokenCount ||
        0    != strcmp("move", tokens->tokens[0]))
    {
        INTERNAL_ERROR;
        return -1;
    }

    if (3 != tokens->tokenCount)
    {
        printf("%s:%u: \"move\" requires exactly 2 arguments\n", inputFileName, lineNumber);
        return INS_FORM_ERR;
    }

    if (INS_FORM_3 == instructionForm)
    {
        return 0x01;
    }

    if (INS_FORM_2W == instructionForm)
    {
        return 0x02;
    }

    if (false == parseLiteral(tokens->tokens[2], &parsedLiteral, &isFloat))
    {
        INTERNAL_ERROR;
        return -1;
    }

    if (INS_FORM_2H == instructionForm)
    {
        if (parsedLiteral > 0xFFFF)
        {
            return 0x04;
        }
        return 0x03;
    }

    if (INS_FORM_2C == instructionForm)
    {
        if (parsedLiteral > 0xFF)
        {
            return 0x06;
        }
        return 0x05;
    }

    INTERNAL_ERROR;
    return -1;
}

static instructionForm_e loadFormResolver(tokens_t *tokens)
{
    if (NULL == tokens ||
        1    >  tokens->tokenCount)
    {
        return INS_FORM_ERR;
    }

    if (3 != tokens->tokenCount)
    {
        printf("%s:%u: \"%s\" requires exactly 2 arguments\n",
               inputFileName, lineNumber, tokens->tokens[0]);
        return INS_FORM_ERR;
    }

    if (isValidRegsel(tokens->tokens[2]))
    {
        return INS_FORM_3;
    }

    return INS_FORM_2W;
}

static int loadOpCodeResolver(tokens_t         *tokens,
                              instructionForm_e instructionForm)
{
    char                *instructionNameShift = NULL;
    loadWidthModifiers_e lwm                  = 0;
    addressingMode_e     addressingMode       = 0;

    if (NULL == tokens ||
        1    >  tokens->tokenCount)
    {
        return -1;
    }

    if (3 != tokens->tokenCount)
    {
        printf("%s:%u: \"%s\" requires exactly 2 arguments\n",
               inputFileName, lineNumber, tokens->tokens[0]);
        return -1;
    }

    // Skip over 'load' and leave us with the xx part
    instructionNameShift = tokens->tokens[0] + 4;

    // Getting the width modifier
    if ('w' == instructionNameShift[0])
    {
        lwm = LWM_W;
        instructionNameShift += 1;
    }
    else if ('h' == instructionNameShift[0])
    {
        lwm = LWM_H;
        instructionNameShift += 1;
    }
    else if ('\0' != instructionNameShift[0] &&
             's'  == instructionNameShift[0] &&
             'h'  == instructionNameShift[1])
    {
        lwm = LWM_SH;
        instructionNameShift += 2;
    }
    else if ('c' == instructionNameShift[0])
    {
        lwm = LWM_C;
        instructionNameShift += 1;
    }
    else if ('\0' != instructionNameShift[0] &&
             's'  == instructionNameShift[0] &&
             'c'  == instructionNameShift[1])
    {
        lwm = LWM_SH;
        instructionNameShift += 2;
    }
    else
    {
        printf("%s:%u: unknown instruction \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
        return -1;
    }

    // Getting the addressing mode
    if (2 != strlen(instructionNameShift))
    {
        printf("%s:%u: unknown instruction \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
        return -1;
    }

    if (0 == strcmp(instructionNameShift, "ab"))
    {
        addressingMode = ADDRESSING_MODE_AB;
    }
    else if (0 == strcmp(instructionNameShift, "pc"))
    {
        addressingMode = ADDRESSING_MODE_PC;
    }
    else if (0 == strcmp(instructionNameShift, "oa"))
    {
        addressingMode = ADDRESSING_MODE_OA;
    }
    else if (0 == strcmp(instructionNameShift, "ob"))
    {
        addressingMode = ADDRESSING_MODE_OB;
    }
    else if (0 == strcmp(instructionNameShift, "oc"))
    {
        addressingMode = ADDRESSING_MODE_OC;
    }
    else
    {
        printf("%s:%u: unknown instruction \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
        return -1;
    }

    if (INS_FORM_3 == instructionForm)
    {
        return 0x07 + lwm * 5 + addressingMode;
    }
    return 0x20 + lwm * 5 + addressingMode;
}

static int getInstructionSize(tokens_t *tokens)
{
    instructionDescriptor_t *descriptorInstance = NULL;
    instructionForm_e        instructionForm    = 0;

    if (NULL == tokens             ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return -1;
    }

    for (uint16_t i = 0; i < sizeof(instructionDescriptors) / sizeof(instructionDescriptor_t); i++)
    {
        descriptorInstance = &(instructionDescriptors[i]);

        if (0 == strcmp(tokens->tokens[0], descriptorInstance->instructionStr) ||
            (descriptorInstance->useWildcards &&
             simpleRegex(descriptorInstance->instructionStr, tokens->tokens[0])))
        {
            if (NULL != descriptorInstance->formResolverFunction)
            {
                instructionForm = descriptorInstance->formResolverFunction(tokens);

                if (INS_FORM_ERR == instructionForm)
                {
                    return -1;
                }

                return formInstructionSize[instructionForm];
            }

            // TODO
            INTERNAL_ERROR;
            return -1;
        }
    }

    printf("%s:%u: Invalid instruction \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);

    return -1;
}

static bool parseLabel(char *labelWithoutColon, uint32_t address)
{
    label_t *newLabel = NULL;

    if (NULL == labelWithoutColon)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (NULL == currentSection)
    {
        printf("%s:%u label \"%s\" not in a section\n", inputFileName, lineNumber, labelWithoutColon);
        return false;
    }

    if (true == isValidRegsel(labelWithoutColon))
    {
        printf("%s:%u: Label name cannot be a regsel\n", inputFileName, lineNumber);
        return false;
    }

    if (false == isAlphanumericString(labelWithoutColon))
    {
        printf("%s:%u: label \"%s\" contains invalid characters\n", inputFileName, lineNumber, labelWithoutColon);
        return false;
    }

    if (NULL != getLabel(currentLabelList, labelWithoutColon))
    {
        printf("%s:%u: label \"%s\" already exists\n", inputFileName, lineNumber, labelWithoutColon);
        return false;
    }

    if (NULL != getAlias(&aliasList, labelWithoutColon))
    {
        printf("%s:%u: label \"%s\" conflicts with alias of the same name\n", inputFileName, lineNumber, labelWithoutColon);
        return false;
    }

    newLabel = addNewLabel(currentLabelList);

    if (NULL == newLabel)
    {
        INTERNAL_ERROR;
        return false;
    }

    newLabel->label   = strcpy(calloc(strlen(labelWithoutColon) + 1, sizeof(char)), labelWithoutColon);
    newLabel->address = address;

    return true;
}

static bool parseAliasDefine(tokens_t *tokens, char *fileName, uint32_t lineNumber)
{
    alias_t *newAlias = NULL;

    if (tokens->tokenCount != 3)
    {
        printf("%s:%u: define requires two arguments\n", fileName, lineNumber);
        return false;
    }

    if (false == isAlphanumericString(tokens->tokens[1]))
    {
        printf("%s:%u: alias \"%s\" contains invalid characters\n", fileName, lineNumber, tokens->tokens[1]);
        return false;
    }

    if (NULL != getAlias(&aliasList, tokens->tokens[1]))
    {
        printf("%s:%u: alias \"%s\" already exists\n", fileName, lineNumber, tokens->tokens[1]);
        return false;
    }

    if (NULL != getLabel(currentLabelList, tokens->tokens[1]))
    {
        printf("%s:%u: alias \"%s\" conflicts with label of the same name\n", fileName, lineNumber, tokens->tokens[1]);
        return false;
    }

    newAlias = addNewAlias(&aliasList);

    if (NULL == newAlias)
    {
        INTERNAL_ERROR;
        return false;
    }

    newAlias->alias     = strdup(tokens->tokens[1]);
    newAlias->expansion = strdup(tokens->tokens[2]);

    return true;
}

static bool parseAliasUndef(tokens_t *tokens, char *fileName, uint32_t lineNumber)
{
    if (tokens->tokenCount != 2)
    {
        printf("%s:%u: undef requires two arguments\n", fileName, lineNumber);
        return false;
    }

    removeAlias(&aliasList, tokens->tokens[1]);

    return true;
}

static bool doInclude(tokens_t *tokens)
{
    FILE    *fd                = NULL;
    char     inputBuffer[2048] = {0};
    uint32_t fileLineNumber    = 0;
    tokens_t fileTokens        = {0};

    if (NULL == tokens)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (2 != tokens->tokenCount)
    {
        printf("%s:%u: include directive takes one argument\n", inputFileName, lineNumber);
        return false;
    }

    fd = fopen(tokens->tokens[1], "r");

    if (NULL == fd)
    {
        printf("%s:%u: failed to open \"%s\" for import\n", inputFileName, lineNumber, tokens->tokens[1]);
        return false;
    }

    while (fgets(inputBuffer, 2048, fd))
    {
        fileLineNumber++;

        freeTokensContents(&fileTokens);

        if (false == parseTokens(inputBuffer, &fileTokens))
        {
            fclose(fd);
            INTERNAL_ERROR;
            return false;
        }

        if (0 == fileTokens.tokenCount)
        {
            continue;
        }

        if (0 == strcmp(fileTokens.tokens[0], DEFINE_STR))
        {
            if (false == parseAliasDefine(&fileTokens, tokens->tokens[1], fileLineNumber))
            {
                freeTokensContents(&fileTokens);
                return false;
            }
        }
        else if (0 == strcmp(fileTokens.tokens[0], UNDEF_STR))
        {
            if (false == parseAliasUndef(&fileTokens, tokens->tokens[1], fileLineNumber))
            {
                freeTokensContents(&fileTokens);
                return false;
            }
        }
    }

    freeTokensContents(&fileTokens);

    return true;
}

static char *parseEscapeCharacters(const char *inputString)
{
    char     buf[2048]   = {0};
    uint16_t inputIndex  = 0;
    uint16_t outputIndex = 0;
    bool     inEscape    = false;

    while ('\0' != inputString[inputIndex])
    {
        if (inEscape)
        {
            switch (inputString[inputIndex])
            {
                case 'n':
                    buf[outputIndex++] = '\n';
                    break;
                case 'r':
                    buf[outputIndex++] = '\r';
                    break;
                default:
                    buf[outputIndex++] = inputString[inputIndex];
                    break;
            }
            inEscape = false;
        }
        else if ('\\' == inputString[inputIndex])
        {
            inEscape = true;
        }
        else
        {
            buf[outputIndex++] = inputString[inputIndex];
        }

        inputIndex++;
    }

    buf[outputIndex] = '\0';

    return strcpy(calloc(strlen(buf) + 1, sizeof(char)), buf);
}

static bool parseDotDirectiveFirstPass(tokens_t *tokens, uint32_t *address)
{
    uint32_t buf32           = 0;
    char    *parsedSetString = NULL;
    label_t *tmpLabel1       = NULL;
    label_t *tmpLabel2       = NULL;
    bool     wasFloat        = false;

    if (NULL == tokens ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (0 == strcmp(tokens->tokens[0], DEFINE_STR))
    {
        return parseAliasDefine(tokens, inputFileName, lineNumber);
    }

    if (0 == strcmp(tokens->tokens[0], UNDEF_STR))
    {
        return parseAliasUndef(tokens, inputFileName, lineNumber);
    }

    resolveAliases(&aliasList, tokens);

    if (0 == strcmp(tokens->tokens[0], INCLUDE_STR))
    {
        return doInclude(tokens);
    }

    if (0 == strcmp(tokens->tokens[0], RESERVE_STR))
    {
        if (2 != tokens->tokenCount)
        {
            printf("%s:%u: reserve directive takes one argument\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL == currentSection)
        {
            printf("%s:%u: reserve directive not in section\n", inputFileName, lineNumber);
            return false;
        }

        if (false == parseLiteral(tokens->tokens[1], &buf32, &wasFloat))
        {
            printf("%s:%u: could not parse literal \"%s\"\n", inputFileName, lineNumber, tokens->tokens[1]);
            return false;
        }

        if (wasFloat)
        {
            printf("%s:%u: cannot reserve fractional number of bytes\n", inputFileName, lineNumber);
            return false;
        }

        *address += buf32;
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], SET_STR))
    {
        if (3 != tokens->tokenCount)
        {
            printf("%s:%u: set directive takes two argument\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL == currentSection)
        {
            printf("%s:%u: set directive not in section\n", inputFileName, lineNumber);
            return false;
        }

        if (1   == strlen(tokens->tokens[1]) &&
            '*' == tokens->tokens[1][0])
        {
            parsedSetString = parseEscapeCharacters(tokens->tokens[2]);
            buf32 = strlen(parsedSetString) + 1;
            free(parsedSetString);
            *address += buf32;
            return true;
        }
        
        if (1   == strlen(tokens->tokens[1]) &&
            '@' == tokens->tokens[1][0])
        {
            *address += 4;
            return true;
        }

        if (false == parseLiteral(tokens->tokens[1], &buf32, &wasFloat))
        {
            printf("%s:%u: could not parse literal \"%s\"\n", inputFileName, lineNumber, tokens->tokens[1]);
            return false;
        }

        if (wasFloat)
        {
            printf("%s:%u: cannot set fractional number of bytes\n", inputFileName, lineNumber);
            return false;
        }

        if (buf32 != 4 &&
            buf32 != 2 &&
            buf32 != 1)
        {
            printf("%s:%u: sets of numeric literals must be 1, 2, or 4 bytes\n", inputFileName, lineNumber);
            return false;
        }

        *address += buf32;
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], SECTION_STR))
    {
        if (false == link)
        {
            return true;
        }

        if (2 != tokens->tokenCount)
        {
            printf("%s:%u: section directive takes one argument\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL != getSection(&sectionList, tokens->tokens[1]))
        {
            printf("%s:%u: duplicate section name \"%s\"\n", inputFileName, lineNumber, tokens->tokens[1]);
            return false;
        }

        currentSection = addNewSection(&sectionList);

        currentSection->name = strcpy(calloc(strlen(tokens->tokens[1]) + 1, sizeof(char)),
                                      tokens->tokens[1]);
        *address = 0;

        currentLabelList       = &(currentSection->labelList);
        currentExportedLabels  = &(currentSection->exportedLabels);
        currentRequiredLabels  = &(currentSection->requiredLabels);
        currentOriginRelatives = &(currentSection->originRelatives);

        return true;
    }

    if (0 == strcmp(tokens->tokens[0], EXPORT_STR))
    {
        if (false == link)
        {
            return true;
        }

        if (2 != tokens->tokenCount)
        {
            printf("%s:%u: export directive takes one argument\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL == currentSection)
        {
            printf("%s:%u: export directive not in section\n", inputFileName, lineNumber);
            return false;
        }

        if (false == parseLabel(tokens->tokens[1], *address))
        {
            // Error already logged
            return false;
        }

        tmpLabel1 = addNewLabel(currentExportedLabels);

        if (NULL == tmpLabel1)
        {
            INTERNAL_ERROR;
            return false;
        }

        tmpLabel1->label = strcpy(calloc(strlen(tokens->tokens[1]) + 1, sizeof(char)), tokens->tokens[1]);

        tmpLabel2 = getLabel(currentLabelList, tokens->tokens[1]);

        if (NULL == tmpLabel2)
        {
            INTERNAL_ERROR;
            return false;
        }

        tmpLabel1->address = tmpLabel2->address;

        return true;
    }

    if (0 == strcmp(tokens->tokens[0], REQUIRES_STR))
    {
        if (false == link)
        {
            return true;
        }

        if (2 != tokens->tokenCount)
        {
            printf("%s:%u: requires directive takes one argument\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL == currentSection)
        {
            printf("%s:%u: requires directive not in section\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL != getRequiredLabel(currentRequiredLabels, tokens->tokens[1]))
        {
            printf("%s:%u: label \"%s\" required twice\n", inputFileName, lineNumber, tokens->tokens[1]);
            return false;
        }

        addNewRequiredLabel(currentRequiredLabels)->name = 
                strcpy(calloc(strlen(tokens->tokens[1]) + 1, sizeof(char)), tokens->tokens[1]);

        return true;
    }

    printf("%s:%u: invalid directive \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
    return false;
}

static bool completeSection()
{
    char nullChar = '\0';

    // Exported labels

    currentSection->exportedLabelsOffset = ftell(outputFile);

    for (label_t *tmpExport = currentExportedLabels->first;
         NULL != tmpExport;
         tmpExport = tmpExport->next)
    {
        if (1 != fwrite(tmpExport->label, strlen(tmpExport->label) + 1, 1, outputFile) ||
            1 != fwrite(&(tmpExport->address), 4, 1, outputFile))
        {
            printf("Error writing to \"%s\"\n", outputFileName);
            return false;
        }
    }

    if (1 != fwrite(&nullChar, 1, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileName);
        return false;
    }

    // Required labels

    currentSection->requiredLabelsOffset = ftell(outputFile);

    for (requiredLabel_t *tmpReq = currentRequiredLabels->first;
         NULL != tmpReq;
         tmpReq = tmpReq->next)
    {
        if (1 != fwrite(tmpReq->name, strlen(tmpReq->name) + 1, 1, outputFile) ||
            1 != fwrite(&(tmpReq->instances.count), 2, 1, outputFile))
        {
            printf("Error writing to \"%s\"\n", outputFileName);
            return false;
        }

        for (resolutionInstance_t *tmpInstance = tmpReq->instances.first;
             NULL != tmpInstance;
             tmpInstance = tmpInstance->next)
        {
            if (1 != fwrite(&(tmpInstance->instructionOffset), 4, 1, outputFile) ||
                1 != fwrite(&(tmpInstance->injectionOffset),   1, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
        }
    }

    if (1 != fwrite(&nullChar, 1, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileName);
        return false;
    }

    // Origin relatives

    currentSection->originRelativeOffset = ftell(outputFile);

    if (1 != fwrite(&(currentOriginRelatives->count), 2, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileName);
        return false;
    }

    for (originRelative_t *tmpOR = currentOriginRelatives->first;
         NULL != tmpOR;
         tmpOR = tmpOR->next)
    {
        if (1 != fwrite(&(tmpOR->address), 4, 1, outputFile))
        {
            printf("Error writing to \"%s\"\n", outputFileName);
            return false;
        }
    }

    return true;
}

static bool parseDotDirectiveSecondPass(tokens_t *tokens, uint32_t *address)
{
    uint32_t              reserveSize     = 0;
    uint32_t              setValue        = 0;
    uint16_t              buf16           = 0;
    uint8_t               buf8            = 0;
    int                   rc              = 0;
    char                 *parsedSetString = NULL;
    label_t              *tmpLbl          = NULL;
    requiredLabel_t      *tmpReqLbl       = NULL;
    resolutionInstance_t *tmpResInst      = NULL;
    bool                  wasFloat        = false;

    if (NULL == tokens ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], DEFINE_STR))
    {
        return parseAliasDefine(tokens, inputFileName, lineNumber);
    }

    if (0 == strcmp(tokens->tokens[0], UNDEF_STR))
    {
        return parseAliasUndef(tokens, inputFileName, lineNumber);
    }

    resolveAliases(&aliasList, tokens);

    if (0 == strcmp(tokens->tokens[0], INCLUDE_STR))
    {
        return doInclude(tokens);
    }

    if (0 == strcmp(tokens->tokens[0], RESERVE_STR))
    {
        if (2 != tokens->tokenCount)
        {
            printf("%s:%u: reserve directive takes one argument\n", inputFileName, lineNumber);
            return false;
        }

        if (false == parseLiteral(tokens->tokens[1], &reserveSize, &wasFloat))
        {
            printf("%s:%u: could not parse literal \"%s\"\n", inputFileName, lineNumber, tokens->tokens[1]);
            return false;
        }

        if (wasFloat)
        {
            printf("%s:%u: cannot reserve fractional number of bytes\n", inputFileName, lineNumber);
            return false;
        }

        if (reserveSize != 0)
        {
            if (reserveSize > 1 &&
                0 != fseek(outputFile, reserveSize - 1, SEEK_CUR))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }

            if ('\0' != fputc('\0', outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
        }

        *address += reserveSize;
        
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], SET_STR))
    {
        if (3 != tokens->tokenCount)
        {
            printf("%s:%u: set directive takes two argument\n", inputFileName, lineNumber);
            return false;
        }

        if (1   == strlen(tokens->tokens[1]) &&
            '*' == tokens->tokens[1][0])
        {
            parsedSetString = parseEscapeCharacters(tokens->tokens[2]);
            if (strlen(parsedSetString) + 1 !=  fwrite(parsedSetString, sizeof(char), strlen(parsedSetString) + 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                free(parsedSetString);
                return false;
            }
            free(parsedSetString);
            return true;
        }

        if (1   == strlen(tokens->tokens[1]) &&
            '@' == tokens->tokens[1][0])
        {
            if (NULL == currentSection)
            {
                printf("%s:%u: set directive not in section\n", inputFileName, lineNumber);
                return false;
            }

            if (link)
            {
                tmpLbl = getLabel(currentLabelList, tokens->tokens[2]);

                if (NULL != tmpLbl)
                {
                    // Add an origin relative entry
                    addNewOriginRelative(currentOriginRelatives)->address = *address;
                    if (false == fwrite(&(tmpLbl->address), 4, 1, outputFile))
                    {
                        printf("Error writing to \"%s\"\n", outputFileName);
                        return false;
                    }

                    return true;
                }

                tmpReqLbl = getRequiredLabel(currentRequiredLabels, tokens->tokens[2]);

                if (NULL == tmpReqLbl)
                {
                    printf("%s:%u: could not resolve label\n", inputFileName, lineNumber);
                    return false;
                }

                tmpResInst = addNewResolutionInstance(&(tmpReqLbl->instances));
                tmpResInst->instructionOffset = *address;
                tmpResInst->injectionOffset   = 0x80;
            }
            else
            {
                tmpLbl = getLabel(currentLabelList, tokens->tokens[2]);

                if (NULL == tmpLbl)
                {
                    printf("%s:%u: could not resolve label\n", inputFileName, lineNumber);
                    return false;
                }

                if (1 != fwrite(&(tmpLbl->address), 4, 1, outputFile))
                {
                    printf("Error writing to \"%s\"\n", outputFileName);
                    return false;
                }
            }

            return true;
        }

        if (false == parseLiteral(tokens->tokens[1], &reserveSize, &wasFloat))
        {
            printf("%s:%u: could not parse literal \"%s\"\n", inputFileName, lineNumber, tokens->tokens[1]);
            return false;
        }

        if (wasFloat)
        {
            printf("%s:%u: cannot set fractional number of bytes\n", inputFileName, lineNumber);
            return false;
        }

        if (false == parseLiteral(tokens->tokens[2], &setValue, &wasFloat))
        {
            printf("%s:%u: could not parse literal \"%s\"\n", inputFileName, lineNumber, tokens->tokens[2]);
            return false;
        }

        if (reserveSize != 1 &&
            reserveSize != 2 &&
            reserveSize != 4)
        {
            printf("%s:%u: sets of numeric literals must be 1, 2, or 4 bytes\n", inputFileName, lineNumber);
            return false;
        }

        if (wasFloat && reserveSize != 4)
        {
            printf("%s:%u: floats require 4 bytes to be reserved\n", inputFileName, lineNumber);
            return false;
        }

        if (setValue >= (((uint64_t) 0x100) << ((reserveSize - 1) * 8)))
        {
            printf("%s:%u: %s cannot fit into %u bytes\n", inputFileName, lineNumber, tokens->tokens[2], reserveSize);
            return false;
        }

        switch (reserveSize)
        {
            case 1:
                buf8 = setValue & 0xFF;
                rc = fwrite(&buf8, 1, 1, outputFile);
                break;
            case 2:
                buf16 = setValue & 0xFFFF;
                rc = fwrite(&buf16, 2, 1, outputFile);
                break;
            case 4:
                rc = fwrite(&setValue, 4, 1, outputFile);
                break;
        }

        if (1 != rc)
        {
            printf("Error writing to \"%s\"\n", outputFileName);
            return false;
        }

        *address += reserveSize;

        return true;
    }

    if (0 == strcmp(tokens->tokens[0], SECTION_STR))
    {
        if (false == link)
        {
            return true;
        }

        if (2 != tokens->tokenCount)
        {
            printf("%s:%u: section directive takes one argument\n", inputFileName, lineNumber);
            return false;
        }

        if (NULL != currentSection &&
            false == completeSection(outputFile))
        {
            return false;
        }

        currentSection = getSection(&sectionList, tokens->tokens[1]);

        if (NULL == currentSection)
        {
            INTERNAL_ERROR;
            return false;
        }

        currentSection->codeSegmentOffset = ftell(outputFile);

        *address = 0;
        currentLabelList       = &(currentSection->labelList);
        currentExportedLabels  = &(currentSection->exportedLabels);
        currentRequiredLabels  = &(currentSection->requiredLabels);
        currentOriginRelatives = &(currentSection->originRelatives);

        return true;
    }

    if (0 == strcmp(tokens->tokens[0], EXPORT_STR))
    {
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], REQUIRES_STR))
    {
        return true;
    }

    printf("%s:%u: invalid directive \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
    return false;
}

static bool parseRegselAndFlagConditions(tokens_t                *tokens,
                                         instructionForm_e        instructionForm)
{
    uint8_t byteOut = 0;

    if (NULL == tokens)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (formArgCount[instructionForm] != tokens->tokenCount - 1)
    {
        printf("%s:%u: expected %hhu arguments for \"%s\"\n",
               inputFileName, lineNumber, tokens->tokenCount, tokens->tokens[0]);
        return false;
    }

    // Anything form 1 won't have regsel or flag conditions
    if (instructionForm < INS_FORM_2)
    {
        return true;
    }

    // First byte //

    // Regsel 1
    if (formRegselCount[instructionForm] >= 1)
    {
        if (false == isValidRegsel(tokens->tokens[1]))
        {
            printf("%s:%u: expected regsel for the 1st argument of \"%s\"\n",
                    inputFileName, lineNumber, tokens->tokens[0]);
            return false;
        }

        byteOut = getRegsel(tokens->tokens[1]);
    }

    // Regsel 2
    if (formRegselCount[instructionForm] >= 2)
    {
        if (false == isValidRegsel(tokens->tokens[2]))
        {
            printf("%s:%u: expected regsel for the 2nd argument of \"%s\"\n",
                    inputFileName, lineNumber, tokens->tokens[0]);
            return false;
        }

        byteOut |= getRegsel(tokens->tokens[2]) << 4;
    }

    // TODO flag condition for byte 1

    if (1 != fwrite(&byteOut, 1, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileName);
        return false;
    }

    // Bail if we don't need a 2nd byte
    if ((instructionForm >= INS_FORM_2 &&
         instructionForm <= INS_FORM_3C) ||
        (instructionForm >= INS_FORM_5 &&
         instructionForm <= INS_FORM_6C))
    {
        return true;
    }
    // Second Byte //
    byteOut = 0;

    // Regsel 3
    if (formRegselCount[instructionForm] >= 3)
    {
        if (false == isValidRegsel(tokens->tokens[3]))
        {
            printf("%s:%u: expected regsel for the 3rd argument of \"%s\"\n",
                    inputFileName, lineNumber, tokens->tokens[0]);
            return false;
        }

        byteOut = getRegsel(tokens->tokens[3]);
    }

    // TODO flag condition for byte 2
    if (1 != fwrite(&byteOut, 1, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileName);
        return false;
    }

    return true;
}

static bool parseInstructionArgument(tokens_t                *tokens,
                                     uint32_t                 address,
                                     instructionDescriptor_t *descriptor,
                                     instructionForm_e        instructionForm)
{
    char                 *literalArgument    = NULL;
    label_t              *label              = NULL;
    requiredLabel_t      *requiredLabel      = NULL;
    resolutionInstance_t *resolutionInstance = NULL;
    uint32_t              literalValue32     = 0;
    uint16_t              literalValue16     = 0;
    uint8_t               literalValue8      = 0;
    bool                  wasFloat           = false;

    if (NULL == tokens     ||
        NULL == descriptor ||
        1    >  tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (formArgCount[instructionForm] != tokens->tokenCount - 1)
    {
        printf("%s:%u: expected %hhu arguments for \"%s\"\n",
               inputFileName, lineNumber, tokens->tokenCount, tokens->tokens[0]);
        return false;
    }

    if (0 == instructionForm % 4)
    {
        // Base form, no argument
        return true;
    }

    literalArgument = tokens->tokens[tokens->tokenCount - 1];

    label = getLabel(currentLabelList, literalArgument);

    if (NULL == label)
    {
        if (false == parseLiteral(literalArgument, &literalValue32, &wasFloat))
        {
            if (false == link)
            {
                printf("%s:%u: could not resolve argument \"%s\"\n", inputFileName, lineNumber, literalArgument);
                return false;
            }
            
            requiredLabel = getRequiredLabel(currentRequiredLabels, literalArgument);

            if (NULL == requiredLabel)
            {
                printf("%s:%u: could not resolve argument \"%s\"\n", inputFileName, lineNumber, literalArgument);
                return false;
            }

            // Make sure we're using a word sub-form
            if (1 != instructionForm % 4)
            {
                INTERNAL_ERROR;
                return false;
            }

            // TODO support absolute addresses
            resolutionInstance = addNewResolutionInstance(&(requiredLabel->instances));

            resolutionInstance->instructionOffset = address;

            // Injection offset is the same as the base instruction size without
            // an instruction argument. The / 4 * 4 snaps us back to the base
            resolutionInstance->injectionOffset =
                    formInstructionSize[(instructionForm / 4) * 4];
        }
    }
    else
    {
        if (false == descriptor->labelsAllowed)
        {
            printf("%s:%u: cannot use a label as an argument for \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
            return false;
        }

        // Make sure we're using a word sub-form
        if (1 != instructionForm % 4)
        {
            INTERNAL_ERROR;
            return false;
        }
        // TODO support absolute addresses
        literalValue32 = label->address - address;
    }

    switch (instructionForm % 4)
    {
        case 1:
        {
            // Word sub-form
            if (1 != fwrite(&literalValue32, 4, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
            break;
        }
        case 2:
        {
            // Short sub-form
            if (wasFloat)
            {
                INTERNAL_ERROR;
                return false;
            }
            if (literalValue32 > 0xFFFF &&
                literalValue32 < 0xFFFF8000)
            {
                printf("%s:%u: value too large for \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
                return false;
            }

            literalValue16 = literalValue32 & 0xFFFF;

            if (1 != fwrite(&literalValue16, 2, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
            break;
        }
        case 3:
        {
            // Character sub-form
            if (wasFloat)
            {
                INTERNAL_ERROR;
                return false;
            }
            if (literalValue8 > 0xFF &&
                literalValue8 < 0xFFFFFF80)
            {
                printf("%s:%u: value too large for \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);
                return false;
            }

            literalValue8 = literalValue32 & 0xFF;

            if (1 != fwrite(&literalValue8, 1, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
            break;
        }
    }

    return true;
}

static bool parseInstruction(tokens_t *tokens, uint32_t *address)
{
    instructionDescriptor_t *descriptorInstance = NULL;
    instructionForm_e        instructionForm    = 0;
    int                      rc                 = 0;
    uint8_t                  opCode             = 0;

    if (NULL == tokens ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return false;
    }

    for (uint16_t i = 0; i < sizeof(instructionDescriptors) / sizeof(instructionDescriptor_t); i++)
    {
        descriptorInstance = &(instructionDescriptors[i]);

        if (0 == strcmp(tokens->tokens[0], descriptorInstance->instructionStr) ||
            (descriptorInstance->useWildcards &&
             simpleRegex(descriptorInstance->instructionStr, tokens->tokens[0])))
        {
            if (NULL == descriptorInstance->formResolverFunction)
            {
                // TODO
                INTERNAL_ERROR;
                return false;
            }
            else
            {
                instructionForm = descriptorInstance->formResolverFunction(tokens);
                if (INS_FORM_ERR == instructionForm)
                {
                    return false;
                }

                if (NULL == descriptorInstance->opCodeResolver)
                {
                    opCode = descriptorInstance->opCode;
                }
                else
                {
                    rc = descriptorInstance->opCodeResolver(tokens, instructionForm);

                    if (-1 == rc)
                    {
                        return false;
                    }

                    opCode = rc;
                }
            }

            if (1 != fwrite(&opCode, 1, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }

            if (false == parseRegselAndFlagConditions(tokens, instructionForm) ||
                false == parseInstructionArgument(tokens, *address, descriptorInstance, instructionForm))
            {
                return false;
            }

            *address += formInstructionSize[instructionForm];

            return true;
        }
    }

    printf("%s:%u: unknown instruction \"%s\"\n", inputFileName, lineNumber, tokens->tokens[0]);

    return false;
}

static bool firstPass()
{
    char     inputBuffer[2048] = {0};
    uint32_t address           = 0;
    int      rc                = 0;
    tokens_t tokens            = {0};

    lineNumber = 0;
    currentSection = NULL;

    if (false == link)
    {
        currentSection = addNewSection(&sectionList);

        currentLabelList       = &(currentSection->labelList);
        currentExportedLabels  = &(currentSection->exportedLabels);
        currentRequiredLabels  = &(currentSection->requiredLabels);
        currentOriginRelatives = &(currentSection->originRelatives);
    }

    while (fgets(inputBuffer, 2048, inputFile))
    {
        lineNumber++;

        freeTokensContents(&tokens);

        if (false == parseTokens(inputBuffer, &tokens))
        {
            INTERNAL_ERROR;
            return false;
        }

        if (0 == tokens.tokenCount)
        {
            continue;
        }

        if (':' == tokens.tokens[0][0])
        {
            // Is label
            if (tokens.tokenCount > 1)
            {
                printf("%s:%u: Labels take no arguments\n", inputFileName, lineNumber);
                return false;
            }

            if (1 == strlen(tokens.tokens[0]))
            {
                printf("%s:%u: Label requires a name\n", inputFileName, lineNumber);
                return false;
            }

            if (false == parseLabel(&(tokens.tokens[0][1]), address))
            {
                freeTokensContents(&tokens);
                return false;
            }
            continue;
        }

        if ('.' == tokens.tokens[0][0])
        {
            // Is a dot directive
            if (false == parseDotDirectiveFirstPass(&tokens, &address))
            {
                freeTokensContents(&tokens);
                return false;
            }

            continue;
        }

        resolveAliases(&aliasList, &tokens);

        if (NULL == currentSection)
        {
            printf("%s:%u instruction not in section\n", inputFileName, lineNumber);
            return false;
        }

        rc = getInstructionSize(&tokens);

        if (-1 == rc)
        {
            // Error logged by getInstructionSize
            freeTokensContents(&tokens);
            return false;
        }

        address += rc;
    }    

    freeTokensContents(&tokens);

    return true;
}

static bool secondPass()
{
    char     inputBuffer[2048] = {0};
    tokens_t tokens            = {0};
    uint32_t address           = 0;

    lineNumber = 0;

    freeAliasListContents(&aliasList);

    if (link)
    {
        if (NULL == currentSection)
        {
            // empty file
            return true;
        }

        currentSection = sectionList.first;

        for (currentSection = sectionList.first;
             NULL != currentSection;
             currentSection = currentSection->next)
        {
            if (1 != fwrite(currentSection->name,
                            strlen(currentSection->name) + 1,
                            1,
                            outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }

            // Reserve space for pointers
            if (4 != fwrite(inputBuffer, 4, 4, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
        }

        inputBuffer[0] = '\0';
        if (1 != fwrite(inputBuffer, 1, 1, outputFile))
        {
            printf("Error writing to \"%s\"\n", outputFileName);
            return false;
        }

        currentSection = NULL;
    }

    rewind(inputFile);

    while (fgets(inputBuffer, 2048, inputFile))
    {
        lineNumber++;

        freeTokensContents(&tokens);

        if (false == parseTokens(inputBuffer, &tokens))
        {
            INTERNAL_ERROR;
            return false;
        }

        if (0 == tokens.tokenCount)
        {
            continue;
        }

        if (':' == tokens.tokens[0][0])
        {
            // Is label
            continue;
        }

        if ('.' == tokens.tokens[0][0])
        {
            // Is dot directive
            if (false == parseDotDirectiveSecondPass(&tokens, &address))
            {
                freeTokensContents(&tokens);
                return false;
            }

            continue;
        }

        resolveAliases(&aliasList, &tokens);

        if (false == parseInstruction(&tokens, &address))
        {
            freeTokensContents(&tokens);
            return false;
        }
    }

    if (link)
    {
        completeSection();

        rewind(outputFile);

        for (currentSection = sectionList.first;
             NULL != currentSection;
             currentSection = currentSection->next)
        {
            if (0 != fseek(outputFile, strlen(currentSection->name) + 1, SEEK_CUR) ||
                1 != fwrite(&(currentSection->codeSegmentOffset),    4, 1, outputFile) ||
                1 != fwrite(&(currentSection->exportedLabelsOffset), 4, 1, outputFile) ||
                1 != fwrite(&(currentSection->requiredLabelsOffset), 4, 1, outputFile) ||
                1 != fwrite(&(currentSection->originRelativeOffset), 4, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileName);
                return false;
            }
        }
    }

    freeTokensContents(&tokens);
    return true;
}

static void teardown()
{
    freeSectionListContents(&sectionList);
    freeAliasListContents(&aliasList);
    currentSection = NULL;
}

bool dssembler(char *in, char *out, bool linkedMode)
{
    if (NULL == in ||
        NULL == out)
    {
        INTERNAL_ERROR;
        return false;
    }

    link = linkedMode;

    inputFileName  = in;
    outputFileName = out;

    inputFile = fopen(inputFileName, "r");

    if (NULL == inputFile)
    {
        printf("Could not open input file \"%s\"\n", inputFileName);
        return false;
    }

    outputFile = fopen(outputFileName, "wb");

    if (NULL == outputFile)
    {
        printf("Could not create output file \"%s\"\n", outputFileName);
        fclose(inputFile);
        return false;
    }

    if (false == firstPass() ||
        false == secondPass())
    {
        teardown();
        fclose(inputFile);
        fclose(outputFile);
        remove(outputFileName);
        return false;
    }

    teardown();
    fclose(inputFile);
    fclose(outputFile);

    return true;
}