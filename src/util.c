#include "dssembly.h"

instruction_t *addNewInstruction(instructionList_t *instructionList)
{
    if (NULL == instructionList)
    {
        return NULL;
    }

    if (NULL == instructionList->first)
    {
        instructionList->first = calloc(1, sizeof(instruction_t));
        instructionList->last = instructionList->first;

        return instructionList->last;
    }

    if (NULL == instructionList->last)
    {
        return NULL;
    }

    instructionList->last->next = calloc(1, sizeof(instruction_t));
    instructionList->last = instructionList->last->next;

    return instructionList->last;
}

void freeInstructionListContents(instructionList_t *instructionList)
{
    instruction_t *current = NULL;
    instruction_t *next    = NULL;

    if (NULL == instructionList)
    {
        return;
    }

    current = instructionList->first;
    while (NULL != current)
    {
        if (NULL != current->targetLabel)
        {
            free(current->targetLabel);
        }

        next = current->next;
        free(current);
        current = next;
    }

    instructionList->first = NULL;
    instructionList->last  = NULL;
}

label_t *addNewLabel(labelList_t *labelList)
{
    if (NULL == labelList)
    {
        return NULL;
    }

    if (NULL == labelList->first)
    {
        labelList->first = calloc(1, sizeof(labelList_t));
        labelList->last = labelList->first;

        return labelList->last;
    }

    if (NULL == labelList->last)
    {
        return NULL;
    }

    labelList->last->next = calloc(1, sizeof(labelList_t));
    labelList->last = labelList->last->next;

    return labelList->last;
}

void freeLabelListContents(labelList_t *labelList)
{
    label_t *current = NULL;
    label_t *next    = NULL;

    if (NULL == labelList)
    {
        return;
    }

    current = labelList->first;
    while (NULL != current)
    {
        if (NULL != current->label)
        {
            free(current->label);
        }

        next = current->next;
        free(current);
        current = next;
    }

    labelList->first = NULL;
    labelList->last  = NULL;
}

void freeTokensContents(tokens_t *tokens)
{
    if (NULL == tokens)
    {
        return;
    }

    for (uint8_t i = 0; i < MAX_TOKENS; i++)
    {
        if (NULL != tokens->tokens[i])
        {
            free(tokens->tokens[i]);
            tokens->tokens[i] = NULL;
        }
    }

    tokens->tokenCount = 0;
}

static void removeCommentTokens(tokens_t *tokens)
{
    bool hitComment = false;

    if (NULL == tokens)
    {
        return;
    }

    for (uint8_t i = 0; i < MAX_TOKENS; i++)
    {
        if (NULL == tokens->tokens[i])
        {
            return;
        }

        if (true == hitComment)
        {
            free(tokens->tokens[i]);
            tokens->tokens[i] = NULL;
            continue;
        }

        if (0 == strncmp(tokens->tokens[i], COMMENT_PREFIX, strlen(COMMENT_PREFIX)))
        {
            hitComment = true;
            free(tokens->tokens[i]);
            tokens->tokens[i] = NULL;
            tokens->tokenCount = i;
        }
    }
}

bool parseTokens(char* inputBuffer, tokens_t *tokens)
{
    char     current      = '\0';
    char     last         = '\0';
    uint16_t startIndex   = 0;
    uint16_t currentIndex = 0;
    uint8_t  numTokens    = 0;

    if (NULL == tokens ||
        NULL == inputBuffer)
    {
        return false;
    }

    freeTokensContents(tokens);

    if (0 == strlen(inputBuffer))
    {
        return true;
    }

    current = inputBuffer[currentIndex];
    while (numTokens < MAX_TOKENS)
    {
        if ('\0' == last)
        {
            if ('\0' == current ||
                '\n' == current)
            {
                break;
            }

            if (' ' != current)
            {
                startIndex = currentIndex;
            }
        }
        else
        {
            if ((' ' == current ||
                '\0' == current ||
                '\n' == current) &&
                ' '  != last)
            {
                // Found the end of a token.
                tokens->tokens[numTokens] = calloc(currentIndex - startIndex + 1, sizeof(char));
                snprintf(tokens->tokens[numTokens], currentIndex - startIndex + 1, "%s", &(inputBuffer[startIndex]));
                numTokens++;
            }
            else if (' ' != current &&
                     ' ' == last)
            {
                // Found the beginning of a token.
                startIndex = currentIndex;
            }
        }

        if ('\0' == current ||
            '\n' == current)
        {
            break;
        }

        last = current;
        current = inputBuffer[++currentIndex];
    }

    tokens->tokenCount = numTokens;

    removeCommentTokens(tokens);

    return true;
}
