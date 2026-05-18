#include "dssembly.h"

static FILE *inputFile          = NULL;
static char *outputFileLocation = NULL;

static labelList_t labelList = {0};
static aliasList_t aliasList = {0};

static instructionDescriptor_t instructionDescriptors[] =
{// instructionStr 
//                    opCodePrimaryVal 
//                          primaryForm 
//                                  hasAlternateForm 
//                                         opCodeAlternateVal 
//                                               alternateForm 
//                                                       hasInstructionAugment 
//                                                              instructionAugment 
//                                                                    alternateFormUsesArgAugment 
//                                                                           takesAddress
    {"MOVE",          0x06, form_2, true,  0x07, form_5, false, 0,    false, false},
    {"ADD",           0x18, form_1, true,  0x19, form_3, false, 0,    false, false},
    {"SUB",           0x28, form_1, true,  0x29, form_3, false, 0,    false, false},
    {"AND",           0x38, form_1, true,  0x39, form_3, false, 0,    false, false},
    {"OR",            0x48, form_1, true,  0x49, form_3, false, 0,    false, false},
    {"XOR",           0x58, form_1, true,  0x59, form_3, false, 0,    false, false},
    {"NOT",           0x68, form_2, false, 0   , 0,      false, 0,    false, false},
    {"BSLT",          0x78, form_1, true,  0x79, form_3, false, 0,    false, false},
    {"BSRT",          0x88, form_1, true,  0x89, form_3, false, 0,    false, false},
    {"BSLC",          0x98, form_1, true,  0x99, form_3, false, 0,    false, false},
    {"BSRC",          0xA8, form_1, true,  0xA9, form_3, false, 0,    false, false},
    {"COMP",          0x0A, form_2, true,  0x0B, form_4, false, 0,    true,  false},
    {"PUSH",          0x0E, form_4, false, 0,    0,      false, 0,    false, false},
    {"POP",           0x1E, form_4, false, 0,    0,      false, 0,    false, false},
    {"PUSHALL",       0x2E, form_6, false, 0,    0,      false, 0,    false, false},
    {"POPALL",        0x3E, form_6, false, 0,    0,      false, 0,    false, false},
    {"PEEK",          0x4E, form_4, false, 0,    0,      false, 0,    false, false},
    {"RETURN",        0x5E, form_6, false, 0,    0,      false, 0,    false, false},
    {"INTSUS",        0x01, form_6, false, 0,    0,      false, 0,    false, false},
    {"INTRES",        0x11, form_6, false, 0,    0,      false, 0,    false, false},
    {"INTTRG",        0x21, form_4, true,  0x31, form_7, false, 0,    false, false},
    {"INTFIN",        0x41, form_6, false, 0,    0,      false, 0,    false, false},
    {"INTGPR",        0x51, form_4, false, 0,    0,      false, 0,    false, false},
    {"LOAD",          0x02, form_2, true,  0x03, form_4, true,  0x80, true,  true},
    {"LOAD-ABS",      0x02, form_2, true,  0x03, form_4, true,  0x00, true,  false},
    {"LOAD-OA",       0x02, form_2, true,  0x03, form_4, true,  0x01, true,  false},
    {"LOAD-OB",       0x02, form_2, true,  0x03, form_4, true,  0x02, true,  false},
    {"LOAD-OC",       0x02, form_2, true,  0x03, form_4, true,  0x03, true,  false},
    {"LOAD-W2S0",     0x02, form_2, true,  0x03, form_4, true,  0x84, true,  true},
    {"LOAD-W2S0-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x04, true,  false},
    {"LOAD-W2S0-OA",  0x02, form_2, true,  0x03, form_4, true,  0x05, true,  false},
    {"LOAD-W2S0-OB",  0x02, form_2, true,  0x03, form_4, true,  0x06, true,  false},
    {"LOAD-W2S0-OC",  0x02, form_2, true,  0x03, form_4, true,  0x07, true,  false},
    {"LOAD-W2S1",     0x02, form_2, true,  0x03, form_4, true,  0x94, true,  true},
    {"LOAD-W2S1-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x14, true,  false},
    {"LOAD-W2S1-OA",  0x02, form_2, true,  0x03, form_4, true,  0x15, true,  false},
    {"LOAD-W2S1-OB",  0x02, form_2, true,  0x03, form_4, true,  0x16, true,  false},
    {"LOAD-W2S1-OC",  0x02, form_2, true,  0x03, form_4, true,  0x17, true,  false},
    {"LOAD-W2S2",     0x02, form_2, true,  0x03, form_4, true,  0xA4, true,  true},
    {"LOAD-W2S2-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x24, true,  false},
    {"LOAD-W2S2-OA",  0x02, form_2, true,  0x03, form_4, true,  0x25, true,  false},
    {"LOAD-W2S2-OB",  0x02, form_2, true,  0x03, form_4, true,  0x26, true,  false},
    {"LOAD-W2S2-OC",  0x02, form_2, true,  0x03, form_4, true,  0x27, true,  false},
    {"LOAD-W1S0",     0x02, form_2, true,  0x03, form_4, true,  0x88, true,  true},
    {"LOAD-W1S0-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x08, true,  false},
    {"LOAD-W1S0-OA",  0x02, form_2, true,  0x03, form_4, true,  0x09, true,  false},
    {"LOAD-W1S0-OB",  0x02, form_2, true,  0x03, form_4, true,  0x0A, true,  false},
    {"LOAD-W1S0-OC",  0x02, form_2, true,  0x03, form_4, true,  0x0B, true,  false},
    {"LOAD-W1S1",     0x02, form_2, true,  0x03, form_4, true,  0x98, true,  true},
    {"LOAD-W1S1-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x18, true,  false},
    {"LOAD-W1S1-OA",  0x02, form_2, true,  0x03, form_4, true,  0x19, true,  false},
    {"LOAD-W1S1-OB",  0x02, form_2, true,  0x03, form_4, true,  0x1A, true,  false},
    {"LOAD-W1S1-OC",  0x02, form_2, true,  0x03, form_4, true,  0x1B, true,  false},
    {"LOAD-W1S2",     0x02, form_2, true,  0x03, form_4, true,  0xA8, true,  true},
    {"LOAD-W1S2-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x28, true,  false},
    {"LOAD-W1S2-OA",  0x02, form_2, true,  0x03, form_4, true,  0x29, true,  false},
    {"LOAD-W1S2-OB",  0x02, form_2, true,  0x03, form_4, true,  0x2A, true,  false},
    {"LOAD-W1S2-OC",  0x02, form_2, true,  0x03, form_4, true,  0x2B, true,  false},
    {"LOAD-W1S3",     0x02, form_2, true,  0x03, form_4, true,  0xB8, true,  true},
    {"LOAD-W1S3-ABS", 0x02, form_2, true,  0x03, form_4, true,  0x38, true,  false},
    {"LOAD-W1S3-OA",  0x02, form_2, true,  0x03, form_4, true,  0x39, true,  false},
    {"LOAD-W1S3-OB",  0x02, form_2, true,  0x03, form_4, true,  0x3A, true,  false},
    {"LOAD-W1S3-OC",  0x02, form_2, true,  0x03, form_4, true,  0x3B, true,  false},
    {"STOR",          0x04, form_2, true,  0x05, form_4, true,  0x80, true,  true},
    {"STOR-ABS",      0x04, form_2, true,  0x05, form_4, true,  0x00, true,  false},
    {"STOR-OA",       0x04, form_2, true,  0x05, form_4, true,  0x01, true,  false},
    {"STOR-OB",       0x04, form_2, true,  0x05, form_4, true,  0x02, true,  false},
    {"STOR-OC",       0x04, form_2, true,  0x05, form_4, true,  0x03, true,  false},
    {"STOR-W2S0",     0x04, form_2, true,  0x05, form_4, true,  0x84, true,  true},
    {"STOR-W2S0-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x04, true,  false},
    {"STOR-W2S0-OA",  0x04, form_2, true,  0x05, form_4, true,  0x05, true,  false},
    {"STOR-W2S0-OB",  0x04, form_2, true,  0x05, form_4, true,  0x06, true,  false},
    {"STOR-W2S0-OC",  0x04, form_2, true,  0x05, form_4, true,  0x07, true,  false},
    {"STOR-W2S1",     0x04, form_2, true,  0x05, form_4, true,  0x94, true,  true},
    {"STOR-W2S1-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x14, true,  false},
    {"STOR-W2S1-OA",  0x04, form_2, true,  0x05, form_4, true,  0x15, true,  false},
    {"STOR-W2S1-OB",  0x04, form_2, true,  0x05, form_4, true,  0x16, true,  false},
    {"STOR-W2S1-OC",  0x04, form_2, true,  0x05, form_4, true,  0x17, true,  false},
    {"STOR-W2S2",     0x04, form_2, true,  0x05, form_4, true,  0xA4, true,  true},
    {"STOR-W2S2-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x24, true,  false},
    {"STOR-W2S2-OA",  0x04, form_2, true,  0x05, form_4, true,  0x25, true,  false},
    {"STOR-W2S2-OB",  0x04, form_2, true,  0x05, form_4, true,  0x26, true,  false},
    {"STOR-W2S2-OC",  0x04, form_2, true,  0x05, form_4, true,  0x27, true,  false},
    {"STOR-W1S0",     0x04, form_2, true,  0x05, form_4, true,  0x88, true,  true},
    {"STOR-W1S0-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x08, true,  false},
    {"STOR-W1S0-OA",  0x04, form_2, true,  0x05, form_4, true,  0x09, true,  false},
    {"STOR-W1S0-OB",  0x04, form_2, true,  0x05, form_4, true,  0x0A, true,  false},
    {"STOR-W1S0-OC",  0x04, form_2, true,  0x05, form_4, true,  0x0B, true,  false},
    {"STOR-W1S1",     0x04, form_2, true,  0x05, form_4, true,  0x98, true,  true},
    {"STOR-W1S1-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x18, true,  false},
    {"STOR-W1S1-OA",  0x04, form_2, true,  0x05, form_4, true,  0x19, true,  false},
    {"STOR-W1S1-OB",  0x04, form_2, true,  0x05, form_4, true,  0x1A, true,  false},
    {"STOR-W1S1-OC",  0x04, form_2, true,  0x05, form_4, true,  0x1B, true,  false},
    {"STOR-W1S2",     0x04, form_2, true,  0x05, form_4, true,  0xA8, true,  true},
    {"STOR-W1S2-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x28, true,  false},
    {"STOR-W1S2-OA",  0x04, form_2, true,  0x05, form_4, true,  0x29, true,  false},
    {"STOR-W1S2-OB",  0x04, form_2, true,  0x05, form_4, true,  0x2A, true,  false},
    {"STOR-W1S2-OC",  0x04, form_2, true,  0x05, form_4, true,  0x2B, true,  false},
    {"STOR-W1S3",     0x04, form_2, true,  0x05, form_4, true,  0xB8, true,  true},
    {"STOR-W1S3-ABS", 0x04, form_2, true,  0x05, form_4, true,  0x38, true,  false},
    {"STOR-W1S3-OA",  0x04, form_2, true,  0x05, form_4, true,  0x39, true,  false},
    {"STOR-W1S3-OB",  0x04, form_2, true,  0x05, form_4, true,  0x3A, true,  false},
    {"STOR-W1S3-OC",  0x04, form_2, true,  0x05, form_4, true,  0x3B, true,  false},
    {"GETABS",        0x02, form_2, true,  0x03, form_4, true,  0xC0, true,  true},
    {"GETABS-OA",     0x02, form_2, true,  0x03, form_4, true,  0x41, true,  false},
    {"GETABS-OB",     0x02, form_2, true,  0x03, form_4, true,  0x42, true,  false},
    {"GETABS-OC",     0x02, form_2, true,  0x03, form_4, true,  0x43, true,  false},
    {"BRAL",          0x0C, form_4, true,  0x0D, form_6, true,  0x80, true,  true},
    {"BRAL-ABS",      0x0C, form_4, true,  0x0D, form_6, true,  0x00, true,  false},
    {"BRAL-OA",       0x0C, form_4, true,  0x0D, form_6, true,  0x01, true,  false},
    {"BRAL-OB",       0x0C, form_4, true,  0x0D, form_6, true,  0x02, true,  false},
    {"BRAL-OC",       0x0C, form_4, true,  0x0D, form_6, true,  0x03, true,  false},
    {"BRAL-P",        0x0C, form_4, true,  0x0D, form_6, true,  0xC0, true,  true},
    {"BRAL-P-ABS",    0x0C, form_4, true,  0x0D, form_6, true,  0x40, true,  false},
    {"BRAL-P-OA",     0x0C, form_4, true,  0x0D, form_6, true,  0x41, true,  false},
    {"BRAL-P-OB",     0x0C, form_4, true,  0x0D, form_6, true,  0x42, true,  false},
    {"BRAL-P-OC",     0x0C, form_4, true,  0x0D, form_6, true,  0x43, true,  false},
    {"BREQ",          0x1C, form_4, true,  0x1D, form_6, true,  0x80, true,  true},
    {"BREQ-ABS",      0x1C, form_4, true,  0x1D, form_6, true,  0x00, true,  false},
    {"BREQ-OA",       0x1C, form_4, true,  0x1D, form_6, true,  0x01, true,  false},
    {"BREQ-OB",       0x1C, form_4, true,  0x1D, form_6, true,  0x02, true,  false},
    {"BREQ-OC",       0x1C, form_4, true,  0x1D, form_6, true,  0x03, true,  false},
    {"BREQ-P",        0x1C, form_4, true,  0x1D, form_6, true,  0xC0, true,  true},
    {"BREQ-P-ABS",    0x1C, form_4, true,  0x1D, form_6, true,  0x40, true,  false},
    {"BREQ-P-OA",     0x1C, form_4, true,  0x1D, form_6, true,  0x41, true,  false},
    {"BREQ-P-OB",     0x1C, form_4, true,  0x1D, form_6, true,  0x42, true,  false},
    {"BREQ-P-OC",     0x1C, form_4, true,  0x1D, form_6, true,  0x43, true,  false},
    {"BRNE",          0x2C, form_4, true,  0x2D, form_6, true,  0x80, true,  true},
    {"BRNE-ABS",      0x2C, form_4, true,  0x2D, form_6, true,  0x00, true,  false},
    {"BRNE-OA",       0x2C, form_4, true,  0x2D, form_6, true,  0x01, true,  false},
    {"BRNE-OB",       0x2C, form_4, true,  0x2D, form_6, true,  0x02, true,  false},
    {"BRNE-OC",       0x2C, form_4, true,  0x2D, form_6, true,  0x03, true,  false},
    {"BRNE-P",        0x2C, form_4, true,  0x2D, form_6, true,  0xC0, true,  true},
    {"BRNE-P-ABS",    0x2C, form_4, true,  0x2D, form_6, true,  0x40, true,  false},
    {"BRNE-P-OA",     0x2C, form_4, true,  0x2D, form_6, true,  0x41, true,  false},
    {"BRNE-P-OB",     0x2C, form_4, true,  0x2D, form_6, true,  0x42, true,  false},
    {"BRNE-P-OC",     0x2C, form_4, true,  0x2D, form_6, true,  0x43, true,  false},
    {"BRHI",          0x3C, form_4, true,  0x3D, form_6, true,  0x80, true,  true},
    {"BRHI-ABS",      0x3C, form_4, true,  0x3D, form_6, true,  0x00, true,  false},
    {"BRHI-OA",       0x3C, form_4, true,  0x3D, form_6, true,  0x01, true,  false},
    {"BRHI-OB",       0x3C, form_4, true,  0x3D, form_6, true,  0x02, true,  false},
    {"BRHI-OC",       0x3C, form_4, true,  0x3D, form_6, true,  0x03, true,  false},
    {"BRHI-P",        0x3C, form_4, true,  0x3D, form_6, true,  0xC0, true,  true},
    {"BRHI-P-ABS",    0x3C, form_4, true,  0x3D, form_6, true,  0x40, true,  false},
    {"BRHI-P-OA",     0x3C, form_4, true,  0x3D, form_6, true,  0x41, true,  false},
    {"BRHI-P-OB",     0x3C, form_4, true,  0x3D, form_6, true,  0x42, true,  false},
    {"BRHI-P-OC",     0x3C, form_4, true,  0x3D, form_6, true,  0x43, true,  false},
    {"BRHS",          0x4C, form_4, true,  0x4D, form_6, true,  0x80, true,  true},
    {"BRHS-ABS",      0x4C, form_4, true,  0x4D, form_6, true,  0x00, true,  false},
    {"BRHS-OA",       0x4C, form_4, true,  0x4D, form_6, true,  0x01, true,  false},
    {"BRHS-OB",       0x4C, form_4, true,  0x4D, form_6, true,  0x02, true,  false},
    {"BRHS-OC",       0x4C, form_4, true,  0x4D, form_6, true,  0x03, true,  false},
    {"BRHS-P",        0x4C, form_4, true,  0x4D, form_6, true,  0xC0, true,  true},
    {"BRHS-P-ABS",    0x4C, form_4, true,  0x4D, form_6, true,  0x40, true,  false},
    {"BRHS-P-OA",     0x4C, form_4, true,  0x4D, form_6, true,  0x41, true,  false},
    {"BRHS-P-OB",     0x4C, form_4, true,  0x4D, form_6, true,  0x42, true,  false},
    {"BRHS-P-OC",     0x4C, form_4, true,  0x4D, form_6, true,  0x43, true,  false},
    {"BRLS",          0x5C, form_4, true,  0x5D, form_6, true,  0x80, true,  true},
    {"BRLS-ABS",      0x5C, form_4, true,  0x5D, form_6, true,  0x00, true,  false},
    {"BRLS-OA",       0x5C, form_4, true,  0x5D, form_6, true,  0x01, true,  false},
    {"BRLS-OB",       0x5C, form_4, true,  0x5D, form_6, true,  0x02, true,  false},
    {"BRLS-OC",       0x5C, form_4, true,  0x5D, form_6, true,  0x03, true,  false},
    {"BRLS-P",        0x5C, form_4, true,  0x5D, form_6, true,  0xC0, true,  true},
    {"BRLS-P-ABS",    0x5C, form_4, true,  0x5D, form_6, true,  0x40, true,  false},
    {"BRLS-P-OA",     0x5C, form_4, true,  0x5D, form_6, true,  0x41, true,  false},
    {"BRLS-P-OB",     0x5C, form_4, true,  0x5D, form_6, true,  0x42, true,  false},
    {"BRLS-P-OC",     0x5C, form_4, true,  0x5D, form_6, true,  0x43, true,  false},
    {"BRLO",          0x6C, form_4, true,  0x6D, form_6, true,  0x80, true,  true},
    {"BRLO-ABS",      0x6C, form_4, true,  0x6D, form_6, true,  0x00, true,  false},
    {"BRLO-OA",       0x6C, form_4, true,  0x6D, form_6, true,  0x01, true,  false},
    {"BRLO-OB",       0x6C, form_4, true,  0x6D, form_6, true,  0x02, true,  false},
    {"BRLO-OC",       0x6C, form_4, true,  0x6D, form_6, true,  0x03, true,  false},
    {"BRLO-P",        0x6C, form_4, true,  0x6D, form_6, true,  0xC0, true,  true},
    {"BRLO-P-ABS",    0x6C, form_4, true,  0x6D, form_6, true,  0x40, true,  false},
    {"BRLO-P-OA",     0x6C, form_4, true,  0x6D, form_6, true,  0x41, true,  false},
    {"BRLO-P-OB",     0x6C, form_4, true,  0x6D, form_6, true,  0x42, true,  false},
    {"BRLO-P-OC",     0x6C, form_4, true,  0x6D, form_6, true,  0x43, true,  false},
    {"BRGT",          0x7C, form_4, true,  0x7D, form_6, true,  0x80, true,  true},
    {"BRGT-ABS",      0x7C, form_4, true,  0x7D, form_6, true,  0x00, true,  false},
    {"BRGT-OA",       0x7C, form_4, true,  0x7D, form_6, true,  0x01, true,  false},
    {"BRGT-OB",       0x7C, form_4, true,  0x7D, form_6, true,  0x02, true,  false},
    {"BRGT-OC",       0x7C, form_4, true,  0x7D, form_6, true,  0x03, true,  false},
    {"BRGT-P",        0x7C, form_4, true,  0x7D, form_6, true,  0xC0, true,  true},
    {"BRGT-P-ABS",    0x7C, form_4, true,  0x7D, form_6, true,  0x40, true,  false},
    {"BRGT-P-OA",     0x7C, form_4, true,  0x7D, form_6, true,  0x41, true,  false},
    {"BRGT-P-OB",     0x7C, form_4, true,  0x7D, form_6, true,  0x42, true,  false},
    {"BRGT-P-OC",     0x7C, form_4, true,  0x7D, form_6, true,  0x43, true,  false},
    {"BRGE",          0x8C, form_4, true,  0x8D, form_6, true,  0x80, true,  true},
    {"BRGE-ABS",      0x8C, form_4, true,  0x8D, form_6, true,  0x00, true,  false},
    {"BRGE-OA",       0x8C, form_4, true,  0x8D, form_6, true,  0x01, true,  false},
    {"BRGE-OB",       0x8C, form_4, true,  0x8D, form_6, true,  0x02, true,  false},
    {"BRGE-OC",       0x8C, form_4, true,  0x8D, form_6, true,  0x03, true,  false},
    {"BRGE-P",        0x8C, form_4, true,  0x8D, form_6, true,  0xC0, true,  true},
    {"BRGE-P-ABS",    0x8C, form_4, true,  0x8D, form_6, true,  0x40, true,  false},
    {"BRGE-P-OA",     0x8C, form_4, true,  0x8D, form_6, true,  0x41, true,  false},
    {"BRGE-P-OB",     0x8C, form_4, true,  0x8D, form_6, true,  0x42, true,  false},
    {"BRGE-P-OC",     0x8C, form_4, true,  0x8D, form_6, true,  0x43, true,  false},
    {"BRLE",          0x9C, form_4, true,  0x9D, form_6, true,  0x80, true,  true},
    {"BRLE-ABS",      0x9C, form_4, true,  0x9D, form_6, true,  0x00, true,  false},
    {"BRLE-OA",       0x9C, form_4, true,  0x9D, form_6, true,  0x01, true,  false},
    {"BRLE-OB",       0x9C, form_4, true,  0x9D, form_6, true,  0x02, true,  false},
    {"BRLE-OC",       0x9C, form_4, true,  0x9D, form_6, true,  0x03, true,  false},
    {"BRLE-P",        0x9C, form_4, true,  0x9D, form_6, true,  0xC0, true,  true},
    {"BRLE-P-ABS",    0x9C, form_4, true,  0x9D, form_6, true,  0x40, true,  false},
    {"BRLE-P-OA",     0x9C, form_4, true,  0x9D, form_6, true,  0x41, true,  false},
    {"BRLE-P-OB",     0x9C, form_4, true,  0x9D, form_6, true,  0x42, true,  false},
    {"BRLE-P-OC",     0x9C, form_4, true,  0x9D, form_6, true,  0x43, true,  false},
    {"BRLT",          0xAC, form_4, true,  0xAD, form_6, true,  0x80, true,  true},
    {"BRLT-ABS",      0xAC, form_4, true,  0xAD, form_6, true,  0x00, true,  false},
    {"BRLT-OA",       0xAC, form_4, true,  0xAD, form_6, true,  0x01, true,  false},
    {"BRLT-OB",       0xAC, form_4, true,  0xAD, form_6, true,  0x02, true,  false},
    {"BRLT-OC",       0xAC, form_4, true,  0xAD, form_6, true,  0x03, true,  false},
    {"BRLT-P",        0xAC, form_4, true,  0xAD, form_6, true,  0xC0, true,  true},
    {"BRLT-P-ABS",    0xAC, form_4, true,  0xAD, form_6, true,  0x40, true,  false},
    {"BRLT-P-OA",     0xAC, form_4, true,  0xAD, form_6, true,  0x41, true,  false},
    {"BRLT-P-OB",     0xAC, form_4, true,  0xAD, form_6, true,  0x42, true,  false},
    {"BRLT-P-OC",     0xAC, form_4, true,  0xAD, form_6, true,  0x43, true,  false},
    {"BRMI",          0xBC, form_4, true,  0xBD, form_6, true,  0x80, true,  true},
    {"BRMI-ABS",      0xBC, form_4, true,  0xBD, form_6, true,  0x00, true,  false},
    {"BRMI-OA",       0xBC, form_4, true,  0xBD, form_6, true,  0x01, true,  false},
    {"BRMI-OB",       0xBC, form_4, true,  0xBD, form_6, true,  0x02, true,  false},
    {"BRMI-OC",       0xBC, form_4, true,  0xBD, form_6, true,  0x03, true,  false},
    {"BRMI-P",        0xBC, form_4, true,  0xBD, form_6, true,  0xC0, true,  true},
    {"BRMI-P-ABS",    0xBC, form_4, true,  0xBD, form_6, true,  0x40, true,  false},
    {"BRMI-P-OA",     0xBC, form_4, true,  0xBD, form_6, true,  0x41, true,  false},
    {"BRMI-P-OB",     0xBC, form_4, true,  0xBD, form_6, true,  0x42, true,  false},
    {"BRMI-P-OC",     0xBC, form_4, true,  0xBD, form_6, true,  0x43, true,  false},
    {"BRPZ",          0xCC, form_4, true,  0xCD, form_6, true,  0x80, true,  true},
    {"BRPZ-ABS",      0xCC, form_4, true,  0xCD, form_6, true,  0x00, true,  false},
    {"BRPZ-OA",       0xCC, form_4, true,  0xCD, form_6, true,  0x01, true,  false},
    {"BRPZ-OB",       0xCC, form_4, true,  0xCD, form_6, true,  0x02, true,  false},
    {"BRPZ-OC",       0xCC, form_4, true,  0xCD, form_6, true,  0x03, true,  false},
    {"BRPZ-P",        0xCC, form_4, true,  0xCD, form_6, true,  0xC0, true,  true},
    {"BRPZ-P-ABS",    0xCC, form_4, true,  0xCD, form_6, true,  0x40, true,  false},
    {"BRPZ-P-OA",     0xCC, form_4, true,  0xCD, form_6, true,  0x41, true,  false},
    {"BRPZ-P-OB",     0xCC, form_4, true,  0xCD, form_6, true,  0x42, true,  false},
    {"BRPZ-P-OC",     0xCC, form_4, true,  0xCD, form_6, true,  0x43, true,  false},
    {"BROV",          0xDC, form_4, true,  0xDD, form_6, true,  0x80, true,  true},
    {"BROV-ABS",      0xDC, form_4, true,  0xDD, form_6, true,  0x00, true,  false},
    {"BROV-OA",       0xDC, form_4, true,  0xDD, form_6, true,  0x01, true,  false},
    {"BROV-OB",       0xDC, form_4, true,  0xDD, form_6, true,  0x02, true,  false},
    {"BROV-OC",       0xDC, form_4, true,  0xDD, form_6, true,  0x03, true,  false},
    {"BROV-P",        0xDC, form_4, true,  0xDD, form_6, true,  0xC0, true,  true},
    {"BROV-P-ABS",    0xDC, form_4, true,  0xDD, form_6, true,  0x40, true,  false},
    {"BROV-P-OA",     0xDC, form_4, true,  0xDD, form_6, true,  0x41, true,  false},
    {"BROV-P-OB",     0xDC, form_4, true,  0xDD, form_6, true,  0x42, true,  false},
    {"BROV-P-OC",     0xDC, form_4, true,  0xDD, form_6, true,  0x43, true,  false},
    {"BRNV",          0xEC, form_4, true,  0xED, form_6, true,  0x80, true,  true},
    {"BRNV-ABS",      0xEC, form_4, true,  0xED, form_6, true,  0x00, true,  false},
    {"BRNV-OA",       0xEC, form_4, true,  0xED, form_6, true,  0x01, true,  false},
    {"BRNV-OB",       0xEC, form_4, true,  0xED, form_6, true,  0x02, true,  false},
    {"BRNV-OC",       0xEC, form_4, true,  0xED, form_6, true,  0x03, true,  false},
    {"BRNV-P",        0xEC, form_4, true,  0xED, form_6, true,  0xC0, true,  true},
    {"BRNV-P-ABS",    0xEC, form_4, true,  0xED, form_6, true,  0x40, true,  false},
    {"BRNV-P-OA",     0xEC, form_4, true,  0xED, form_6, true,  0x41, true,  false},
    {"BRNV-P-OB",     0xEC, form_4, true,  0xED, form_6, true,  0x42, true,  false},
    {"BRNV-P-OC",     0xEC, form_4, true,  0xED, form_6, true,  0x43, true,  false},
    {"NOOP",          0x00, form_6, false, 0,    0,      false, 0,    false, false},
    {"TERM",          0xFF, form_6, false, 0,    0,      false, 0,    false, false}
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
    0xFFFFF,   // form_5
    0,         // form_6
    0xFFFFFF,  // form_7
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

static bool parseLiteral(char *literal, uint32_t *out)
{
    if (NULL == literal ||
        NULL == out)
    {
        return false;
    }

    if (strlen(literal) >= 3)
    {
        // Has a prefix
        if (0 == strncmp(literal, "0b", 2))
        {
            // Binary
            return parseBinary(&(literal[2]), out);

        }
        else if (0 == strncmp(literal, "0x", 2))
        {
            // Hexadecimal
            return parseHexadecimal(&(literal[2]), out);
        }
    }

    return parseDecimal(literal, out);;
}

static int getInstructionSize(tokens_t *tokens, uint32_t lineNumber)
{
    instructionDescriptor_t *descriptorInstance = NULL;
    bool                     isAlternateForm    = false;

    if (NULL == tokens             ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return false;
    }

    for (uint16_t i = 0; i < sizeof(instructionDescriptors) / sizeof(instructionDescriptor_t); i++)
    {
        descriptorInstance = &(instructionDescriptors[i]);

        if (0 == strcmp(tokens->tokens[0], descriptorInstance->instructionStr))
        {
            if (tokens->tokenCount - 1 != formArgCount[descriptorInstance->primaryForm])
            {
                if (tokens->tokenCount - 1 > formArgCount[descriptorInstance->primaryForm])
                {
                    printf("Error on line %u: too many arguments for instruction \"%s\"\n", lineNumber, tokens->tokens[0]);
                }
                else
                {
                    printf("Error on line %u: too few arguments for instruction \"%s\"\n", lineNumber, tokens->tokens[0]);
                }
                return -1;
            }

            // If the last argument is valid regsel, then we're not using alternate form.
            isAlternateForm = !isValidRegsel(tokens->tokens[tokens->tokenCount - 1]);

            if (false == isAlternateForm ||
                false == descriptorInstance->alternateFormUsesArgAugment)
            {
                if (false == descriptorInstance->hasInstructionAugment)
                {
                    return 4;
                }

                return 5;
            }

            if (false == descriptorInstance->hasInstructionAugment)
            {
                return 8;
            }

            return 9;
        }
    }

    printf("Error on line %u: Invalid instruction \"%s\"\n", lineNumber, tokens->tokens[0]);

    return -1;
}

static bool parseLabel(tokens_t *tokens, uint32_t lineNumber, uint32_t address)
{
    label_t *newLabel          = NULL;
    char    *labelWithoutColon = NULL;

    if (tokens->tokenCount > 1)
    {
        printf("Error on line %u: Labels take no arguments\n", lineNumber);
        return false;
    }

    if (1 == strlen(tokens->tokens[0]))
    {
        printf("Error on line %u: Label requires a name\n", lineNumber);
        return false;
    }

    labelWithoutColon = &(tokens->tokens[0][1]);

    if (true == isValidRegsel(labelWithoutColon))
    {
        printf("Error on line %u: Label name cannot be a regsel\n", lineNumber);
        return false;
    }

    if (false == isAlphanumericString(labelWithoutColon))
    {
        printf("Error on line %u: label \"%s\" contains invalid characters\n", lineNumber, labelWithoutColon);
        return false;
    }

    if (NULL != getLabel(&labelList, labelWithoutColon))
    {
        printf("Error on line %u: label \"%s\" already exists\n", lineNumber, labelWithoutColon);
        return false;
    }

    if (NULL != getAlias(&aliasList, labelWithoutColon))
    {
        printf("Error on line %u: label \"%s\" conflicts with alias of the same name\n", lineNumber, labelWithoutColon);
        return false;
    }

    newLabel = addNewLabel(&labelList);

    if (NULL == newLabel)
    {
        INTERNAL_ERROR;
        return false;
    }

    newLabel->label   = strcpy(calloc(strlen(labelWithoutColon) + 1, sizeof(char)), labelWithoutColon);
    newLabel->address = address;

    return true;
}

static bool parseAlias(tokens_t *tokens, uint32_t lineNumber)
{
    uint32_t buf32    = 0;
    alias_t *newAlias = NULL;

    if (tokens->tokenCount != 3)
    {
        printf("Error on line %u: alias requires two arguments\n", lineNumber);
        return false;
    }

    if (false == isAlphanumericString(tokens->tokens[1]))
    {
        printf("Error on line %u: alias \"%s\" contains invalid characters\n", lineNumber, tokens->tokens[1]);
        return false;
    }

    if (NULL != getAlias(&aliasList, tokens->tokens[1]))
    {
        printf("Error on line %u: alias \"%s\" already exists\n", lineNumber, tokens->tokens[1]);
        return false;
    }

    if (NULL != getLabel(&labelList, tokens->tokens[1]))
    {
        printf("Error on line %u: alias \"%s\" conflicts with label of the same name\n", lineNumber, tokens->tokens[1]);
        return false;
    }

    if (false == parseLiteral(tokens->tokens[2], &buf32))
    {
        printf("Error on line %u: could not parse literal \"%s\"\n", lineNumber, tokens->tokens[2]);
        return false;
    }

    newAlias = addNewAlias(&aliasList);

    if (NULL == newAlias)
    {
        INTERNAL_ERROR;
        return false;
    }

    newAlias->alias  = strcpy(calloc(strlen(tokens->tokens[1]) + 1, sizeof(char)), tokens->tokens[1]);
    newAlias->value = buf32;

    return true;
}

static int parseDotDirectiveFirstPass(tokens_t *tokens, uint32_t lineNumber)
{
    uint32_t buf32 = 0;

    if (NULL == tokens ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return -1;
    }

    if (0 == strcmp(tokens->tokens[0], ALIAS_STR))
    {
        if (false == parseAlias(tokens, lineNumber))
        {
            return -1;
        }
        return 0;
    }

    if (0 == strcmp(tokens->tokens[0], RESERVE_STR))
    {
        if (2 != tokens->tokenCount)
        {
            printf("Error on line %u: reserve directive takes one argument\n", lineNumber);
            return -1;
        }

        if (false == parseLiteral(tokens->tokens[1], &buf32))
        {
            printf("Error on line %u: could not parse literal \"%s\"\n", lineNumber, tokens->tokens[1]);
            return -1;
        }

        return buf32;
    }

    if (0 == strcmp(tokens->tokens[0], SET_STR))
    {
        if (3 != tokens->tokenCount)
        {
            printf("Error on line %u: set directive takes two argument\n", lineNumber);
            return -1;
        }

        if (1   == strlen(tokens->tokens[1]) &&
            '*' == tokens->tokens[1][0])
        {
            return strlen(tokens->tokens[2]) + 1;
        }

        if (false == parseLiteral(tokens->tokens[1], &buf32))
        {
            printf("Error on line %u: could not parse literal \"%s\"\n", lineNumber, tokens->tokens[1]);
            return -1;
        }

        return buf32;
    }

    printf("Error on line %u: invalid directive \"%s\"\n", lineNumber, tokens->tokens[0]);
    return -1;
}

static bool parseDotDirectiveSecondPass(tokens_t *tokens, uint32_t lineNumber, FILE *outputFile)
{
    uint32_t reserveSize = 0;
    uint32_t setValue    = 0;
    uint16_t buf16       = 0;
    uint8_t  buf8        = 0;
    int      rc          = 0;

    if (NULL == tokens             ||
        0    == tokens->tokenCount ||
        NULL == outputFile)
    {
        INTERNAL_ERROR;
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], ALIAS_STR))
    {
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], RESERVE_STR))
    {
        if (2 != tokens->tokenCount)
        {
            printf("Error on line %u: reserve directive takes one argument\n", lineNumber);
            return false;
        }

        if (false == parseLiteral(tokens->tokens[1], &reserveSize))
        {
            printf("Error on line %u: could not parse literal \"%s\"\n", lineNumber, tokens->tokens[1]);
            return false;
        }

        if (reserveSize != 0)
        {
            if (reserveSize > 1 &&
                0 != fseek(outputFile, reserveSize - 1, SEEK_CUR))
            {
                printf("Error writing to \"%s\"\n", outputFileLocation);
                return false;
            }

            if ('\0' != fputc('\0', outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileLocation);
                return false;
            }
        }
        
        return true;
    }

    if (0 == strcmp(tokens->tokens[0], SET_STR))
    {
        if (3 != tokens->tokenCount)
        {
            printf("Error on line %u: set directive takes two argument\n", lineNumber);
            return false;
        }

        if (1   == strlen(tokens->tokens[1]) &&
            '*' == tokens->tokens[1][0])
        {
            if (strlen(tokens->tokens[2]) + 1 !=  fwrite(tokens->tokens[2], sizeof(char), strlen(tokens->tokens[2]) + 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileLocation);
                return false;
            }
            return true;
        }

        if (false == parseLiteral(tokens->tokens[1], &reserveSize))
        {
            printf("Error on line %u: could not parse literal \"%s\"\n", lineNumber, tokens->tokens[1]);
            return false;
        }

        if (false == parseLiteral(tokens->tokens[2], &setValue))
        {
            printf("Error on line %u: could not parse literal \"%s\"\n", lineNumber, tokens->tokens[2]);
            return false;
        }

        if (reserveSize != 1 &&
            reserveSize != 2 &&
            reserveSize != 4)
        {
            printf("Error on line %u: sets of numeric literals must be 1, 2, or 4 bytes\n", lineNumber);
            return false;
        }

        if (setValue >= (((uint64_t) 0x100) << ((reserveSize - 1) * 8)))
        {
            printf("Error on line %u: %s cannot fit into %u bytes\n", lineNumber, tokens->tokens[2], reserveSize);
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
            printf("Error writing to \"%s\"\n", outputFileLocation);
            return false;
        }

        return true;
    }

    printf("Error on line %u: invalid directive \"%s\"\n", lineNumber, tokens->tokens[0]);
    return false;
}

static bool parseMainInstructionSegment(tokens_t                *tokens, 
                                        instructionDescriptor_t *descriptor,
                                        bool                    *requiresArgAug,
                                        uint32_t                 address,
                                        uint32_t                 lineNumber,
                                        FILE                    *outputFile)
{
    uint32_t outputBuf          = 0;
    uint8_t  minimumRegselCount = 0;
    bool     isAlternateForm    = false;
    form_e   form               = 0;
    char    *literalArgument    = NULL;
    label_t *label              = NULL;
    alias_t *alias              = NULL;
    uint32_t literalValue       = 0;

    if (NULL == tokens         ||
        NULL == descriptor     ||
        NULL == requiresArgAug ||
        NULL == outputFile)
    {
        INTERNAL_ERROR;
        return false;
    }

    if (tokens->tokenCount - 1 != formArgCount[descriptor->primaryForm])
    {
        if (tokens->tokenCount - 1 > formArgCount[descriptor->primaryForm])
        {
            printf("Error on line %u: too many arguments for instruction \"%s\"\n", lineNumber, tokens->tokens[0]);
        }
        else
        {
            printf("Error on line %u: too few arguments for instruction \"%s\"\n", lineNumber, tokens->tokens[0]);
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
                printf("Error on line %u: argument #3 for \"%s\" must be a regsel\n", lineNumber, tokens->tokens[0]);
                return false;
            }

            outputBuf |= ((uint32_t) getRegsel(tokens->tokens[3])) << REGSEL_3_OFFSET;

        case 2:
            if (false == isValidRegsel(tokens->tokens[2]))
            {
                printf("Error on line %u: argument #2 for \"%s\" must be a regsel\n", lineNumber, tokens->tokens[0]);
                return false;
            }

            outputBuf |= ((uint32_t) getRegsel(tokens->tokens[2])) << REGSEL_2_OFFSET;

        case 1:
            if (false == isValidRegsel(tokens->tokens[1]))
            {
                printf("Error on line %u: argument #1 for \"%s\" must be a regsel\n", lineNumber, tokens->tokens[0]);
                return false;
            }

            outputBuf |= ((uint32_t) getRegsel(tokens->tokens[1])) << REGSEL_1_OFFSET;

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
                    outputBuf |= ((uint32_t) getRegsel(tokens->tokens[3])) << REGSEL_3_OFFSET;
                    isAlternateForm = false;
                }
                break;
            
            case 2:
                if (true == isValidRegsel(tokens->tokens[2]))
                {
                    outputBuf |= ((uint32_t) getRegsel(tokens->tokens[2])) << REGSEL_2_OFFSET;
                    isAlternateForm = false;
                }
                break;

            case 1:
                if (true == isValidRegsel(tokens->tokens[1]))
                {
                    outputBuf |= ((uint32_t) getRegsel(tokens->tokens[1])) << REGSEL_1_OFFSET;
                    isAlternateForm = false;
                }
                break;
        }
    }

    if (isAlternateForm)
    {
        outputBuf      |= descriptor->opCodeAlternateVal << OP_CODE_OFFSET;
        *requiresArgAug = descriptor->alternateFormUsesArgAugment;
        form            = descriptor->alternateForm;
    }
    else
    {
        outputBuf      |= descriptor->opCodePrimaryVal << OP_CODE_OFFSET;
        *requiresArgAug = false;
        form            = descriptor->primaryForm;
    }

    if (formArgCount[form] != formRegselCount[form])
    {
        literalArgument = tokens->tokens[tokens->tokenCount - 1];

        label = getLabel(&labelList, literalArgument);

        if (NULL == label)
        {
            alias = getAlias(&aliasList, literalArgument);

            if (NULL == alias)
            {
                if (false == parseLiteral(literalArgument, &literalValue))
                {
                    printf("Error on line %u: could not resolve argument \"%s\"\n", lineNumber, literalArgument);
                    return false;
                }

            }
            else
            {
                literalValue = alias->value;
            }
        }
        else
        {
            literalValue = label->address - address;
        }

        if (literalValue > formMaxArgSize[form])
        {
            printf("Error on line %u: \"%s\" does not fit as an argument for \"%s\"\n",
                   lineNumber, literalArgument, tokens->tokens[0]);
            return false;
        }

        outputBuf |= literalValue;
    }

    if (1 != fwrite(&outputBuf, 4, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileLocation);
        return false;
    }

    return true;
}

static bool parseArgumentAugment(tokens_t *tokens, uint32_t address, uint32_t lineNumber, FILE *outputFile)
{
    char    *literalArgument = NULL;
    label_t *label           = 0;
    alias_t *alias           = 0;
    uint32_t literalValue    = 0;

    literalArgument = tokens->tokens[tokens->tokenCount - 1];

    label = getLabel(&labelList, literalArgument);

    if (NULL == label)
    {
        alias = getAlias(&aliasList, literalArgument);

        if (NULL == alias)
        {
            if (false == parseLiteral(literalArgument, &literalValue))
            {
                printf("Error on line %u: could not resolve argument \"%s\"\n", lineNumber, literalArgument);
                return false;
            }

        }
        else
        {
            literalValue = alias->value;
        }
    }
    else
    {
        literalValue = label->address - address;
    }

    if (1 != fwrite(&literalValue, 4, 1, outputFile))
    {
        printf("Error writing to \"%s\"\n", outputFileLocation);
        return false;
    }

    return true;
}

static bool parseInstruction(tokens_t *tokens, uint32_t lineNumber, FILE *outputFile)
{
    instructionDescriptor_t *descriptorInstance = NULL;
    uint32_t                 address            = 0;
    bool                     requiresArgAugment = 0;

    if (NULL == tokens ||
        0    == tokens->tokenCount)
    {
        INTERNAL_ERROR;
        return false;
    }

    for (uint16_t i = 0; i < sizeof(instructionDescriptors) / sizeof(instructionDescriptor_t); i++)
    {
        descriptorInstance = &(instructionDescriptors[i]);

        if (0 == strcmp(tokens->tokens[0], descriptorInstance->instructionStr))
        {
            address = ftell(outputFile);

            if (false == parseMainInstructionSegment(tokens,
                                                     descriptorInstance, 
                                                     &requiresArgAugment,
                                                     address,
                                                     lineNumber,
                                                     outputFile))
            {
                return false;
            }

            if (descriptorInstance->hasInstructionAugment &&
                1 != fwrite(&(descriptorInstance->instructionAugment), 1, 1, outputFile))
            {
                printf("Error writing to \"%s\"\n", outputFileLocation);
                return false;
            }

            if (requiresArgAugment &&
                false == parseArgumentAugment(tokens, address, lineNumber, outputFile))
            {
                return false;
            }

            return true;
        }
    }

    printf("Error on line %u: unknown instruction \"%s\"\n", lineNumber, tokens->tokens[0]);

    return false;
}

static bool firstPass()
{
    char     inputBuffer[2048] = {0};
    uint32_t lineNumber        = 0;
    uint32_t address           = 0;
    int      rc                = 0;
    tokens_t tokens            = {0};

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
            if (false == parseLabel(&tokens, lineNumber, address))
            {
                freeTokensContents(&tokens);
                return false;
            }
            continue;
        }

        if ('.' == tokens.tokens[0][0])
        {
            // Is a dot directive
            rc = parseDotDirectiveFirstPass(&tokens, lineNumber);
            
            if (-1 == rc)
            {
                freeTokensContents(&tokens);
                return false;
            }

            address += rc;

            continue;
        }

        rc = getInstructionSize(&tokens, lineNumber);

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
    FILE    *outputFile        = NULL;
    char     inputBuffer[2048] = {0};
    tokens_t tokens            = {0};
    uint32_t lineNumber        = 0;

    outputFile = fopen(outputFileLocation, "wb");

    if (NULL == outputFile)
    {
        printf("Could not write to output file \"%s\"\n", outputFileLocation);
        return false;
    }

    rewind(inputFile);

    while (fgets(inputBuffer, 2048, inputFile))
    {
        lineNumber++;

        freeTokensContents(&tokens);

        if (false == parseTokens(inputBuffer, &tokens))
        {
            INTERNAL_ERROR;
            fclose(outputFile);
            remove(outputFileLocation);
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
            if (false == parseDotDirectiveSecondPass(&tokens, lineNumber, outputFile))
            {
                fclose(outputFile);
                remove(outputFileLocation);
                freeTokensContents(&tokens);
                return false;
            }

            continue;
        }

        if (false == parseInstruction(&tokens, lineNumber, outputFile))
        {
            fclose(outputFile);
            remove(outputFileLocation);
            freeTokensContents(&tokens);
            return false;
        }
    }

    fclose(outputFile);
    freeTokensContents(&tokens);
    return true;
}

static void teardown()
{
    freeLabelListContents(&labelList);

    if (NULL != inputFile)
    {
        fclose(inputFile);
        inputFile = NULL;
    }
}

int main(int argc, char* argv[])
{
    printf("Dssembly\n");

    if (false == parseArgs(argc, argv))
    {
        return -1;
    }

    if (false == firstPass() ||
        false == secondPass())
    {
        teardown();
        return -1;
    }

    teardown();

    printf("Success!\n");

    return 0;
}