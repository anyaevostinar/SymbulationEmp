![Symbulation Logo](https://github.com/anyaevostinar/SymbulationEmp/blob/main/SymbulationLogo.png "Symbulation")

[![CI](https://github.com/anyaevostinar/SymbulationEmp/actions/workflows/CI.yml/badge.svg)](https://github.com/anyaevostinar/SymbulationEmp/actions/workflows/CI.yml)
[![codecov](https://codecov.io/gh/anyaevostinar/SymbulationEmp/branch/main/graph/badge.svg?token=BVQUX9SK5S)](https://codecov.io/gh/anyaevostinar/SymbulationEmp)
[![DOI](https://zenodo.org/badge/92536524.svg)](https://zenodo.org/badge/latestdoi/92536524)
[![Documentation Status](https://readthedocs.org/projects/symbulation/badge/?version=latest)](https://symbulation.readthedocs.io/en/latest/?badge=latest)


This is a software model of symbiosis which allows for evolution of parasitism and mutualism. 

Go to [https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html) to try it out in your web browser!

Documentation at [https://symbulation.readthedocs.io](https://symbulation.readthedocs.io).


We have a cookiecutter template here: [https://github.com/anyaevostinar/SymbulationProjectTemplate](https://github.com/anyaevostinar/SymbulationProjectTemplate).
To build, you must first download Empirical (https://github.com/devosoft/Empirical) and place the two repositories on the same level. Then within SymbulationEmp, run 'make' and then './symbulation'. The script 'stats_scripts/simple_repeat.py' can be used to run many replicates and treatments.

The current settings can be configured in SymSettings.cfg. 

To run end-to-end tests run 'make test'. This will overwrite the default SymSettings.cfg!

Created part of my free logo at LogoMakr.com.
