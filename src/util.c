#include "dssembly.h"

label_t *addNewLabel(labelList_t *labelList)
{
    if (NULL == labelList)
    {
        return NULL;
    }

    if (NULL == labelList->first)
    {
        labelList->first = calloc(1, sizeof(label_t));
        labelList->last = labelList->first;

        return labelList->last;
    }

    if (NULL == labelList->last)
    {
        return NULL;
    }

    labelList->last->next = calloc(1, sizeof(label_t));
    labelList->last = labelList->last->next;

    return labelList->last;
}

label_t *getLabel(labelList_t *labelList, char *name)
{
    if (NULL == name ||
        NULL == labelList)
    {
        return NULL;
    }

    for (label_t *l = labelList->first; l != NULL; l = l->next)
    {
        if (NULL == l->label)
        {
            return NULL;
        }

        if (0 == strcmp(l->label, name))
        {
            return l;
        }
    }

    return NULL;
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

alias_t *addNewAlias(aliasList_t *aliasList)
{
    if (NULL == aliasList)
    {
        return NULL;
    }

    if (NULL == aliasList->first)
    {
        aliasList->first = calloc(1, sizeof(alias_t));
        aliasList->last = aliasList->first;

        return aliasList->last;
    }

    if (NULL == aliasList->last)
    {
        return NULL;
    }

    aliasList->last->next = calloc(1, sizeof(alias_t));
    aliasList->last = aliasList->last->next;

    return aliasList->last;
}

alias_t *getAlias(aliasList_t *aliasList, char *name)
{
    if (NULL == name ||
        NULL == aliasList)
    {
        return NULL;
    }

    for (alias_t *a = aliasList->first; a != NULL; a = a->next)
    {
        if (NULL == a->alias)
        {
            return NULL;
        }

        if (0 == strcmp(a->alias, name))
        {
            return a;
        }
    }

    return NULL;
}

void freeAliasListContents(aliasList_t *aliasList)
{
    alias_t *current = NULL;
    alias_t *next    = NULL;

    if (NULL == aliasList)
    {
        return;
    }

    current = aliasList->first;
    while (NULL != current)
    {
        if (NULL != current->alias)
        {
            free(current->alias);
        }

        next = current->next;
        free(current);
        current = next;
    }

    aliasList->first = NULL;
    aliasList->last  = NULL;
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

bool isAlphanumericString(char *input)
{
    char     c = '0';
    uint32_t i = -1;

    c = input[0];

    if (c >= '0' && c <= '9')
    {
        return false;
    }

    while (true)
    {
        i++;
        c = input[i];

        if (c == '\0')
        {
            return true;
        }

        if (c >= 'a' && c <= 'z')
        {
            continue;
        }

        if (c >= 'A' && c <= 'Z')
        {
            continue;
        }

        if (c >= '0' && c <= '9')
        {
            continue;
        }

        if (c == '_')
        {
            continue;
        }

        return false;
    }
}
