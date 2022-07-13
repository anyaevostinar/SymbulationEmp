# Creating a New Configuration Setting

Once you start making changes to Symbulation, you'll probably want to be able to control those changes from the command line at run time.
It is very easy to do so because Symbulation uses Empirical's configuration management system.
This guide walks you through the step-by-step process of adding and using a new configuration setting.

## Add Setting to ConfigSetup.h

The first step for adding a new configuration setting is to navigate to the configuration setup file `SymbulationEmp/source/ConfigSetup.h`.
This file is currently shared by all modes, but we're planning to eventually have separate files for each mode because it's already getting rather crowded!

In this file, you'll see that there are several groups.
You are probably safe putting your configuration setting into the `MAIN` group, unless it is a mutation rate or size.
If it is a mutation rate or size and you want it to be zeroed out during the "no mutation updates" at the end of an experiment, you should put it into the `MUTATION` group.
If you are making a lot of new configuration settings that are related to each other, you might want to have a group all to yourself.

The syntax for adding a new value is to just add a new line with the following:
```
VALUE(NAME_OF_SETTING, type, default_value, "English description of setting"),
```

For example, when I was creating a new limited pool of resources for the world, here is the configuration setting that I added:
```
VALUE(LIMITED_RES_TOTAL, int, -1, "Number of total resources available over the entire run, -1 for unlimited"),
```

It is common convention in Symbulation's configuration settings for -1 to be used to turn a feature off (so that we don't have to have two settings for each feature).

.. |fa-bug| raw:: html

   <i class="fa-solid fa-bug"></i>

|fa-bug| Bug alert: Make sure you don't forget the comma at the end!</i>

That's all you have to do to get your new configuration option in the configuration object that gets passed all around the code!

## Using and Changing the Setting

Now that you have your new configuration setting, you can access is pretty easily from most classes.
`Host`, `Symbiont`, and `SymWorld` (and therefore their subclasses) all have an instance variable pointing to the configuration object that they all share, generally called `my_config`.
The configuration class automatically provides both getters/accessors and setters/mutators for all the configuration settings!

To access your new configuration in `SymWorld`, `Host`, `Symbiont` or any of their subclasses:
```
type name_of_setting = my_config->NAME_OF_SETTING();
```

For example, in `SymWorld`, here is how the amount of total resources is accessed:
```
total_res = my_config->LIMITED_RES_TOTAL();
```

If you want to change the setting from what the configuration file specified, you can use the provided setter:

```
my_config->NAME_OF_SETTING(new_value);
```

This is particularly useful to do in test files, like so:
```
SymConfigBase config;
config.LIMITED_RES_TOTAL(150);
```

Remember that you can now also change your configuration setting on the command line (also handled for you automatically):
```
./symbulation_default -NAME_OF_SETTING value
```

It will also automatically show up in the configuration panel on the web GUI if it is in the `MAIN` group, though the GUI might not be able to show the settings effect depending on what it does. 

## Trying it out
The sky is the limit when it comes to new features that can be controlled with configuration settings.
Here are some ideas of things to practice with:
* Expand on the limited resources by having a small amount trickle into the world every update or based on chance or something about the population
* Change something about how hosts and symbionts interact with each other, such as how resources are distributed (in `Host::DistribResources`)
* Make various catastrophic events happen in the world at random or set times, such as a portion of the population dying