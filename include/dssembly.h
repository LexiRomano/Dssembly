#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKENS 8
#define COMMENT_PREFIX "//"

typedef enum
{
    form_1,
    form_2,
    form_3,
    form_4,
    form_5,
    form_6,
    form_7
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
    char *tokens[MAX_TOKENS];
} tokens_t;

// Util functions
instruction_t *addNewInstruction(instructionList_t *instructionList);

void freeInstructionListContents(instructionList_t *instructionList);

void freeTokensContents(tokens_t *tokens);

bool parseTokens(char* inputBuffer, tokens_t *tokens);
