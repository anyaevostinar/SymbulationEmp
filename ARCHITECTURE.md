# Purpose and History

The Symbulation project is fairly large at this point, so this document is a guide aimed at getting started with the architecture of the entire codebase and the general principles and philosophy that should guide all of our future development.

This document aims to discuss at a high-level. You should consult the [API reference](https://symbulation.readthedocs.io/en/latest/api/index.html) for specific methods, etc. 

This document was originally written on 16th of June, 2026 by Anya Vostinar. Hopefully, it will continue to be updated and clarified!

# Overview

Symbulation is separated into several "modes". The active ones are:
* default: the original basic co-evolving host/symbiont functionality where interaction is determined by a floating point between -1 and 1
* sgp: the addition of assembly-like genomes and logical tasks similar to Avida for the purposes of exploring more complex interactions between hosts and symbionts. This mode will likely consume the inactive ones in the future.

Symbulation is strongly object-oriented, and so within each mode, you will find the relevant flavor of the following classes:
* Host
* Symbiont
* World

The actual creation and running of an experiment occurs within a `native/symbulation_x.cc` file. 

Symbulation has a large amount of tests in the `test` folder. Each mode has its own tests for unit (specific methods) and functional (interactions between methods) tests. There is additionally an `integration_test` folder for tests that capture results from published experiments.

## SGP Mode
Because of the additional functionality of SGP Mode, it brought in a large amount of complexity. This subsection aims to provide a roadmap for the core functionality of SGP Mode.

![Overview figure](https://docs.google.com/drawings/d/e/2PACX-1vRi89yMMLF-7yPNPHa5jKs2VwJbNeI95I1wN_NVUSA4mDoU70Xy2cqRg3kfC6lOeEbZga1vyJe9ouXo/pub?w=1440&amp;h=1080)

* `native/symbulation_sgp.cc` is what is compiled when you type `make sgp-mode` on the command line. It creates a configuration object from whatever command line arguments you entered along with what is in your `SymSettings.cfg` file. It then creates a world, sets it up based on your configuration settings, and runs the experiment. This file is an ideal place to do further customization of output that can't be handled by the `.cfg` or `.json` files.
* `SGPWorld` manages everything. The heart of its functionality is a population of `SGPHost`s. There could additionally be a population of free-living `SGPSymbiont`s, if you have enabled that setting. To manage these evolving populations, the world uses many helper objects, such as a   `Scheduler`, `ReproductionQueue`, `LogicTaskEnvironment`, `Mutator`, many `DataMonitors`, etc. Because the world is so large, it has many functions that are defined in helper files. 
    * `SGPWorldSetup.cc` defines the main `Setup` method, which defines all necessary functors and otherwise sets up the world based on the configuration file. Because it is very large itself, it is further broken up.
        * `SGPW_InteractionMechanismSetup.cc` defines all helper methods related to Nutrient, Stress, or Health interactions. All methods particular to those types of interactions should be defined here.
        * `SGPW_TaskProfileSetup.cc` defines all helper methods related to task performance and matching; specifically, how tasks affect organism interactions such as transmission compatibility.
        * `SGPWorldData.cc` defines helper methods that create SGP specific data files (many data files are already handled by default mode)
    * `SGPWorld.cc` defines methods that can't be defined in the header due to specifics of the methods. This generally includes many of the SGP-specific organism methods, such as those related to host or symbiont birth and processing.
* `SGPHost` manages the internals of each individual host. This can include a list of a host's endosymbionts. It also includes an `SGPHardware` that in turn manages all the "virtual CPU" related objects.
* `SGPSymbiont` manages the internals of each individual symbiont (free-living or endosymbiont). It also includes an `SGPHardware`.
* `SGPHardware` contains many objects imported from the signalgp-lite library such as a CPU, genome, jump table, etc. It also contains a separate `CPUState` object for maintaining additional state.
* `CPUState` contains the organism's input buffer (a `RingBuffer`) and output buffer, along with the necessary stacks and registers for Turing-complete computation and tracking variables for an individual's task performance etc. The reason for this separate state separate from the `SGPHost` and `SGPSymbiont` state is because it is all shared between them and we are currently trying to avoid multi-inheritance (since they both inherit from their default-mode counterparts already and so can't easily also inherit from an `SGPOrganism`, though we may explore that in the future).

# Philosophy
Here, we attempt to write down the general principles that have guided our development decisions up to this point. We don't always stick to these, but we want to, and we hope that writing them down will help facilitate that.

1. We love the object-oriented programming philosophy and generally try to utilize it to structure the organisms and their components because it helps make sense of this complex set of programs.
    1. If an event occurs strictly "within" a host, that code should be in `SGPHost` or one of its components, even if that means that `SGPHost` needs to call some `SGPWorld` methods to get the job done. Same for a symbiont.
    2. If an event occurs between a single host and symbiont, that code should be in one or the other, depending on which is initiating or otherwise conceptually "owns" the event.
    3. If an event occurs between more than one host or free-living symbiont (i.e. at the population level conceptually), then it likely should be handled by the World, such as host reproduction and horizontal transmission from one host to another.
    4. Events handled by the world should still try to call host and symbiont methods to enable those classes to handle their own internal state as much as possible.
2. We leverage functors and signals to allow for easier hooking into core events to customize experimental setup based on configuration settings.
    1. Signals should be held by the World both because they apply to many different organisms, and so don't belong to any single organisms, and because this avoids them needing to be continuous recreated whenever an organism is born.
3. We aim for many short and well-named methods along with longer (if necessary) and clear variable names. We want individual components of the codebase to be highly readable, such that a newcomer is able to start reading a relevant section and understand what that small part does at least.
4. We like efficient code, but we would rather have understandable code even if it means small efficiency sacrifices. We believe it is always possible to have both efficient and readable code, but it might take us some additional effort.
5. We believe in the value of thorough documentation and aim for 100% correct and complete documentation.
4. We shun code duplication in production code because we have been burned too many times before. We accept code duplication in test code to ensure clear and independent tests.
3. We embrace refactoring as a core tenet. If code is confusing or not serving its purpose, we either make it better or make a note that it needs to be improved.
4. We aim for 100% code coverage but recognize that it's not always practical. Because we love refactoring, we definitely want coverage on code that is likely to be refactored. 
5. When we do find a bug, we write a test for it.
6. We accept that it will take more time to write better code and are willing to put in that time. However, we also would rather have code that is done and imperfect than never finished (because no code is perfect and that's what refactoring is for). Therefore, the time investment is a collaborative investment; we don't place that time burden on any one individual and welcome whatever gifts of time an individual is willing to give. 
7. We aim for our tests to be an additional form of documentation of how functionality works. Therefore, we aim for our tests to be readable and discoverable. To facilitate discoverability, we organize them (in sgp-mode) into `unit` and `functional` and have an overall `integration` level. Unit tests are for testing a specific method and should be named based on the method that they are testing. Functional tests are for testing important sets of interacting methods and should be named either based on the interacting units or with a descriptive name that captures the focus of the test. Integration tests are for testing full experimental results in an effort to avoid accidentally destroying replicability of previous published results.
