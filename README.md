# Dssembly

## Introduction

Dssembly is an assembly language for the DGC-32 computer architecture.

## Usage

To build Dssembly, run:

```
bash$ ./make.sh
```

Once built, use dssembly as such:

```
bash$ ./dssembly  #print usage
bash$ ./dssembly -r <inputFileName> <outputFileName>
bash$ ./dssembly <configFileName>
```

## Resources

See `doc/syntax.md` for a list of commands and syntax examples. See `doc/linker-config` to see how to configure the linker (dinker).