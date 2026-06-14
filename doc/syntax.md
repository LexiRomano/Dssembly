# Syntax

## Registers

`G0:G7`: General purpose registers - 32bit.

`OA`, `OB`, and `OC`: Offset registers - 32bit

`SB`: Stack base - 32bit

`SS`: Stack size - 16bit

`SP`: Stack pointer - 16bit

`IL`: Interrupt table location - 32bit

`FL`: Flags register - 8bit

## Comments

Comments are indicated with a preceding `//` and require a space seperation from any code preceding it on the same line (so `NOOP//does nothing` is not allowed).

## Literals

Literals can be specified as hexadecimal with `0x`, binary with `0b`, or decimal with no prefix.

```
    MOVE G0 123
    MOVE G1 0b1001
    MOVE G2 0x80FE
```

## Directives

### Label

Labels are a portable way to reffer to certain parts of code. They can only be used with instructions which interact with memory in the relative addressing mode. They are defined with a preceding `:` as such:

```
:myLabel
    BRAL myLabel
```

### Alias

Aliases are text representation of numbers. They are defined with the `.alias` directive as such:

```
.alias myVariable 0x9000

    LOAD-ABS G0 myVariable

    // Is the same as:
    LOAD-ABS G0 0x9000
```

### Include

Include will define aliases defined in another file. Includes do not occur recursively if that file also has an include directive. They are defined with the `.include` directive as such:

```
// constant.dsb contains a bunch of .alias
// directives, including "TABLE_ADDR" and
// "TABLE_END"
.include constants.dsb

    MOVE      G0 0
    MOVE      G1 TABLE_ADDR
:loop
    LOAD-W1S0 G2 G1
    COMP      G2 TABLE_END
    BREQ      break
    ADD       G0 G0 1
    ADD       G1 G1 1
    BRAL      loop

:break

```

### Reserve

Reservations will leave a certain number of bytes as empty space between instructions. They are defined with the `.reserve` directive as such:

```
:aVariable
    .reserve 4

    STOR G0 aVariable
```

### Set

Sets are similar to reservations, but they also initialize the memory to a given value. They are restricted to a size of 1, 2, and 4 bytes when setting numeric literals. They are defined with the `.set` directive as such:

```
:anotherVariable
    .set 4 0x00008000

    LOAD G0 anotherVariable
```

Sets can also be used to store null-terminated strings as such:

```
:programName
    .set * "Hello, world!"

    GETABS       OA programName
    MOVE         G1 0
:printProgramName
    LOAD-W1S0-OA G0 G1
    COMP         G0 0
    BREQ         done
    BRAL-P       print
    ADD          G1 G1 1
    BRAL         printProgramName
```

### Section

The section directive declares all following instructions and labels to be part of a section. It is required when assembling in linked mode but ignored when assembling in raw mode, in which case it is ignored. Sections are defined with the `.section` directive as such:

```
.section start
    :reset
    MOVE     0x1000 OA
    BRAL-OA  0
```

Multiple sections may be defined in one dssembly file, but labels may not be shared between those sections without usage of the next two directives:

### Export and Requires

The export defines a label for a given section to be used in other sections. The requires directive allows the usage of an exported label form another section. These directives are defined with the `.export` and `.requires` directives as such:

```
.section code

.requires helloWorld
.requires print
    
    GETABS G0 helloWorld
    BRAL-P print
    TERM


.section text

.export helloWorld
    .set * "Hello, world!"
```

These directives are optional when assembling in likned mode and are ignored when assembling in raw mode.

## Instructions

```
NOOP
// Does nothing, no arguments

MOVE dest src
// Moves data between two registers or puts a value into a register. `src` 
// can be either a register or an immediate value, `dest` is a register.

LOAD-<wordGeometry>-<addressingMode> dest src
// Loads a value from memory into a register. `src` specifies the
// location in memory to load from. It can be an immediate value, a register,
// or a label. `dest` is the destination register. `addressingMode` selects
// how the memory address is interpreted (see section below). `wordGeometry`
// describes how many bytes to load and how to put them in `dest` (see
// Word Geometry section).

STOR-<wordGeometry>-<addressingMode> src dest
// Stores a value from a register into memory. `src` is the register to store
// in memory. `dest` specifies the location in memory to store to. It can be
// an immediate value, a register, or a label. `addressingMode` selects how
// the memory address is interpreted (see section below). `wordGeometry`
// describes how many bytes to store and how to extract them from `dest` (see
// Word Geometry section).

ADD dest src1 src2
// Adds two values together and stores it in a register. `src1` is a register.
// `src2` can be either a register or an immediate value. `dest` is the
// register where the result is stored.

SUB dest src1 src2
// Same as ADD but subtracting `src2` from `src1`.

AND dest src1 src2
// Same as ADD but doing the bitwise AND.

OR dest src1 src2
// Same as ADD but doing the bitwise OR.

XOR dest src1 src2
// Same as ADD but doing the bitwise XOR.

NOT dest src
// Does a bitwise negation. `src` is the input register. `dest` is the
// output register.

BSLT dest src count
// Shifts a value to the left and truncates any bits that got shifted out of
// the register. `src` is the input register to be shifted. `count` is how
// many bits to shift by. `dest` is the register where the result is stored.

BSRT
// Same as BSLT but shifting to the right.

BSLC
// Same as BSLT but any bits that got shifted out of the register carry
// around to the other side.

BSRC
// Same as BSLC but shifting to the right

COMP src1 src2
// Compares two values to each other. This will subtract `src2` from `src1` and
// update the flags register without saving the result. `src1` is a register
// while `src2` can be a register or an immediate value.

BR<condition>-<pushReturn>-<addressingMode> address
// Branches execution to another place in memory. `address` is the location to
// branch to. It can either be a register or an immediate value.
// `addressingMode` selects how the memory address is interpreted (see section
// below). `condition` specifies the condition under which the branch will be
// taken (see Branch Conditions). `pushReturn` specifies whether or not to push
// a return address to the stack. Add `P` to push, omit to not.

PUSH src
// Pushes the `src` register to the stack.

POP  dest
// Pops the `dest` register from the stack into `dest`.

PUSHALL
// Pushes all of the general purpose registers plus the flags register to the
// stack. No arguments

POPALL
// Pops all of the general purpose registers plus the flags register from the
// stack. No arguments

PEEK dest
// Peeks the top of the stack into the `dest` register without removing it from
// the stack.

RETURN
// Returns using a return address pushed by a BR command that used the `P` flag.
// No arguments.

INTSUS
// Susspends all non-critical interrupts. No arugments

INTRES
// Resums all interrupts. No arguments.

INTTRG src
// Triggers an interrupt. `src` indicates what kind of interrupt, it can either
// be a register or an immediate value.

INTFIN
// Finishes the handling of an interrupt. Will return to where the processor
// was executing before the interrupt was triggered. Will also resume allowing
// other interrupts to get triggered. No arguments.

INTGPR dest
// Gets the parameter of the interrupt. Loads it into `dest`.

GETABS-<addressingMode> dest address
// Gets the absolute memory address after computing with an addressing mode.
// This works the same as LOAD except that it loads the memory address of the
// target rather than the target itself. `address` is the address to consider.
// It can be a register, immediate value, or label. `dest` is where to store
// the calculated memory address. `addressingMode` is what addressing mode
// to consider (see section below). The ABS addressing mode is not supported.

GETREL-<addressingMode> dest address
// Works the same as except it returns an address which can be resolved relative
// to the selected offset register. Addressing mode must be an offset register.

TERM
// Terminates the execution of the processor. No arugments.

```

### Addressing Modes

None: The memory address will be interpreted as relative to the command. Jumps to labels can only be relative.

`OA`, `OB`, and `OC` will make the command add the respective offset register to the related memory address

`ABS` will interpret the memory address to be absolute.

### Branch Conditions

| Code | Function            | Flags          |
| ---- | ------------------- | -------------- |
| AL   | always              | none           |
| EQ   | equal               | Z==1           |
| NE   | not equal           | Z==0           |
| HI   | usig higher         | C==1 && Z==0   |
| HS   | usig higher or same | C==1           |
| LS   | usig lower or same  | C==0 \|\| Z==1 |
| LO   | usig lower          | C==0           |
| GT   | sig greater         | Z==0 && N==V   |
| GE   | sig greater or same | N==V           |
| LE   | sig less or same    | Z==1 \|\| N!=V |
| LT   | sig less            | N!=V           |
| MI   | negative            | N==1           |
| PZ   | positive or zero    | N==0           |
| OV   | signed overflow     | V==1           |
| NV   | no signed overflow  | V==0           |

### Word Geometry

By default, a LOAD or STORE command will interact with 4 bytes. You can select different word sizes (W) and offsets (S) within the register using the following flags:

```
W2S0 - Selects a 2 byte word with a 0 byte shift:

  0xABCDEFGH
        \__/
          Selected

W2S1 - Selects a 2 byte word with a 1 byte shift:

  0xABCDEFGH
      \__/
        Selected

W2S2 - Selects a 2 byte word with a 2 byte shift:

  0xABCDEFGH
    \__/
      Selected

W1S0 - Selects a 1 byte word with a 0 byte shift:

  0xABCDEFGH
          \/
           Selected

W1S1 - Selects a 1 byte word with a 1 byte shift:

  0xABCDEFGH
        \/
         Selected

W1S2 - Selects a 1 byte word with a 2 byte shift:

  0xABCDEFGH
      \/
       Selected

W1S3 - Selects a 1 byte word with a 3 byte shift:

  0xABCDEFGH
    \/
     Selected
```

Note that using a word size larger than the register it is bound for may result in undesired behaviour. For example, when doing a `LOAD` into `SP` with no specified word geometry, the processor will try to fit 32 bits into a 16 bit register. Therefore, a W2S* or W1S* word geometry must be used.

## Example Code

This code will scan for certain devices and store their device data addresses for later use

```
.alias derialAddress          0x8000
.alias dardriveManagerAddress 0x8004
.alias dardriveDriveAddress   0x8008

.alias peripheralDeviceCode 0x05
.alias storageDeviceCode    0x02
.alias deviceRegistryEOT    0xFF

.alias serialTypeID     0x53
.alias derialProtocolID 0x01

.alias dardriveManagerSPI 0x01
.alias dardriveDriveSPI   0x02

// Scan for some devices
    MOVE          OA 0x5FF8
:scanTop
    ADD           OA OA 8
    COMP          OA 0x6100
    BREQ          doneScan
    LOAD-W1S0-OA  G0 1
    COMP          G0 peripheralDeviceCode
    BREQ          isPeripheral
    COMP          G0 peripheralDeviceCode
    BREQ          isStorage
    COMP          G0 deviceRegistryEOT
    BREQ          doneScan
    BRAL          scanTop


:isPeripheral
    LOAD-OA       OB 4
    LOAD-W1S0-OB  G0 0
    COMP          G0 serialTypeID
    BRNE          scanTop
    // is serial port
    LOAD-W1S0-OB  G0 1
    COMP          G0 derialProtocolID
    BRNE          scanTop
    // is derial
    STOR-ABS      OB derialAddress
    BRAL          scanTop

:isStorage
    LOAD-OA       OB 4
    LOAD-W1S0-OB  G0 0
    COMP          G0 dardriveManagerSPI
    BREQ          isDardriveManager
    COMP          G0 dardriveDriveSPI
    BRNE          scanTop
    // is dardrive drive
    STOR-ABS      OB dardriveDriveAddress
    BRAL          scanTop

:isDardriveManager
    STOR-ABS      OB dardriveManagerAddress
    BRAL          scanTop

:doneScan
    NOOP
    // Continue on to do something else
```