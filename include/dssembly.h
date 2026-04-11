#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKENS 8
#define COMMENT_PREFIX "//"
#define OP_CODE_MAX_LEN 32

typedef enum
{
    form_1 = 0,
    form_2 = 1,
    form_3 = 2,
    form_4 = 3,
    form_5 = 4,
    form_6 = 5,
    form_7 = 6
} form_e;

typedef struct instruction_t
{
    uint8_t        opCode;
    uint8_t        instructionAugment;
    char          *label;
    uint8_t        regsel1;
    uint8_t        regsel2;
    uint8_t        regsel3;
    uint32_t       immediate;
    struct instruction_t *argument;
    struct instruction_t *next;
} instruction_t;

typedef struct
{
    instruction_t *first;
    instruction_t *last;
} instructionList_t;

typedef struct
{
    char    opCodeStr[OP_CODE_MAX_LEN];
    uint8_t opCodePrimaryVal;
    form_e  primaryForm;

    bool    hasAlternateForm;
    uint8_t opCodeAlternateVal;
    form_e  alternateForm;

    bool    hasInstructionAugment;
    uint8_t instructionAugment;

    bool    alternateFormUsesArgAugment;
} instructionDescriptor_t;

typedef struct
{
    char   *tokens[MAX_TOKENS];
    uint8_t tokenCount;
} tokens_t;

// Util functions
instruction_t *addNewInstruction(instructionList_t *instructionList);

void freeInstructionListContents(instructionList_t *instructionList);

void freeTokensContents(tokens_t *tokens);

bool parseTokens(char* inputBuffer, tokens_t *tokens);
