# Default

Symbulation allows you to compile your C++ to target

- running at the command line (e.g., native machine code) and
- running in the web browser (e.g., javascript assembly code).

This how-to aims to walk you through all the nitty gritty required to
successfully compile a simple "Default" for both targets.

## Install: Native C++ Compiler

In the Unix-verse (e.g., Linux / MacOS) commonly used compilers include
`gcc` and `clang`. From this point onwards, we assume that you're
working with `gcc`. Unless you really know what you're doing, you'll
want to have `gcc` installed. The good news is: you might already!

Bring up a terminal and try entering:

```shell
which gcc
```

If `which` spits out a path, then you have `gcc` installed! If `which`
says "not found," you'll need to go ahead and install `gcc`. For
Linux users, your package manager (e.g., `yum`, `apt`, etc.) is probably
the way to go about this. For MacOS users, you'll need to get Apple's
"Command Line Tools for Xcode." Either way, give it a quick web search
(e.g., "install gcc on [my operating system]") and there should be
plenty of how-to guides that walk you through step-by-step.

TODO Windows... Maybe you should try git for Windows (e.g.,
"GitBash")?

## Compile & Run: Command Line

Assuming you haven't already pulled down a clone of Symbulation, let's
get your working environment all set. 

```shell
git clone https://github.com/anyaevostinar/SymbulationEmp
```
In the same level as SymbulationEmp, you will be a recursive copy of Empirical. 

```shell
git clone --recursive https://github.com/devosoft/Empirical.git 
```

Let's compile!

```shell
cd SymbulationEmp
make
```

```shell
./symbulation
```
