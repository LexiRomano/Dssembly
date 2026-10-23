# Syntax

## Registers

- `G0:G7`: General purpose registers - 32bit.
- `OA`, `OB`, and `OC`: Offset registers - 32bit
- `SB`: Stack base - 32bit
- `SS`: Stack size - 16bit
- `SP`: Stack pointer - 16bit
- `IL`: Interrupt table location - 32bit
- `FL`: Flags register - 8bit

## Comments

Comments are indicated with a preceding `//` and require a space seperation from any code preceding it on the same line (so `intres//resume interrupts` is not allowed).

## Literals

Literals can be specified as hexadecimal with `0x`, binary with `0b`, or decimal with no prefix. Decimal numbers can have a negative sign or decimal point where an instruction supports it.

```
    move G0 123
    move G1 0b1001
    move G2 0x80FE
    move G3 -53.875
```

## Directives

### Label

Labels are a portable way to reffer to certain parts of code. They can only be used with instructions which interact with memory in the relative addressing mode. They are defined with a preceding `:` as such:

```
:myLabel
    bralpc myLabel
```

For portability, all instructions using a label as an address are enforced to use the PC relative addressing mode.

### Aliases

Aliases are text representation of numbers. They are defined with `.define` and undefiend `.undef` directives with  as such:

```
.define myVariable 0x9000

    loadwab G0 myVariable

    // Is the same as:
    loadwab G0 0x9000

// Don't need this anymore
.undef myVariable
```

### Include

Include will define aliases defined in another file. Includes do not occur recursively if that file also has an include directive. They are defined with the `.include` directive as such:

```
// constant.dsb contains a bunch of .define
// directives, including "TABLE_ADDR" and
// "TABLE_END"
.include constants.dsb

    move     G0 0
    move     G1 TABLE_ADDR
:loop
    loadcab  G2 G1
    compi    G2 TABLE_END
    breqpc   break
    addi     G0 G0 1
    addi     G1 G1 1
    bralpc   loop

:break

```

### Reserve

Reservations will leave a certain number of bytes as empty space between instructions. They are defined with the `.reserve` directive as such:

```
:aVariable
    .reserve 4

    storwpc aVariable
```

### Set

Sets are similar to reservations, but they also initialize the memory to a given value. They are restricted to a size of 1, 2, and 4 bytes when setting numeric literals. They are defined with the `.set` directive as such:

```
:anotherVariable
    .set 4 0x00008000

    loadwpc G0 anotherVariable
```

Sets can also be used to store the 4-byte address of a label relative to the program's origin as such:

```
// At the very top of the first section:
:origin
// ...

    getabs   OA origin
    getreloa someFunction
    bpalpc   someFunction
.set @ someFunction

:someFunction
    // ...
```

Finally, sets can be used to store null-terminated strings as such:

```
:programName
    .set * "Hello, world!"

    getabs  OA programName
    move    G1 0
:printProgramName
    loadcoa G0 G1
    compi   G0 0
    breqpc  done
    bpal    print
    addi    G1 G1 1
    bralpc  printProgramName
```

### Section

The section directive declares all following instructions and labels to be part of a section. It is required when assembling in linked mode but ignored when assembling in raw mode, in which case it is ignored. Sections are defined with the `.section` directive as such:

```
.section start
    :reset
    move    OA 0x1000
    braloa  0
```

Multiple sections may be defined in one dssembly file, but labels may not be shared between those sections without usage of the next two directives:

### Export and Requires

The export defines a label for a given section to be used in other sections. The requires directive allows the usage of an exported label form another section. These directives are defined with the `.export` and `.requires` directives as such:

```
.section code

.requires helloWorld
.requires print
    
    getabs G0 helloWorld
    bpalpc print


.section text

.export helloWorld
    .set * "Hello, world!"
```

These directives are optional when assembling in likned mode and are ignored when assembling in raw mode.

## Instructions

### `move <dest> <src>`

Moves a number into a register. This number can be a literal or a number from another register. Supports negative and fractional decimal numbers.

Examples:
```
move G0 12

move G3 -5      // sign extended 2's compliment

move IL 0x8000

move G7 -100.5  // IEEE-754, evaluates to 0xC2C90000
```

### `loadxx <dest> <src>`

Loads a number from memory into a register. The source can be a register, literal which points to a memory address, or label. Different data widths and addressing modes, and sign extensions can be specified as follows:

- Width modifiers and sign extensions
    - load***w***x  -> load a word (4 bytes)
    - load***h***x  -> load a short (2 bytes)
    - load***sh***x -> load a signed short (2 bytes)
    - load***c***x  -> load a char (1 byte)
    - load***sc***x -> load a signed char (1 byte)
- Addressing modes
    - loadx***ab*** -> load from an absolute address
    - loadx***pc*** -> load relative to PC (used only with labels)
    - loadx***oa*** -> load relative to OA
    - loadx***ob*** -> load relative to OB
    - loadx***oc*** -> load relative to OC

Examples:
```
loadwab  G0 0x8000       // Loads 4 bytes from address 0x8000 into G0

loadhpc  SP stackRestore // Loads 2 bytes from the stackRestore label into SP

loadscoc G3 2            // Loads 1 signed byte from OC + 2 into G3

loadwoa  G7 G2           // Loads 4 bytes from OA + G2 into G7
```

### `storxx <src> <dest>`

Stores a number from a register into memory. It is the inverse of `loadxx` with the exception that there are no signed modifiers, so `storshx` and `storscx` do not exist.

### `swapxx <reg> <mem>`

Swaps between a register and memory. Functions the same as `loadxx` and `storxx` in the same instruction. All modifications listed in `loadxx` are supported.

### `addx <dest> <src1> <src2>`

Adds two numbers together and stores the result in a register. Can add both integers and floats with `addi` and `addf` respectively. The first source argument must be a register, but the second source register may be a register, literal, or label. Literals may be binary, hex, or signed decimal integers when using `addi` and may be signed decimal integers or fractionals when using `addf`. Flags register is updated.

### `subx <dest> <src1> <src2>`

Same as `addx` but with subtraction. `subi` for integers and `subf` for floats.

### `mulx <dest> <src1> <src2>`

Same as `addx` but with multiplication. `muli` for integers and `mulf` for floats.

### `divx <dest> <src1> <src2>`

Same as `addx` but with division. `src1` is the divident and `src2` is the divisor. `divi` for integers and `divf` for floats.

### `mod <dest> <src1> <src2>`

Same as `divi` but returns the remainder of the operation (modulo).

### `and <dest> <src1> <src2>`

Computes the bitwise AND of two numbers and stores in in a register. The first source argument must be a register, but the second source register may be a register or literal. Flags register is updated.

### `or <dest> <src1> <src2>`

Same as `and` but with bitwise OR.

### `xor <dest> <src1> <src2>`

Same as `and` but with bitwise XOR.

### `not <dest> <src>`

Computes the bitwise negation of a number and stores it in a register. The source must be a register. Flags register is updated.

### `bsxx <dest> <src> <amount>`

Bit shifts a number left or right and stores it in a register. `source` is the register containing the number to be shifted. `amount` is the number of bits to shift, it can either be a register or a literal. Only the lower 5 bits of `amount` are considered for the shift since the maximum shift that makes sense is 32 bits, so effectively `amount % 32` is used. Flags register is updated.

There are four variants of bit shifts, two directions and two wrapping behaviours. They are as follows:

Direction:
- bs***l***x -> shift to the left
- bs***r***x -> shift to the right

Wrapping behaviour
- bsx***t*** -> truncate the bits that get shifted out of the register
- bsx***c*** -> carry the carry flag into the incoming side of the register

Examples:
```
bslt G1 G1 3 // Shift G1 left by 3 bits, truncating, storing back into G1

bsrc G0 G3 1 // Shift G3 right by 1 bit, carrying around, storing into G0
```

### `ftoi <dest> <src>`

Converts a number from a float to a signed 32 bit integer. Both `src` and `dest` are registers.

### `uitof <dest> <src>`

Converts a number from an unsigned 32 bit integer to a float. Both `src` and `dest` are registers.

### `sitof <dest> <src>`

Same as `uitof` but the source is an unsigned 32 bit integer.

### `compx <src1> <src2>`

Compares two numbers to each other. Performs `src1 - src2`, saves the flags, but does not save the numerical result to a register. The arguments (excluding the lack of a destination register) are the same as `addx`. Comparisons can be made between either integers or floats with `compi` and `compf` respectively.

### `evalxx <dest> <src1> <src2>`

Compares two numbers to each other and evaluates a comparison condition to a boolean result. The two sources behave the same as `compx` but with the addition of a destination to store the resulting boolean value. The condition to evaluate is determined with the codes described in `Branch Conditions` near the end of this document. The flags will contain the result from the comparison, not the boolean resulting in the destination register.

Examples:
```
evalihs G0 G1 0x23 // If the unsigned contents of G1 are higher or the same as 0x23, put a 1 in G0. Else, put a 0.

evalflt G5 G7 -20.25 // If the float contents of G7 are lower than -20.25, put a 1 in G5. Else, put a 0.

evaline G4 G4 0 // Basically simplifies G4 to a boolean. If it's zero, keep it as a zero. If it's non-zero, make it a 1.
```

### `brxx <address>`

Branches to an address if a condition is satisfied. See `Branch Conditions` near the end of this document to see what conditions are supported. The address can be a register, literal, or label. There are several addressing modes supported to modify the destination address as such:

- brx***ab*** -> Branch to the absolute address provided
- brx***pc*** -> Branch to the address relative to PC (used only with labels)
- brx***oa*** -> Branch to the address + OA
- brx***ob*** -> Branch to the address + OB
- brx***oc*** -> Branch to the address + OC

Examples:
```
bralab 0x8000   // Unconditional branch to the absolute address 0x8000

brcspc overflow // Branch to a label called overflow if the carry flag is set

breqoa 0x12     // Branch to oa + 0x12 if the last operation resulted in an equivalent comparison
```

### `bpxx <address>`

The same as `brxx` but also push PC to the stack before branching. Used in conjunction with `return`.

### `push <src>`

Pushes a register value to the stack. The source must be a register.

### `pop <dest>`

Pops a value from the stack into a register. The destination must be a register.

### `peek <dest>`

Peeks a value from the stack into a register. The destination must be a register.

### `return`

Pops a value from the stack into PC. Used in conjunction with `bpxx`.

### `intsus`

Suspends non-maskable interrupts from being serviced.

### `intres`

Resumes the servicing of interrupts. If already in an interrupt service routine, the servicing will be completed without returning to the location from which the processor was interrupted from.

### `intfin`

Finishes the servicing of an interrupt by setting PC to IA. This is usually where the processor was originally interrupted, but can be modified with `intsra`.

### `inttgr <code>`

Interrupts the processor with an interrupt code. This interrupt code can be provided by a register or 16-bit literal.

### `intgpr <dest>`

Puts the interrupt parameter in the destination register.

### `intgra <dest>`

Puts the interrupt return address (IA) in a register. Destination must be a register.

### `intsra <source>`

Sets the interrupt return address (IA). Source can be either a register or a literal.

### `getabs <dest> <address>`

Gets the absolue address of a provided address relative to PC. Basically dest = address + PC. The address can be either a register or a literal.

### `getrelx <dest> <address>`

Gets an address which can be resolved relative to an offset register. The address is calculated relative to PC. Basically dest = address + PC - OX. The three offset registers may be selected as `getreloa`, `getrelob`, and `getreloc`.

### `debug <reg>`

For emulator debug mode only. Prints the contents of the specified register to the screen.

### Branch Conditions

| Code | Function            | Flags          |
| ---- | ------------------- | -------------- |
| al   | always              | none           |
| eq   | equal               | Z==1           |
| zs   | zero set            | "              |
| ne   | not equal           | Z==0           |
| zc   | zero cleared        | "              |
| hi   | usig higher         | C==1 && Z==0   |
| hs   | usig higher or same | C==1           |
| cs   | carry set           | "              |
| ls   | usig lower or same  | C==0 \|\| Z==1 |
| lo   | usig lower          | C==0           |
| cc   | carry cleared       | "              |
| gt   | sig greater         | Z==0 && N==V   |
| ge   | sig greater or same | N==V           |
| le   | sig less or same    | Z==1 \|\| N!=V |
| lt   | sig less            | N!=V           |
| mi   | negative            | N==1           |
| pz   | positive or zero    | N==0           |
| ov   | signed overflow     | V==1           |
| nv   | no signed overflow  | V==0           |

## Example Code

This code will scan for certain devices and store their device data addresses for later use.

```
.define derialAddress          0x8000
.define dardriveManagerAddress 0x8004
.define dardriveDriveAddress   0x8008

.define peripheralDeviceCode 0x05
.define storageDeviceCode    0x02
.define deviceRegistryEOT    0xFF

.define serialTypeID     0x53
.define derialProtocolID 0x01

.define dardriveManagerSPI 0x01
.define dardriveDriveSPI   0x02

// Scan for some devices
    move     OA 0x5FF8
:scanTop
    addi     OA OA 8
    compi    OA 0x6100
    breqpc   doneScan
    loadcoa  G0 1
    compi    G0 peripheralDeviceCode
    breqpc   isPeripheral
    compi    G0 peripheralDeviceCode
    breqpc   isStorage
    compi    G0 deviceRegistryEOT
    breqpc   doneScan
    bralpc   scanTop


:isPeripheral
    loadwoa  OB 4
    loadcob  G0 0
    compi    G0 serialTypeID
    brnepc   scanTop

    // is serial port
    loadcob  G0 1
    compi    G0 derialProtocolID
    brnepc   scanTop

    // is derial
    storwab  OB derialAddress
    bralpc   scanTop

:isStorage
    loadwoa  OB 4
    loadcob  G0 0
    compi    G0 dardriveManagerSPI
    breqpc   isDardriveManager
    compi    G0 dardriveDriveSPI
    brnepc   scanTop

    // is dardrive drive
    storwab  OB dardriveDriveAddress
    bralpc   scanTop

:isDardriveManager
    storwab  OB dardriveManagerAddress
    bralpc   scanTop

:doneScan
    // Continue on to do something else
```
