# Object File

Dssembly object files (`.dob`)

## Overview

Dssembly object files are comprised of the following parts:

- Header
    - Section names
    - Pointers to section segments
- Sections
    - Code segment
    - Exported labels segment
    - Required labels segment

The header is always the first to show up in the object file. Following that are all of the sections.

## Header

The header of an object file contains the names of all sections, and pointers to the different segments of each section. Each header entry starts with the null-terminated name of the section it represents. Following the null terminator, three 4-byte file offsets are provided corresponding to the code, exported labels, and required labels segments. After the final header entry, a single null byte signifies the end of the header, after which the sections shall follow.

## Sections

### Code segment

This segment contains all of the source code of the segment. Its size shall be determined from the corresponding header entry by subtracting the code segment offset from the exported labels segment.

### Exported labels segment

This segment contains all of the labels in the corresponding code segment which can be referenced by other sections. Each entry is comprised of a null-terminated name of the label, followed by a 4-byte offset from the beginning of the code segment corresponding to the location of the label. After the final entry, a single null byte signifies the end of the segment.

### Required labels segment

This segment contains all of the labels which the corresponding code segment requires to be resolved. Each entry is comprised of a null-terminated name of the required label, a 2-byte instance count, and one or more 5-byte resolution instances. The instances count specifies how many resolution instances there are for the required label, so if the label was required by 7 different instructions, this number will be 7.

Each resolution instance is comprised of an instruction offset and an injection offset. The instruction offset is 4 bytes and corresponds to the offset within the code segment where the instruction requiring the label resides. The injection offset is 1 byte and corresponds to the offset from the instruction's origin to where the required label information shall be injected. For example, take a LOAD instruction residing at an offset of 0x3A7 within the code segment. In this case the instruction offset will be equal to 0x3A7, and the injection offset will be equal to 5 since a LOAD instruction takes 4 bytes for the main instruction, a 1 byte instruction augment, and then a 4 byte argument augment where the address information resides.

After the final required label entry, a single null byte signifies the end of the segment and therby the section.
