# Symbulation -- Agent based modeling of symbiont ecology and evolution
% MyST does not yet have support for field lists, so we need to use rST here
% see https://github.com/executablebooks/MyST-Parser/issues/163
```{eval-rst}
:Authors: Anya Vostinar and contributors. 
:GitHub: https://github.com/anyaevostinar/SymbulationEmp
```

This is a software model of symbiosis which allows for evolution of
parasitism and mutualism.

Click [here](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html) to
try it out in your web browser!

To build, you must first download Empirical
([https://github.com/devosoft/Empirical](https://github.com/devosoft/Empirical)) and place the two repositories
on the same level. Then within SymbulationEmp run
```
  make
  ./symbulation
```

The script 'stats\_scripts/simple\_repeat.py' can be
used to run many replicates and treatments.

The current settings can be configured in SymSettings.cfg.

To run end-to-end tests run 'make test'. This will overwrite the default
SymSettings.cfg!

```{toctree}
:caption: Using Symbulation
:maxdepth: 2

QuickStartGuides/index
library/index
api/library_root

```

```{toctree}
:caption: Contributing to Symbulation
:maxdepth: 0

dev/Symbulation-development-practices
dev/getting-started
dev/contribution-guidelines-and-review
dev/adding-documentation
dev/guide-to-testing
```

{ref}`genindex`
Contents:
#### Getting Started 
* [Quick start guide]() - Coming soon! 
* [Library]() - Coming soon! 

#### Contributing to Symbulation


* [Symbulation development practices]() - Coming soon! 
* [Getting started]() - Coming soon! 
* [Contribution guidelines and review]() - Coming soon! 
* [Adding documentation]() - Coming soon! 
* [Guide to testing]() - Coming soon! 


{ref}`genindex`
