# Creating A New Mode

*Note: this guide is in progress and does not contain all helpful details.*

Once you have familiarized yourself with Symbulation and its default mode, you might be interested in designing your own experiment, adding functionality to the codebase, and collecting data. 
The existing modes include `default` (Host and Symbiont), `efficient` (Efficient Host and Efficient Symbiont), `lysis` (Bacterium and Phage), and public goods game or `pgg` (PGGHost and PGGSymbiont). 
There are several steps to creating your own world, including following conventions for file structure, adding your own organisms, designing tests, adding a WorldSetup file, adding targets to the makefile, and more. 
This guide will walk you through how to properly add all of these features.

First, you must decide if your new mode falls under one of two scenarios. 
In the first scenario, you wish to change the processes of an existing organism, but will not be adding any new traits. 
The second scenario includes the addition of new traits, or genome values, and will therefore also require functions that track the evolution of this new trait. 
Depending on the goals of your project and which scenario it falls under (1 or 2), you will need to add to the codebase in a varying manner. 
Most sections below pertain to both scenarios and should be completed no matter what. Sections that are specific to a particular scenario will be labeled accordingly.

## Primary Folder
Choose a one word descriptor for your new mode. 
Then make a folder in `SymbulationEmp/source` named `<name>_mode`. 
Inside of this folder will be a world setup source file (explained in more detail later), as well as any necessary header files.

### Organisms
Next, you will need to add new header files for your new organisms. 
The host organism(s) must extend the `Host.h` class in the `source/default_mode`, and similarly, the symbiont organism(s) must extend the `Symbiont.h` class. 
Each new class must contain the following:
    *constructor
    *genome/instance variables (Scenario 2 only)
    *mutate (Scenario 2 only)
    *makeNew
    *process
    *config settings to edit genome/instance variables (Scenario 2 only)

### World Class: Scenario 2 Only
In the second scenario, since you have added new traits to the organisms, you will need to add code that can track the evolution of these traits. 
Once the organisms have been made, they must be put in a world. 
Create a world class that extends the `SymWorld.h` class in the `source/default_mode`.
Next, make sure to add data nodes and functions that set up a data tracking file by using your newly created data nodes. 

### World Setup
Next, create the source file to set up your newly created world. 
Follow a similar pattern as the other mode setup files. 
The primary difference should be the organism types added to the world, which should now be your newly created host(s) and symbiont(s).

## Native File
Next, add a file to `source/native` with the name `symbulation_<name>.cc`. 
This is the main source file that will allow the experiment to function. 
In it, the world is created, set up according to the setup file, and is permitted to run for the specified number of updates.

## Makefile
Lastly, add the necessary targets to the Makefile for your new mode. Your new mode will need:
    - a compiling target, with the naming convention "<name>-mode"
    - a debug taget, with the naming convention "debug-<name>" and "<name>-debug"
    - a testing target, with the naming convention "test-<name>"
    - a debug while testing target, with the naming convention "test-debug-<name>"

## Testing
After you have created your new mode, you must design tests to ensure it is functioning as expected. 
Add a folder to `source/test` with the name `<name>_mode_test`. 
Inside of this folder, add testing source files corresponding to each organism. 
Make sure to also add files that test the interactions between organisms, as well as the data nodes.
Also make sure to do the following: 
    - Include the testing file paths in `source/catch/main.cc`.
    - Add catch tags

## Experiment!
Once you have created your organisms and corresponding world, designed their tests, added a main source file, and added targets to the Makefile, you are ready to experiment!