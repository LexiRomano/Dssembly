# Syntax

## Registers

`G0:G7`: General purpose registers - 32bit.

`OA`, `OB`, and `OC`: Offset registers - 32bit

`SB`: Stack base - 32bit

`SS`: Stack size - 16bit

`IL`: Interrupt table location - 32bit

## Literals

Literals can be specified as hexadecimal with `0x`, binary with `0b`, or decimal with no prefix.

A memory address is designated by a preceding `*`, immediate values have no prefix.

## Labels

Labels are created with a preceding `:` and are referenced with a `$` prefix.

## Instructions

```
NOOP
// Does nothing, no arguments

MOVE dest src
// Moves data between two registers or puts a value into a register. `src` 
// can be either a register or an immediate value, `dest` is a register.

LOAD-<addressingMode> dest src
// Loads a value from memory into a register. `src` specifies the
// location in memory to load from. It can be an immediate value, a register,
// or a label. `dest` is the destination register. `addressingMode` selects
// how the memory address is interpreted (see section below).

STOR-<addressingMode> dest src
// Stores a value from a register into memory. `src` is the register to store
// in memory. `dest` specifies the location in memory to store to. It can be
// an immediate value, a register, or a label. `addressingMode` selects how
// the memory address is interpreted (see section below).

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

None: The memory address will be interpreted as relative to the command.

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