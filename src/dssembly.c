#include "dssembly.h"

static FILE *inputFile          = NULL;
static char *outputFileLocation = NULL;

static instructionList_t instructionList = {0};
static labelList_t       labelList       = {0};

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
    {"PUSH",          0x0E, form_4, false, 0,    0,      0,     0,    false, false},
    {"POP",           0x1E, form_4, false, 0,    0,      0,     0,    false, false},
    {"PUSHALL",       0x2E, form_6, false, 0,    0,      0,     0,    false, false},
    {"POPALL",        0x3E, form_6, false, 0,    0,      0,     0,    false, false},
    {"PEEK",          0x4E, form_4, false, 0,    0,      0,     0,    false, false},
    {"RETURN",        0x5E, form_6, false, 0,    0,      0,     0,    false, false},
    {"INTSUS",        0x01, form_6, false, 0,    0,      0,     0,    false, false},
    {"INTRES",        0x11, form_6, false, 0,    0,      0,     0,    false, false},
    {"INTTRG",        0x21, form_4, true,  0x31, form_7, 0,     0,    false, false},
    {"INTFIN",        0x41, form_6, false, 0,    0,      0,     0,    false, false},
    {"INTGPR",        0x51, form_4, false, 0,    0,      0,     0,    false, false},
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

    instruction->regselCount = minimumRegselCount;

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
                    instruction->regselCount = 3;
                    isAlternateForm = false;
                }
                break;
            
            case 2:
                if (true == isValidRegsel(tokens->tokens[2]))
                {
                    instruction->regsel2 = getRegsel(tokens->tokens[2]);
                    instruction->regselCount = 2;
                    isAlternateForm = false;
                }
                break;

            case 1:
                if (true == isValidRegsel(tokens->tokens[1]))
                {
                    instruction->regsel1 = getRegsel(tokens->tokens[1]);
                    instruction->regselCount = 1;
                    isAlternateForm = false;
                }
                break;
        }
    }

    if (isAlternateForm)
    {
        instruction->opCode        = descriptor->opCodeAlternateVal;
        instruction->hasArgAugment = descriptor->alternateFormUsesArgAugment;
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

static bool outputToFile()
{
    uint32_t address    = 0;
    uint32_t buf32      = 0;
    FILE    *outputFile = NULL;

    outputFile = fopen(outputFileLocation, "wb");

    if (NULL == outputFile)
    {
        printf("Could not open file \"%s\"\n", outputFileLocation);
        freeInstructionListContents(&instructionList);
        freeLabelListContents(&labelList);
        return false;
    }

    for (instruction_t *i = instructionList.first; i != NULL; i = i->next)
    {
        // Preparing the main instruction
        buf32 = 0;
        buf32 += i->opCode << 24;
        switch (i->regselCount)
        {
            case 3:
                buf32 += i->regsel3 << REGSEL_3_OFFSET;
            case 2:
                buf32 += i->regsel2 << REGSEL_2_OFFSET;
            case 1:
                buf32 += i->regsel1 << REGSEL_1_OFFSET;
        }

        if (false == i->hasArgAugment)
        {
            buf32 += i->immediate;
        }

        // Seek if required
        if (address != i->address &&
            0 != fseek(outputFile, address, SEEK_SET))
        {
            INTERNAL_ERROR;
            freeInstructionListContents(&instructionList);
            freeLabelListContents(&labelList);
            fclose(outputFile);
            return false;
        }

        // Print main instruction
        if (1 != fwrite(&buf32, sizeof(buf32), 1, outputFile))
        {
            INTERNAL_ERROR;
            freeInstructionListContents(&instructionList);
            freeLabelListContents(&labelList);
            fclose(outputFile);
            return false;
        }
        address += 4;

        // Print instruction augment
        if (true == i->hasInstructionAugment)
        {
            if (1 != fwrite(&(i->instructionAugment), sizeof(uint8_t), 1, outputFile))
            {
                INTERNAL_ERROR;
                freeInstructionListContents(&instructionList);
                freeLabelListContents(&labelList);
                fclose(outputFile);
                return false;
            }
            address += 1;
        }

        // Print arg augment
        if (true == i->hasArgAugment)
        {
            if (NULL == i->targetLabel_p)
            {
                buf32 = i->immediate;
            }
            else
            {
                buf32 = i->targetLabel_p->address - i->address;
            }

            if (1 != fwrite(&buf32, sizeof(buf32), 1, outputFile))
            {
                INTERNAL_ERROR;
                freeInstructionListContents(&instructionList);
                freeLabelListContents(&labelList);
                fclose(outputFile);
                return false;
            }
            address += 4;
        }
    }

    fclose(outputFile);

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

    if (false == outputToFile())
    {
        return -1;
    }

    freeInstructionListContents(&instructionList);
    freeLabelListContents(&labelList);

    printf("Success!\n");

    return 0;
}