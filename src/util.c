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

resolutionInstance_t *addNewResolutionInstance(resolutionInstanceList_t *instances)
{
    if (NULL == instances)
    {
        return NULL;
    }

    if (NULL == instances->first)
    {
        instances->first = calloc(1, sizeof(resolutionInstance_t));
        instances->last = instances->first;

        instances->count++;

        return instances->last;
    }

    if (NULL == instances->last)
    {
        return NULL;
    }

    instances->last->next = calloc(1, sizeof(resolutionInstance_t));
    instances->last = instances->last->next;

    instances->count++;

    return instances->last;
}

void freeResolutionInstanceListContents(resolutionInstanceList_t *instances)
{
    resolutionInstance_t *current = NULL;
    resolutionInstance_t *next    = NULL;

    if (NULL == instances)
    {
        return;
    }

    current = instances->first;
    while (NULL != current)
    {
        next = current->next;
        free(current);
        current = next;
    }

    instances->first = NULL;
    instances->last  = NULL;
    instances->count = 0;
}

requiredLabel_t *addNewRequiredLabel(requiredLabelList_t *requiredLabelList)
{
    if (NULL == requiredLabelList)
    {
        return NULL;
    }

    if (NULL == requiredLabelList->first)
    {
        requiredLabelList->first = calloc(1, sizeof(requiredLabel_t));
        requiredLabelList->last = requiredLabelList->first;

        return requiredLabelList->last;
    }

    if (NULL == requiredLabelList->last)
    {
        return NULL;
    }

    requiredLabelList->last->next = calloc(1, sizeof(requiredLabel_t));
    requiredLabelList->last = requiredLabelList->last->next;

    return requiredLabelList->last;
}

requiredLabel_t *getRequiredLabel(requiredLabelList_t *requiredLabelList, char *name)
{
    if (NULL == name ||
        NULL == requiredLabelList)
    {
        return NULL;
    }

    for (requiredLabel_t *r = requiredLabelList->first; r != NULL; r = r->next)
    {
        if (NULL == r->name)
        {
            return NULL;
        }

        if (0 == strcmp(r->name, name))
        {
            return r;
        }
    }

    return NULL;
}

void freeRequiredLabelList(requiredLabelList_t *requiredLabelList)
{
    requiredLabel_t *current = NULL;
    requiredLabel_t *next    = NULL;

    if (NULL == requiredLabelList)
    {
        return;
    }

    current = requiredLabelList->first;
    while (NULL != current)
    {
        freeResolutionInstanceListContents(&(current->instances));

        next = current->next;
        free(current);
        current = next;
    }

    requiredLabelList->first = NULL;
    requiredLabelList->last  = NULL;
}

originRelative_t *addNewOriginRelative(originRelativeList_t *originRelativeList)
{
    if (NULL == originRelativeList)
    {
        return NULL;
    }

    if (NULL == originRelativeList->first)
    {
        originRelativeList->first = calloc(1, sizeof(originRelative_t));
        originRelativeList->last = originRelativeList->first;

        originRelativeList->count = 1;
        return originRelativeList->last;
    }

    if (NULL == originRelativeList->last)
    {
        return NULL;
    }

    originRelativeList->last->next = calloc(1, sizeof(originRelative_t));
    originRelativeList->last = originRelativeList->last->next;
    originRelativeList->count++;

    return originRelativeList->last;
}

void freeOriginRelativeList(originRelativeList_t *originRelativeList)
{
    originRelative_t *current = NULL;
    originRelative_t *next    = NULL;

    if (NULL == originRelativeList)
    {
        return;
    }

    current = originRelativeList->first;
    while (NULL != current)
    {
        next = current->next;
        free(current);
        current = next;
    }

    originRelativeList->first = NULL;
    originRelativeList->last  = NULL;
    originRelativeList->count = 0;
}

section_t *addNewSection(sectionList_t *sectionList)
{
    if (NULL == sectionList)
    {
        return NULL;
    }

    if (NULL == sectionList->first)
    {
        sectionList->first = calloc(1, sizeof(section_t));
        sectionList->last = sectionList->first;

        return sectionList->last;
    }

    if (NULL == sectionList->last)
    {
        return NULL;
    }

    sectionList->last->next = calloc(1, sizeof(section_t));
    sectionList->last = sectionList->last->next;

    return sectionList->last;
}

section_t *getSection(sectionList_t *sectionList, char *name)
{
    if (NULL == name ||
        NULL == sectionList)
    {
        return NULL;
    }

    for (section_t *s = sectionList->first; s != NULL; s = s->next)
    {
        if (NULL == s->name)
        {
            return NULL;
        }

        if (0 == strcmp(s->name, name))
        {
            return s;
        }
    }

    return NULL;
}

void freeSectionListContents(sectionList_t *sectionList)
{
    section_t *current = NULL;
    section_t *next    = NULL;

    if (NULL == sectionList)
    {
        return;
    }

    current = sectionList->first;
    while (NULL != current)
    {
        if (NULL != current->name)
        {
            free(current->name);
            current->name = NULL;
        }

        if (NULL != current->source)
        {
            free(current->source);
            current->source = NULL;
        }

        freeLabelListContents(&(current->labelList));

        next = current->next;
        free(current);
        current = next;
    }

    sectionList->first = NULL;
    sectionList->last  = NULL;
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
    bool     isInQuote    = false;

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
            if (((isInQuote ? false : ' ' == current) ||
                 '"' == current ||
                '\0' == current ||
                '\n' == current) &&
                (isInQuote ? true : ' ' != last))
            {
                // Found the end of a token.
                tokens->tokens[numTokens] = calloc(currentIndex - startIndex + 1, sizeof(char));
                snprintf(tokens->tokens[numTokens], currentIndex - startIndex + 1, "%s", &(inputBuffer[startIndex]));
                numTokens++;

                if (isInQuote)
                {
                    // Bump it along one more so we don't pick up another quote
                    last = current;
                    current = inputBuffer[++currentIndex];
                    isInQuote = false;
                }
            }
            else if ((isInQuote ? false : ' ' != current) &&
                     ' ' == last)
            {
                // Found the beginning of a token.
                if ('"' == current)
                {
                    isInQuote = true;
                    startIndex = currentIndex + 1;
                }
                else
                {
                    startIndex = currentIndex;
                }
            }
        }

        if ('\0' == current ||
            '\n' == current)
        {
            break;
        }

        last = current;
        if (last == '\\')
        {
            // Make sure we don't interpret a space or quote
            current = 'z';
            currentIndex++;
        }
        else
        {
            current = inputBuffer[++currentIndex];
        }
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

void freeDinkerConfigContents(dinkerConfig_t *config)
{
    if (NULL == config)
    {
        return;
    }

    if (NULL != config->inputFiles &&
        0    != config->inputFileCount)
    {
        for (uint8_t i = 0; i < config->inputFileCount; i++)
        {
            if (NULL != config->inputFiles[i].sourceName)
            {
                free(config->inputFiles[i].sourceName);
                config->inputFiles[i].sourceName = NULL;
            }

            if (NULL != config->inputFiles[i].objectName)
            {
                free(config->inputFiles[i].objectName);
                config->inputFiles[i].objectName = NULL;
            }
        }

        free(config->inputFiles);
        config->inputFiles     = NULL;
        config->inputFileCount = 0;
    }

    if (NULL != config->outputFile)
    {
        free(config->outputFile);
        config->outputFile = NULL;
    }

    if (NULL != config->sections &&
        0    != config->sectionCount)
    {
        for (uint8_t i = 0; i < config->sectionCount; i++)
        {
            if (NULL != config->sections[i])
            {
                free(config->sections[i]);
                config->sections[i] = NULL;
            }
        }

        free(config->sections);
        config->sections     = NULL;
        config->sectionCount = 0;
    }
}
