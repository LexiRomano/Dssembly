#include "dssembly.h"

#define LINK_ERROR(msg, ...) printf("Link error: " msg "\n", __VA_ARGS__)

static bool extractCodeSegment(FILE *fd, section_t *section)
{
    uint32_t restore  = 0;
    uint32_t codeSize = 0;

    codeSize = section->exportedLabelsOffset - section->codeSegmentOffset;

    if (0 == codeSize)
    {
        return true;
    }

    restore = ftell(fd);
    fseek(fd, section->codeSegmentOffset, SEEK_SET);

    section->codeSegment = malloc(codeSize);

    if (codeSize != fread(section->codeSegment, sizeof(uint8_t), codeSize, fd))
    {
        LINK_ERROR("could not read code segment of section \"%s\"", section->name);
        return false;
    }

    fseek(fd, restore, SEEK_SET);

    return true;
}

static bool extractExportedLabels(FILE *fd, section_t *section)
{
    uint32_t restore              = 0;
    char     labelNameBuffer[256] = {0};
    uint8_t  i                    = 0;
    label_t *newLabel             = NULL;

    restore = ftell(fd);
    fseek(fd, section->exportedLabelsOffset, SEEK_SET);

    while (true)
    {
        // Extract the name
        i = 0;
        do
        {
            labelNameBuffer[i] = fgetc(fd);
        } while (i < 255 && labelNameBuffer[i++] != '\0');

        if ('\0' == labelNameBuffer[0])
        {
            break;
        }

        newLabel = addNewLabel(&(section->exportedLabels));

        if (NULL == newLabel)
        {
            INTERNAL_ERROR;
            return false;
        }

        newLabel->label = strcpy(calloc(strlen(labelNameBuffer) + 1, sizeof(char)),
                                 labelNameBuffer);

        if (1 != fread(&(newLabel->address), sizeof(uint32_t), 1, fd))
        {
            LINK_ERROR("failed to parse object file \"%s\"", section->source);
            return false;
        }
    }

    fseek(fd, restore, SEEK_SET);

    return true;
}

static bool extractRequiredLabels(FILE *fd, section_t *section)
{
    uint32_t              restore              = 0;
    char                  labelNameBuffer[256] = {0};
    uint8_t               i                    = 0;
    requiredLabel_t      *newRequiredLabel     = NULL;
    uint16_t              instanceCount        = 0;
    resolutionInstance_t *resInstance          = NULL;

    restore = ftell(fd);
    fseek(fd, section->requiredLabelsOffset, SEEK_SET);

    while (true)
    {
        // Extract the name
        i = 0;
        do
        {
            labelNameBuffer[i] = fgetc(fd);
        } while (labelNameBuffer[i++] != '\0' && i < 255);

        if ('\0' == labelNameBuffer[0])
        {
            break;
        }

        newRequiredLabel = addNewRequiredLabel(&(section->requiredLabels));

        if (NULL == newRequiredLabel)
        {
            INTERNAL_ERROR;
            return false;
        }

        newRequiredLabel->name = strcpy(calloc(strlen(labelNameBuffer) + 1, sizeof(char)),
                                 labelNameBuffer);

        if (1 != fread(&instanceCount, sizeof(uint16_t), 1, fd))
        {
            LINK_ERROR("failed to parse object file \"%s\"", section->source);
            return false;
        }

        for (i = 0; i < instanceCount; i++)
        {
            resInstance = addNewResolutionInstance(&(newRequiredLabel->instances));

            if (NULL == resInstance)
            {
                INTERNAL_ERROR;
                return false;
            }

            if (1 != fread(&(resInstance->instructionOffset), sizeof(uint32_t), 1, fd) ||
                1 != fread(&(resInstance->injectionOffset),   sizeof(uint8_t),  1, fd))
            {
                LINK_ERROR("failed to parse object file \"%s\"", section->source);
                return false;
            }
        }
    }

    fseek(fd, restore, SEEK_SET);

    return true;
}

static bool extractOriginRelatives(FILE *fd, section_t *section)
{
    uint32_t          restore = 0;
    originRelative_t *tmpOR   = NULL;

    restore = ftell(fd);
    fseek(fd, section->originRelativeOffset, SEEK_SET);

    if (1 != fread(&(section->originRelatives.count), 2, 1, fd))
    {
        LINK_ERROR("failed to parse object file \"%s\"", section->source);
        return false;
    }

    for (uint32_t i = 0; i < section->originRelatives.count; i++)
    {
        tmpOR = addNewOriginRelative(&(section->originRelatives));

        if (1 != fread(&(tmpOR->address), 4, 1, fd))
        {
            LINK_ERROR("failed to parse object file \"%s\"", section->source);
            return false;
        }
    }

    fseek(fd, restore, SEEK_SET);

    return true;
}

static bool extractRequiredSections(dinkerConfig_t *config, char *fileName, sectionList_t *out)
{
    FILE      *fd                     = NULL;
    uint8_t    i                      = 0;
    char       sectionNameBuffer[255] = {0};
    bool       isRequired             = false;
    section_t *newSection             = NULL;

    if (NULL == fileName ||
        NULL == out)
    {
        INTERNAL_ERROR;
        return false;
    }

    fd = fopen(fileName, "r");

    if (NULL == fd)
    {
        printf("Failed to open object file %s\n", fileName);
        return false;
    }

    while (true)
    {
        // Extract the name
        i = 0;
        do
        {
            sectionNameBuffer[i] = fgetc(fd);
        } while (sectionNameBuffer[i++] != '\0' && i < 255 );

        if ('\0' == sectionNameBuffer[0])
        {
            break;
        }

        // Skip if not required by the config file
        isRequired = false;
        for (i = 0; i < config->sectionCount; i++)
        {
            if (0 == strcmp(config->sections[i], sectionNameBuffer))
            {
                isRequired = true;
            }
        }

        if (false == isRequired)
        {
            continue;
        }

        // Extract basic section information
        newSection = addNewSection(out);

        if (NULL == newSection)
        {
            INTERNAL_ERROR;
            return false;
        }

        newSection->name = strcpy(calloc(strlen(sectionNameBuffer) + 1, sizeof(char)),
                                  sectionNameBuffer);

        newSection->source = strcpy(calloc(strlen(fileName) + 1, sizeof(char)),
                                    fileName);

        if (1 != fread(&(newSection->codeSegmentOffset),    sizeof(uint32_t), 1, fd) ||
            1 != fread(&(newSection->exportedLabelsOffset), sizeof(uint32_t), 1, fd) ||
            1 != fread(&(newSection->requiredLabelsOffset), sizeof(uint32_t), 1, fd) ||
            1 != fread(&(newSection->originRelativeOffset), sizeof(uint32_t), 1, fd))
        {
            fclose(fd);
            LINK_ERROR("failed to read from %s", fileName);
            return false;
        }

        // Extract segment data
        if (false == extractCodeSegment    (fd, newSection) ||
            false == extractExportedLabels (fd, newSection) ||
            false == extractRequiredLabels (fd, newSection) ||
            false == extractOriginRelatives(fd, newSection))
        {
            fclose(fd);
            return false;
        }
    }

    fclose(fd);

    return true;
}

static bool extractAllRequiredSections(dinkerConfig_t *config, sectionList_t *sections)
{
    sectionList_t tmpSectionList  = {0};

    if (NULL == config)
    {
        INTERNAL_ERROR;
        return false;
    }

    for (uint8_t i = 0; i < config->inputFileCount; i++)
    {
        if (NULL == config->inputFiles[i].objectName)
        {
            INTERNAL_ERROR;
            return false;
        }

        // Extract the section from each file
        if (false == extractRequiredSections(config, config->inputFiles[i].objectName, &tmpSectionList))
        {
            return false;
        }

        // Ensure no duplicate section names
        for (section_t *s = tmpSectionList.first; s != NULL; s = s->next)
        {
            if (NULL != getSection(sections, s->name))
            {
                LINK_ERROR("multiple sections named \"%s\"", s->name);
                return false;
            }
        }

        // Append lists
        if (NULL == sections->first)
        {
            sections->first = tmpSectionList.first;
            sections->last  = tmpSectionList.last;
        }
        else
        {
            sections->last->next = tmpSectionList.first;
            sections->last       = tmpSectionList.last;
        }

        tmpSectionList.first = NULL;
        tmpSectionList.last  = NULL;
    }

    return true;
}

static bool findExportedLabel(char *name, sectionList_t *sections, section_t **section, label_t **label)
{
    label_t *tmpLbl = NULL;

    *label   = NULL;
    *section = NULL;

    for (section_t *s = sections->first; s != NULL; s = s->next)
    {
        tmpLbl = getLabel(&(s->exportedLabels), name);

        if (NULL == tmpLbl)
        {
            continue;
        }

        *label   = tmpLbl;
        *section = s;

        return true;
    }

    return false;
}

static bool output(dinkerConfig_t *config, sectionList_t *sections)
{
    FILE      *out               = NULL;
    section_t *section           = NULL;
    uint32_t   codeSize          = 0;
    label_t   *foundLabel        = NULL;
    uint32_t   foundLabelOffset  = 0;
    uint32_t   instructionOffset = 0;
    uint32_t   writeVal          = 0;

    if (NULL == config ||
        NULL == sections)
    {
        INTERNAL_ERROR;
        return false;
    }

    out = fopen(config->outputFile, "wb+");

    if (NULL == out)
    {
        LINK_ERROR("Could not create \"%s\"", config->outputFile);
        return false;
    }

    // Fill code segments
    for (uint8_t i = 0; i < config->sectionCount; i++)
    {
        section = getSection(sections, config->sections[i]);

        if (NULL == getSection(sections, config->sections[i]))
        {
            LINK_ERROR("could not find section \"%s\"", config->sections[i]);
            fclose(out);
            remove(config->outputFile);
            return false;
        }

        codeSize = section->exportedLabelsOffset - section->codeSegmentOffset;

        // Convert code segment offset into output file space
        section->codeSegmentOffset = ftell(out);

        if (codeSize > 0 &&
            codeSize != fwrite(section->codeSegment, sizeof(uint8_t), codeSize, out))
        {
            LINK_ERROR("could not write to \"%s\"", config->outputFile);
            fclose(out);
            remove(config->outputFile);
            return false;
        }
    }

    // Resolve labels and origin relatives
    for (section_t *sec = sections->first; sec != NULL; sec = sec->next)
    {
        // Labels
        for (requiredLabel_t *reqLbl = sec->requiredLabels.first;
             NULL != reqLbl;
             reqLbl = reqLbl->next)
        {
            if (false == findExportedLabel(reqLbl->name,
                                           sections,
                                           &section,
                                           &foundLabel))
            {
                LINK_ERROR("could not find label \"%s\" for section \"%s\" (from %s)",
                           reqLbl->name, sec->name, sec->source);
                fclose(out);
                remove(config->outputFile);
                return false;
            }

            foundLabelOffset = section->codeSegmentOffset + foundLabel->address;

            for (resolutionInstance_t *resI = reqLbl->instances.first; resI != NULL; resI = resI->next)
            {
                instructionOffset = resI->instructionOffset + sec->codeSegmentOffset;

                fseek(out, instructionOffset + (resI->injectionOffset & 0x7F), SEEK_SET);

                if (0 != (resI->injectionOffset & 0x80))
                {
                    writeVal = foundLabelOffset;
                }
                else
                {
                    writeVal = foundLabelOffset - instructionOffset;
                }

                if (1 != fwrite(&writeVal, sizeof(uint32_t), 1, out))
                {
                    LINK_ERROR("could not write to \"%s\"", config->outputFile);
                    fclose(out);
                    remove(config->outputFile);
                    return false;
                }
            }
        }

        // Origin relatives
        for (originRelative_t *orRel = sec->originRelatives.first;
             NULL != orRel;
             orRel = orRel->next)
        {
            fseek(out, section->codeSegmentOffset + orRel->address, SEEK_SET);

            if (1 != fread(&writeVal, sizeof(uint32_t), 1, out))
            {
                INTERNAL_ERROR;
                LINK_ERROR("could not write to \"%s\"", config->outputFile);
                fclose(out);
                remove(config->outputFile);
                return false;
            }

            writeVal += section->codeSegmentOffset;

            fseek(out, -4, SEEK_CUR);

            if (1 != fwrite(&writeVal, sizeof(uint32_t), 1, out))
            {
                LINK_ERROR("could not write to \"%s\"", config->outputFile);
                fclose(out);
                remove(config->outputFile);
                return false;
            }
        }
    }

    fclose(out);

    return true;
}

bool dinker(dinkerConfig_t *config)
{
    sectionList_t sections = {0};
    bool          rc       = false;

    if (false == extractAllRequiredSections(config, &sections))
    {
        return false;
    }

    rc = output(config, &sections);
    
    freeSectionListContents(&sections);

    return rc;
}
