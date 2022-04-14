# Creating A New Mode

Once you have familiarized yourself with Symbulation and its default mode, you might be interested in designing
your own experiment, adding functionality to the codebase, and collecting data. The existing modes include
default (Host and Symbiont), efficient (Efficient Host and Efficient Symbiont), lysis (Bacterium and Phage),
and public goods game or PGG (PGG Host and PGG Symbiont). There are several steps to creating your own world,
including following conventions for file structure, adding your own organisms, designing tests, adding a 
WorldSetup file, adding targets to the makefile, and more. 

First, choose a one word descriptor for your new mode. Then make a folder in `source` named `<name>_mode`. Inside of this folder will be the header files for your organisms and your world, along with a world setup source file.

The host organism(s) must extend the `Host.h` class in the `source/default_mode`, and similarly, the symbiont organism(s) must extend the `Symbiont.h` class.
    - constructor
    - genome/instance variables
    - mutate
    - makeNew
    - process
    - config settings to edit genome/instance variables

Once the organisms have been made, they must be put in a world. Create a world class that extends the `SymWorld.h` class in the `source/default_mode`. 
    - Add data nodes and setup file functions

Next, create the source file to set up your newly created world. Follow a similar pattern as the other mode setup files. The primary difference should be the organism types added to the world, which should now be your newly created host(s) and symbiont(s).

After the organisms and their worlds have been created, you must design tests to ensure they are functioning as expected. Add a folder to `source/test` with the name `<name>_mode_test`. Inside of this folder, add testing source files corresponding to each organism. Make sure to also had files that test the interactions between organisms, as well as the data nodes. 
    - Include the testing file paths in `source/catch/main.cc`.
    - Add catch tags

Next, add a file to `source/native` with the name `symbulation_<name>.cc`. This is the main source file that will allow the experiment to function. In it, the world is created, set up according to the setup file, and is permitted to run for the specified number of updates.

Lastly, add the necessary targets to the Makefile for your new mode. Your new mode will need:
    - a compiling target, with the naming convention "<name>-mode"
    - a debug taget, with the naming convention "debug-<name>" and "<name>-debug"
    - a testing target, with the naming convention "test-<name>"
    - a debug while testing target, with the naming convention "test-debug-<name>"

Once you have created your organisms and corresponding world, designed their tests, added a main source file, and added targets to the Makefile, you are ready to experiment!