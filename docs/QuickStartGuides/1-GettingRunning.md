# Default Mode

Symbulation allows you to compile your C++ to either:

* running at the command line (e.g., native machine code) and
* running in the web browser (e.g., javascript assembly code).

This how-to aims to walk you through all the nitty gritty required to
successfully compile a simple "default" executable for running at the command line.

## Install: Native C++ Compiler

In the Unix-verse (e.g., Linux / MacOS) commonly used compilers include `gcc` and `clang`. From this point onwards, we assume that you're working with `gcc`.  
Unless you really know what you're doing, you'll want to have `gcc` installed. The good news is: you might already!

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

For Windows users, things are a bit more complicated.
We recommend using [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/) and then following the steps above for `gcc`.

## Compile & Run: Command Line

Assuming you haven't already pulled down a clone of Symbulation, let's
get your working environment all set. 

### Using Cookiecutter
The easiest thing to do is use our Cookiecutter template. Cookiecutter will attempt to push your new directory to a git repo; establish a git repo with the appropriate project name if you would like to use this functionality. 

- Install the latest Cookiecutter. Depending on your Python, you may need to use pip3 in place of pip:

    ```shell
    pip install -U cookiecutter
    ```

- Generate a Symbulation project:
    ```shell
    cookiecutter https://github.com/anyaevostinar/SymbulationProjectTemplate.git
    ```

- Change directory into your newly created project:
    ```
    cd SymbulationProject
    ```

### Without Cookiecutter
If you have something against delicious cookies and want to do without Cookiecutter, here are the steps. (Note that these steps don't include making Data and Analysis files, which is assumed for the rest of the guide. If you are doing it this way, we assume you have your own preference for organizing things.)

1. Clone Symbulation:
    ```shell
    git clone https://github.com/anyaevostinar/SymbulationEmp
    ```

### Compiling

Regardless of how you got everything downloaded, it's now time to compile!

```shell
cd SymbulationEmp
make
```

Then run:
```shell
./symbulation_default
```

You can make changes to the file `SymSettings.cfg` to change the configuration options without needing to recompile.
These can also be altered at the command line:
```
./<executable file> -<Option to change> <new value>
```

For example:
```
./symbulation_default -VERTICAL_TRANSMISSION 0.5 -GRID_X 50 -GRID_Y 50
```

To see how to use our workflow and scripts to collect and analyze data, please proceed to the [Collecting Data](https://symbulation.readthedocs.io/en/latest/QuickStartGuides/2-CollectingData.html) quickstart guide!

## Install: Web GUI

These instructions are for if you want to make *changes* to the web GUI and run it locally. You don't need to do this to be able to use Symbulation's web browser; you can instead access it here: [https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html).

First, you'll need to set up Emscripten, which is included in the `SymbulationEmp` folder:
```bash
cd emsdk
./emsdk install 1.38.48
./emsdk activate 1.38.48
```

Then every time that you want to build and run Symbulation's web GUI, you'll need to load Emscripten into your `PATH` for a given terminal session. If you've closed and opened a new terminal, you'll need to do this again (this is what people usually forget to do after they've been away for a while):

```
cd emsdk
source ./emsdk_env.sh
```

Then navigate back to SymbulationEmp's top folder and build the web version:

```
cd ..
make web
```

Now you have the website all built and in the `web` folder, so just run:

```
make serve
```

You should then be able to go to [http://localhost:8000/](http://localhost:8000/) to see you custom run Symbulation GUI!