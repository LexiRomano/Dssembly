#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define TEMP_DIR_NAME "dTemp"
#define DINKER_CONFIG_SOURCE ".source"
#define DINKER_CONFIG_OUT    ".out"
#define DINKER_DEFAULT_OUT   "out.bin"
#define DINKER_OBJECT_EXT    ".dob"

#define MAX_TOKENS 8
#define COMMENT_PREFIX "//"
#define ALIAS_STR      ".alias"
#define INCLUDE_STR    ".include"
#define RESERVE_STR    ".reserve"
#define SET_STR        ".set"
#define SECTION_STR    ".section"
#define EXPORT_STR     ".export"
#define REQUIRES_STR   ".requires"
#define OP_CODE_MAX_LEN 32

#define OP_CODE_OFFSET 24
#define REGSEL_1_OFFSET 20
#define REGSEL_2_OFFSET 16
#define REGSEL_3_OFFSET 12

#define INTERNAL_ERROR printf("Internal error: %s:%d\n", __FUNCTION__, __LINE__)

bool dssembler(char *inputFileName, char *outputFileName, bool linkedMode);

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

typedef struct resolutionInstance_t
{
    uint32_t instructionOffset;
    uint8_t  injectionOffset;
    struct resolutionInstance_t *next;
} resolutionInstance_t;

typedef struct
{
    resolutionInstance_t *first;
    resolutionInstance_t *last;
    uint16_t              count;
} resolutionInstanceList_t;

typedef struct requiredLabel_t
{
    char                    *name;
    resolutionInstanceList_t instances;
    struct requiredLabel_t  *next;
} requiredLabel_t;

typedef struct
{
    requiredLabel_t *first;
    requiredLabel_t *last;
} requiredLabelList_t;

typedef struct section_t
{
    char               *name;
    char               *source;
    uint32_t            codeSegmentOffset;
    uint32_t            exportedLabelsOffset;
    uint32_t            requiredLabelsOffset;
    labelList_t         labelList;
    void               *codeSegment;
    labelList_t         exportedLabels;
    requiredLabelList_t requiredLabels;
    struct section_t *next;
} section_t;

typedef struct
{
    section_t *first;
    section_t *last;
} sectionList_t;

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
    bool    takesLabel;
} instructionDescriptor_t;

typedef struct
{
    char   *tokens[MAX_TOKENS];
    uint8_t tokenCount;
} tokens_t;

typedef struct
{
    char *sourceName;
    char *objectName;
} inputFile_t;

typedef struct
{
    inputFile_t *inputFiles;
    uint8_t      inputFileCount;

    char        *outputFile;

    char       **sections;
    uint8_t      sectionCount;
} dinkerConfig_t;

bool dinker(dinkerConfig_t *config);

// Util functions

label_t *addNewLabel          (labelList_t *labelList);
label_t *getLabel             (labelList_t *labelList, char *name);
void     freeLabelListContents(labelList_t *labelList);

resolutionInstance_t *addNewResolutionInstance          (resolutionInstanceList_t *instances);
void                  freeResolutionInstanceListContents(resolutionInstanceList_t *instances);

requiredLabel_t *addNewRequiredLabel  (requiredLabelList_t *requiredLabelList);
requiredLabel_t *getRequiredLabel     (requiredLabelList_t *requiredLabelList, char *name);
void             freeRequiredLabelList(requiredLabelList_t *requiredLabelList);

section_t *addNewSection          (sectionList_t *sectionList);
section_t *getSection             (sectionList_t *sectionList, char *name);
void       freeSectionListContents(sectionList_t *sectionList);

alias_t *addNewAlias          (aliasList_t *aliasList);
alias_t *getAlias             (aliasList_t *aliasList, char *name);
void     freeAliasListContents(aliasList_t *aliasList);

void freeTokensContents(tokens_t *tokens);
bool parseTokens       (char* inputBuffer, tokens_t *tokens);

bool isAlphanumericString(char *input);

void freeDinkerConfigContents(dinkerConfig_t *config);
