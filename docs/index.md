# Symbulation -- Agent based modeling of symbiont ecology and evolution
% MyST does not yet have support for field lists, so we need to use rST here
% see https://github.com/executablebooks/MyST-Parser/issues/163
```{eval-rst}
:Authors: Anya Vostinar and contributors. 
:GitHub: https://github.com/anyaevostinar/SymbulationEmp
```

This is a software model of symbiosis which allows for evolution of
parasitism and mutualism.

Go to
https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html to
try it out in your web browser!

To build, you must first download Empirical
(https://github.com/devosoft/Empirical) and place the two repositories
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


Contents:
#### Getting Started 
* [Quick start guide]()
* [Library]()

#### Contributing to Symbulation


* [Sumbulation development practices]()
* [Getting started]()
* [Contribution guidelines and review]()
* [Adding documentation]()
* [Guide to testing]()


{ref}`genindex`
