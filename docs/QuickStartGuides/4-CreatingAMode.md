# Creating A New Mode

*Note: this guide is in progress and does not contain all helpful details.*

Once you have familiarized yourself with Symbulation and its default mode, you might be interested in designing your own experiment, adding functionality to the codebase, and collecting data.
This guide explains how you can add a new *mode* to Symbulation, which is preferred when you have more new functionality that you want to add than can be handled by a couple of configuration settings.
The guide assumes that you are familiar with the ideas of inheritance and classes. 
It gives a fair amount of assistance on how to do those in C++, though familiarity with general syntax of C++ is assumed.

There are a lot of different ways that symbiosis can occur that have overlaps but are functionality different.
With Symbulation, we want to both support lots of those different ways of implementing symbiosis while keeping any one approach from getting too cluttered and bogged down with code that isn't relevant.
We also don't want to repeat a bunch of code that is shared between those different ways (which we don't always succeed at, but we're constantly improving the codebase!).
We achieve these dual goals with *modes*.

A mode generally has its own subclasses of `Symbiont` and/or `Host` and `SymWorld` defined so that they can be highly customized to the particular functionality of the mode. 
It also has its own compile target and test suite.
The existing modes include:
* `default` (`Host` and `Symbiont` in the "classic" Symbulation functionality of interaction or resource behavior values), 
* `efficient` (`EfficientHost` and `EfficientSymbiont` for work on the Dirty Transmission Hypothesis), 
* `lysis` (`Bacterium` and `Phage` to study bacteriophage/bacterial-specific dynamics), 
* and public goods game or `pgg` (`PGGHost` and `PGGSymbiont` to study ways for symbionts to interact within a host). 
* We're also working a new mode `sgp` that uses the SignalGP-Lite library to have hosts and symbionts with computer programs as their genomes! You can check out our progress on the `complex-genomes` branch.

If you want to create new subclasses of `Host`, `Symbiont`, or `SymWorld`, a new mode is the best way to go.
There are several steps to creating your own mode, including following conventions for file structure, adding your own organisms, adding a WorldSetup file, adding targets to the makefile, designing tests, and more. 
This guide will walk you through how to properly add most of these features.

<!-- First, you must decide if your new mode falls under one of two scenarios. 
In the first scenario, you wish to change the processes of an existing organism, but will not be adding any new traits. 
The second scenario includes the addition of new traits, or genome values, and will therefore also require functions that track the evolution of this new trait. 
Depending on the goals of your project and which scenario it falls under (1 or 2), you will need to add to the codebase in a varying manner. 
Most sections below pertain to both scenarios and should be completed no matter what. Sections that are specific to a particular scenario will be labeled accordingly. -->

## Primary Folder
Choose a one word descriptor for your new mode. 
Then make a folder in `SymbulationEmp/source` named `<name>_mode`. 
Inside of this folder will be a world setup source file (explained in more detail later), as well as any necessary header files such as your new subclasses.

## Organisms
Next, you will need to add new header files for your new organisms. 
The host organism(s) must extend the `Host` class in the `source/default_mode`, and similarly, the symbiont organism(s) must extend the `Symbiont` class. 
We'll go through how to make a new `Symbiont` subclass, but the process would be the same for the `Host` subclasses.
Specifically, we'll use the [`EfficientSymbiont`](https://github.com/anyaevostinar/SymbulationEmp/blob/main/source/efficient_mode/EfficientSymbiont.h) as an example since it's a fairly simple class.

Each new class must contain the following (which we'll go into depth below):
* `ifndef` macro
* constructor(s)
* `MakeNew()`

If you are adding a new trait or component of the genome, you will also need to:
* Define the relevant genome/instance variables
* Define a custom `Mutate()` 
* Overwrite whichever super class methods your new trait is impacting

You'll probably also want to add some [configuration settings](3-CreatingConfigSetting) to alter relevant things about your mode at runtime.

### Macros
Because there is a whole lot of `include`-ing going on, you should first put the following into your file to make sure it doesn't get added more than once:

```
#ifndef NAMEOFCLASS_H
#define NAMEOFCLASS_H

//All your code will go here

#endif
```

For example, here is the one from `EfficientSymbiont.h`:
```
#ifndef EFFSYM_H
#define EFFSYM_H

...

#endif
```

### Includes
Of course, you'll be referring to some other files, so you'll need to include them.
The most obvious will be either `Host.h` or `Symbiont.h` depending on which you are inheriting from.
Since `EfficientSymbiont` also knows about the new world type and host type, here are the includes from it:

```
#include "../default_mode/Symbiont.h"
#include "EfficientWorld.h"
#include "EfficientHost.h"
```

### Class definition
Now it's time to define your new class!
To declare a class that inherits from another class you use the following syntax:
```
class YOUR_CLASS: public SUPER_CLASS {
    //All your class code here
}
```

For example:
```
class EfficientSymbiont: public Symbiont {
    //All the class code here
}
```

You can then add whatever new instance variables you want your class to have.
Remember that your class will inherit the instance variables from the superclass, so you don't need to declare any of those.

For example, here are some of `EfficientSymbiont`'s new instance variables, with documentation removed:
```
protected:
  double efficiency;
  double ht_mut_size = 0.002;
  double ht_mut_rate = 0;
  double eff_mut_rate = 0;
```

### Constructor(s)
You'll probably want to define a constructor since it's unlikely the default is what you want.
Your constructor needs to take parameters that the superclass will need as well as any additional parameters that you want. 
Here is one way of structuring your constructor for a `Symbiont` subclass, which calls the `Symbiont` constructor for you:
```
YourClassName(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0) : Symbiont(_random, _world, _config, _intval, _points) {
    //Your specific code here
}
```

Alternatively for a `Host` subclass, which needs a few more things:
```
YourClassName(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config,
  double _intval =0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
  emp::vector<emp::Ptr<Organism>> _repro_syms = {},
  std::set<int> _set = std::set<int>(),
  double _points = 0.0) :
  Host(_random, _world, _config, _intval, _syms, _repro_syms, _set, _points) {
    //Your specific code here
  }
```

You may also want to specify copy and move constructors, though they generally aren't used in Symbulation and can usually just be the default.

### MakeNew()

Because Symbulation uses a whole lot of inheritance and makes a whole lot of new objects of different subclasses through reproduction events, we rely on every class defining a `MakeNew()` method which can be used to make a new organism of the subclass without needing to know the type of the object. 
This method **should not** handle mutations, just make a new organism that is a clone of the current organism, wrapped in an Empirical pointer, and return it.
It seems very simple, and it is, but it has allowed us to reduced a lot of repeated code between modes! 

Here is the basic structure:
```
emp::Ptr<Organism> MakeNew(){
    emp::Ptr<YourClassName> baby = emp::NewPtr<YourClassName>(random, my_world, my_config, GetIntVal());
    //Setting your specific traits if needed
    return baby;
  }
```

Here is an example from `EfficientSymbiont`:
```
  emp::Ptr<Organism> MakeNew(){
    emp::Ptr<EfficientSymbiont> sym_baby = emp::NewPtr<EfficientSymbiont>(random, my_world, my_config, GetIntVal());
    sym_baby->SetInfectionChance(GetInfectionChance());
    sym_baby->SetEfficiency(GetEfficiency());
    return sym_baby;
  }
```

### (Optional) Creating new traits
If you are interested in adding a new heritable trait to your organism class, you will need to follow the next couple of optional sections. 
If you aren't adding a new heritable trait, you can skip them!

If you want your organism to have a new trait in its genome, you will need to add an instance variable for it up in the `protected` section and should probably add a `Get` and `Set` method as well.

For example, `EfficientSymbiont`s have a new heritable trait called `efficiency` that is just how efficient they are at actually getting points.
There is an instance variable:
```
double efficiency;
```

That instance variable is set in the constructor:
```
efficiency = _efficient;
```

And they have a getter and setter for it:
```
  void SetEfficiency(double _in) {
    if(_in > 1 || _in < 0) throw "Invalid efficiency chance. Must be between 0 and 1 (inclusive)";
    efficiency = _in;
  }

  double GetEfficiency() {return efficiency;}
```

Then, it is used in the overwritten version of the `AddPoints` method:
```
void AddPoints(double _in) {points += (_in * efficiency);}
```

If you add a heritable trait, don't forget to make a new `Mutate` method for it (see below) and include it in the `MakeNew` method!

### (Optional) Mutate
If you are making a new heritable trait, you need to also specify how it should be mutated during reproduction.

You can either completely toss out the `Mutate` method of the superclass, as `EfficientSymbiont` does because it is testing some different mutation approaches, or you can just add on to the existing functionality.
Since `EfficientSymbiont` is a bit complicated in this regard, we're going to switch examples to [`Phage`](https://github.com/anyaevostinar/SymbulationEmp/blob/main/source/lysis_mode/Phage.h).
The `Phage` class has a couple of new heritable traits, but it also wants to keep mutating the original `Symbiont` traits without needing to repeat that code.

Here is the general structure for doing that by first calling the superclass method, and then doing your own logic:
```
void Mutate() {
    Symbiont::Mutate(); //or Host::Mutate(); for those subclasses
    double local_rate = my_config->MUTATION_RATE();
    double local_size = my_config->MUTATION_SIZE();
    //Repeat the below for each new trait that needs mutating
    if (random->GetDouble(0.0, 1.0) <= local_rate) {
      //mutate trait, assuming it should be between 0 and 1
      trait += random->GetRandNormal(0.0, local_size);
      if(trait < 0) trait = 0;
      else if (trait > 1) trait = 1;
    }
  }
```

The `Phage` class has three new traits and has configuration settings for turning mutation for each of those traits on or off, as you can see in the example:
```
  void Mutate() {
    Symbiont::Mutate();
    double local_rate = my_config->MUTATION_RATE();
    double local_size = my_config->MUTATION_SIZE();
    if (random->GetDouble(0.0, 1.0) <= local_rate) {
      //mutate chance of lysis/lysogeny, if enabled
      if(my_config->MUTATE_LYSIS_CHANCE()){
        chance_of_lysis += random->GetRandNormal(0.0, local_size);
        if(chance_of_lysis < 0) chance_of_lysis = 0;
        else if (chance_of_lysis > 1) chance_of_lysis = 1;
      }
      if(my_config->MUTATE_INDUCTION_CHANCE()){
        induction_chance += random->GetRandNormal(0.0, local_size);
        if(induction_chance < 0) induction_chance = 0;
        else if (induction_chance > 1) induction_chance = 1;
      }
      if(my_config->MUTATE_INC_VAL()){
        incorporation_val += random->GetRandNormal(0.0, local_size);
        if(incorporation_val < 0) incorporation_val = 0;
        else if (incorporation_val > 1) incorporation_val = 1;
      }
    }
  }
```

### (Optional) World Class and Data Nodes
If you have added new evolvable traits to the organisms, you will probably want to find out information about those traits.
You may also want to change how the environment impacts the organisms, even if you didn't make new inheritable traits.
In either case, you'll need to create a new "world" class that inherits from `SymWorld` or one of its subclasses. 

For example, here is the start of the [`EfficientWorld`](https://github.com/anyaevostinar/SymbulationEmp/blob/main/source/efficient_mode/EfficientWorld.h) class:

```
#ifndef EFFWORLD_H
#define EFFWORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"

class EfficientWorld : public SymWorld {
//All new code here
}
```

Empirical provides a powerful data-tracking framework that works with the world classes, so there is only a bit of setup that you need to do to track and output data from your experiment.
We're going to focus on data collection here, but of course if you want to change how the environment interacts with the organisms, you can do that by overwriting `SymWorld` methods in this class as well.

You'll need to first make a new instance variable for your [`DataMonitor`](https://empirical.readthedocs.io/en/latest/api/typedef_namespaceemp_1aaf92a6b9dd531666c3e3da2b888c29ba.html). A data monitor is a special version of an Empirical [`DataNode`](https://empirical.readthedocs.io/en/latest/api/classemp_1_1DataNode.html) that is focused on the data that it most recently received and the distribution of values that it has received previously, so it's useful for tracking what the data looks like every so many updates. 
There is a huge amount of power in the `DataNode` functionality that we are ignoring, so if there are different ways that you'd like to track your data, go take a look at Empirical's documentation!

We want our data instance variables to look like this:
```
emp::Ptr<emp::DataMonitor<type>> data_node_name;
```

For example, here is the new data monitor for tracking the efficiency trait of `EfficientSymbiont`s:
```
/**
    *
    * Purpose: Data node tracking the average efficiency of efficient symbionts.
    *
  */
  emp::Ptr<emp::DataMonitor<double>> data_node_efficiency;
```

Before we go further, we should make sure to clean up our memory, so let's define the world destructor to delete the data node:
```
~EfficientWorld(){
      if (data_node_efficiency) data_node_efficiency.Delete();
  }
```

Next, we need to define what data is going to be stored in the data node.
The data nodes use anonymous functions and event triggers so the world knows to add to them whenever specific events happen.
The most common event that we want to add data during is Update, so we'll have a template that looks like this for defining what our data node does:
```
emp::DataMonitor<type>& GetNAMEDataNode() {
    if (!data_node_name) { //if the data node doesn't already exist, make it!
      data_node_name.New();
      OnUpdate([this](size_t){ //this is the anonymous function that is called every update
        data_node_name->Reset(); //assuming you want to clear out data from the previous update
        for (size_t i = 0; i< pop.size(); i++) { // go through the population
          if (IsOccupied(i)) { //check that there is an organism
            data_node_name->AddDatum(pop[i]->GetData()); //add whatever data to the data node
          }//close if
      }//close for
      });
    }
    return *data_node_efficiency; //hand back the data node that's been created
  }
```

As you can see from the inline comments, we are making a method that makes the data node if it doesn't already exist. 
When creating it, we add an unnamed function to the world's OnUpdate to-do list to go through the population and get the information that we want about each of our organisms, which we then add to the data node.

To get data out of the data node and into a file, we use Empirical's `DataFile` class.
A datafile is generally initialized in the `.cc` file for your mode, which we'll discuss below.
However, we need a method in the `World` class to actually setup the datafile and tell it what it will be doing.
Here is the general structure of that method:
```
/**
   * Input: The address of the string representing the file to be
   * created's name
   *
   * Output: The address of the DataFile that has been created.
   *
   * Purpose: To set up the file that will be used to track YOUR TRAIT
   */
  emp::DataFile & SetupTRAITFile(const std::string & filename) {
    auto & file = SetupFile(filename); //A method from the Empirical World class
    auto & node = GetNAMEDataNode(); //The method you made previously
    file.AddVar(update, "update", "Update"); //You'll usually want the update information
    file.AddMean(node, "mean_TRAIT", "Average TRAIT", true);
    file.PrintHeaderKeys();

    return file;
  }
```

Empirical's [datafiles have many statistical methods already available](https://empirical.readthedocs.io/en/latest/library/data/data.html) including all the different flavors of averages, total, min/max, variance, skew, kurtosis, histogram bins, and ways for you to easily add new calculations.
You can have multiple datanodes pulled from in the same file if you wish as well.



### (Optional) World Setup
If you've made new organism(s) and a world, you'll need a new `WorldSetup` file.
The primary difference should be the organism types added to the world, which should now be your newly created host(s) and symbiont(s).


The following is still in development.
## Native File
Next, add a file to `source/native` with the name `symbulation_<name>.cc`. 
This is the main source file that will allow the experiment to function. 
In it, the world is created, set up according to the setup file, and is permitted to run for the specified number of updates.

## Makefile
Lastly, add the necessary targets to the Makefile for your new mode. Your new mode will need:

- a compiling target, with the naming convention "<name>-mode"

Advanced targets:
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