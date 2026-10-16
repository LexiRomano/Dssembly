#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define TEMP_DIR_NAME "dTemp"
#define DINKER_CONFIG_SOURCE ".source"
#define DINKER_CONFIG_OBJECT ".object"
#define DINKER_CONFIG_OUT    ".out"
#define DINKER_DEFAULT_OUT   "out.bin"
#define DINKER_OBJECT_EXT    ".dob"

#define MAX_TOKENS 8
#define COMMENT_PREFIX "//"
#define DEFINE_STR     ".define"
#define UNDEF_STR      ".undef"
#define INCLUDE_STR    ".include"
#define RESERVE_STR    ".reserve"
#define SET_STR        ".set"
#define SECTION_STR    ".section"
#define EXPORT_STR     ".export"
#define REQUIRES_STR   ".requires"

#define INTERNAL_ERROR printf("Internal error: %s:%d\n", __FUNCTION__, __LINE__)

bool dssembler(char *inputFileName, char *outputFileName, bool linkedMode);

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

typedef struct originRelative_t
{
    uint32_t                 address;
    struct originRelative_t *next;
} originRelative_t;

typedef struct
{
    originRelative_t *first;
    originRelative_t *last;
    uint16_t          count;
} originRelativeList_t;

typedef struct section_t
{
    char                *name;
    char                *source;
    uint32_t             codeSegmentOffset;
    uint32_t             exportedLabelsOffset;
    uint32_t             requiredLabelsOffset;
    uint32_t             originRelativeOffset;
    labelList_t          labelList;
    void                *codeSegment;
    labelList_t          exportedLabels;
    requiredLabelList_t  requiredLabels;
    originRelativeList_t originRelatives;
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
    char           *expansion;
    struct alias_t *next;
} alias_t;

typedef struct
{
    alias_t *first;
    alias_t *last;
} aliasList_t;

typedef enum
{
    INS_FORM_ERR = -1,
    INS_FORM_1   = 0,
    INS_FORM_1W  = 1,
    INS_FORM_1H  = 2,
    INS_FORM_1C  = 3,
    INS_FORM_2   = 4,
    INS_FORM_2W  = 5,
    INS_FORM_2H  = 6,
    INS_FORM_2C  = 7,
    INS_FORM_3   = 8,
    INS_FORM_3W  = 9,
    INS_FORM_3H  = 10,
    INS_FORM_3C  = 11,
    INS_FORM_4   = 12,
    INS_FORM_4W  = 13,
    INS_FORM_4H  = 14,
    INS_FORM_4C  = 15,
    INS_FORM_5   = 16,
    INS_FORM_5W  = 17,
    INS_FORM_5H  = 18,
    INS_FORM_5C  = 19,
    INS_FORM_6   = 20,
    INS_FORM_6W  = 21,
    INS_FORM_6H  = 22,
    INS_FORM_6C  = 23,
    INS_FORM_7   = 24,
    INS_FORM_7W  = 25,
    INS_FORM_7H  = 26,
    INS_FORM_7C  = 27,
    INS_FORM_8   = 28,
    INS_FORM_8W  = 29,
    INS_FORM_8H  = 30,
    INS_FORM_8C  = 31
} instructionForm_e;

typedef enum
{
    FLAG_AL = 0,
    FLAG_EQ = 1,
    FLAG_NE = 2,
    FLAG_HI = 3,
    FLAG_HS = 4,
    FLAG_LS = 5,
    FLAG_LO = 6,
    FLAG_GT = 7,
    FLAG_GE = 8,
    FLAG_LE = 9,
    FLAG_LT = 10,
    FLAG_MI = 11,
    FLAG_PZ = 12,
    FLAG_OV = 13,
    FLAG_NC = 14
} flagCondition_e;

typedef struct
{
    char   *tokens[MAX_TOKENS];
    uint8_t tokenCount;
} tokens_t;

typedef instructionForm_e (*formResolver_t)         (tokens_t*);
typedef int               (*opCodeResolver_t)       (tokens_t*,
                                                     instructionForm_e);
typedef flagCondition_e   (*flagConditionResolver_t)(char *);

#define INSTRUCTION_STR_MAX_LEN 16
typedef struct
{
    char                    instructionStr[INSTRUCTION_STR_MAX_LEN];
    bool                    useWildcards;
    instructionForm_e       form;
    formResolver_t          formResolverFunction;
    uint8_t                 opCode;
    opCodeResolver_t        opCodeResolver;
    flagConditionResolver_t flagConditionResolver;
    bool                    labelsAllowed;
} instructionDescriptor_t;

typedef enum
{
    LWM_W  = 0,
    LWM_H  = 1,
    LWM_SH = 2,
    LWM_C  = 3,
    LWM_SC = 4
} loadWidthModifiers_e;

typedef enum
{
    SWM_W  = 0,
    SWM_H  = 1,
    SWM_C  = 2,
} storWidthModifiers_e;

typedef enum
{
    ADDRESSING_MODE_AB = 0,
    ADDRESSING_MODE_PC = 1,
    ADDRESSING_MODE_OA = 2,
    ADDRESSING_MODE_OB = 3,
    ADDRESSING_MODE_OC = 4
} addressingMode_e;

typedef enum
{
    MATH_ADDX,
    MATH_SUBX,
    MATH_MULX,
    MATH_DIVX,
    MATH_MOD,
    MATH_AND,
    MATH_OR,
    MATH_XOR,
    MATH_NOT,
    MATH_BSLT,
    MATH_BSLC,
    MATH_BSRT,
    MATH_BSRC,
    MATH_FTOI,
    MATH_UITOF,
    MATH_SITOF
} mathOperations_e;

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

originRelative_t *addNewOriginRelative  (originRelativeList_t *originRelativeList);
void              freeOriginRelativeList(originRelativeList_t *originRelativeList);

section_t *addNewSection          (sectionList_t *sectionList);
section_t *getSection             (sectionList_t *sectionList, char *name);
void       freeSectionListContents(sectionList_t *sectionList);

alias_t *addNewAlias          (aliasList_t *aliasList);
alias_t *getAlias             (aliasList_t *aliasList, char *name);
void     removeAlias          (aliasList_t *aliasList, char *name);
void     resolveAliases       (aliasList_t *aliasList, tokens_t *tokens);
void     freeAliasListContents(aliasList_t *aliasList);

void freeTokensContents(tokens_t *tokens);
bool parseTokens       (char* inputBuffer, tokens_t *tokens);

bool isAlphanumericString(char *input);

void freeDinkerConfigContents(dinkerConfig_t *config);

bool simpleRegex(char *template, char *target);
