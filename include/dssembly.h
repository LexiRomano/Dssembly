#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKENS 8
#define COMMENT_PREFIX "//"
#define OP_CODE_MAX_LEN 32

#define OP_CODE_OFFSET 24
#define REGSEL_1_OFFSET 20
#define REGSEL_2_OFFSET 16
#define REGSEL_3_OFFSET 12

#define INTERNAL_ERROR printf("Internal error: %s:%d\n", __FUNCTION__, __LINE__)

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
    uint8_t               opCode;
    bool                  hasInstructionAugment;
    uint8_t               instructionAugment;
    uint8_t               regselCount;
    uint8_t               regsel1;
    uint8_t               regsel2;
    uint8_t               regsel3;
    bool                  hasArgAugment;
    uint32_t              lineNumber;
    uint32_t              address;
    uint32_t              immediate;
    char                 *targetLabel;
    struct instruction_t *targetLabel_p;
    struct instruction_t *next;
} instruction_t;

typedef struct
{
    instruction_t *first;
    instruction_t *last;
} instructionList_t;

typedef struct label_t
{
    char           *label;
    instruction_t  *instruction;
    struct label_t *next;
} label_t;

typedef struct
{
    label_t *first;
    label_t *last;
} labelList_t;

typedef struct
{
    char    instructionStr[OP_CODE_MAX_LEN];
    uint8_t opCodePrimaryVal;
    form_e  primaryForm;

    bool    hasAlternateForm;
    uint8_t opCodeAlternateVal;
    form_e  alternateForm;

    bool    hasInstructionAugment;
    uint8_t instructionAugment;

    bool    alternateFormUsesArgAugment;
    bool    takesAddress;
} instructionDescriptor_t;

typedef struct
{
    char   *tokens[MAX_TOKENS];
    uint8_t tokenCount;
} tokens_t;

// Util functions
instruction_t *addNewInstruction(instructionList_t *instructionList);

void freeInstructionListContents(instructionList_t *instructionList);

label_t *addNewLabel(labelList_t *labelList);

void freeLabelListContents(labelList_t *labelList);

void freeTokensContents(tokens_t *tokens);

bool parseTokens(char* inputBuffer, tokens_t *tokens);
