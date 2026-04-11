# Syntax

## Registers

`G0:G7`: General purpose registers - 32bit.

`OA`, `OB`, and `OC`: Offset registers - 32bit

`SB`: Stack base - 32bit

`SS`: Stack size - 16bit

`IL`: Interrupt table location - 32bit

## Literals

Literals can be specified as hexadecimal with `0x`, binary with `0b`, or decimal with no prefix.

## Labels

Labels are created with a preceding `:` and are referenced with a `$` prefix.

## Comments

Comments are indicated with a preceding `//` and require a space seperation from any code preceding it on the same line (so `NOOP//does nothing` is not allowed).

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

STOR-<wordGeometry>-<addressingMode> dest src
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
// while `src1` can be a register or an immediate value.

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
// Pops the `dest` register from the stack into `dest.

PUSHALL
// Pushes all of the general purpose registers plus the flags register to the
// stack. No arguments

POPALL
// Pops all of the general purpose registers plus the flags register from the
// stack. No arguments

PEEK dest
// Peeks the top of the stack into the `dest` register without removing it from
// the stack.

INITSTACK
// Initializes the stack. No arguments.

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

GETABS-<addressingMode> dest address
// Gets the absolute memory address after computing with an addressing mode.
// This works the same as LOAD except that it loads the memory address of the
// target rather than the target itself. `address` is the address to consider.
// It can be a register, immediate value, or label. `dest` is where to store
// the calculated memory address. `addressingMode` is what addressing mode
// to consider (see section below).

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

By default, a LOAD or STORE command will interact with 4 bytes (or 2 bytes for the SS register). You can select different word sizes and offsets within the register using
the following flags:

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

Note that only `W1S0` and `W1S1` are applicable to the `SS` register.

## Example Code
Checks to see if a byte in memory is 7 or lower. If it is, terminate. If it's not, shift it to the left by how much higher it was than 7, store it in memory, jump to some specific location in absolute memory space, and return.

```
// Load a single byte into G0 using the OC register as offset
LOAD-W1S0-OC  G0 *0x1000

// Check if its 7 or lower
COMP          G0 7
BRHE          $finish

// Not 7
AND           G0 G0 0x000000FF
SUB           G1 G0 7
BSLC          G0 G0 G1
STOR-OC       *0x1004 G0

// Some important system call
BRAL-ABS      $0x90FE

:finish
TERM

```