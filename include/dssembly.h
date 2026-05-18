#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKENS 8
#define COMMENT_PREFIX "//"
#define ALIAS_STR      ".alias"
#define RESERVE_STR    ".reserve"
#define SET_STR        ".set"
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


typedef struct label_t
{
    char           *label;
    uint32_t        address;
    struct label_t *next;
} label_t;

typedef struct
{
    label_t *first;
    label_t *last;
} labelList_t;

typedef struct alias_t
{
    char           *alias;
    uint32_t        value;
    struct alias_t *next;
} alias_t;

typedef struct
{
    alias_t *first;
    alias_t *last;
} aliasList_t;

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

label_t *addNewLabel          (labelList_t *labelList);
label_t *getLabel             (labelList_t *labelList, char *name);
void     freeLabelListContents(labelList_t *labelList);

alias_t *addNewAlias          (aliasList_t *aliasList);
alias_t *getAlias             (aliasList_t *aliasList, char *name);
void     freeAliasListContents(aliasList_t *aliasList);

void freeTokensContents(tokens_t *tokens);
bool parseTokens       (char* inputBuffer, tokens_t *tokens);

bool isAlphanumericString(char *input);
