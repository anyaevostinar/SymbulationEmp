![Symbulation Logo](https://github.com/anyaevostinar/SymbulationEmp/blob/main/SymbulationLogo.png "Symbulation")

Symbulation is an artificial life software tool for agent-based modeling of the evolution of biological symbiosis, which can occur along the spectrum between parasitism and mutualism. 

[![CI](https://github.com/anyaevostinar/SymbulationEmp/actions/workflows/CI.yml/badge.svg)](https://github.com/anyaevostinar/SymbulationEmp/actions/workflows/CI.yml)
[![codecov](https://codecov.io/gh/anyaevostinar/SymbulationEmp/branch/main/graph/badge.svg?token=BVQUX9SK5S)](https://codecov.io/gh/anyaevostinar/SymbulationEmp)
[![DOI](https://zenodo.org/badge/92536524.svg)](https://zenodo.org/badge/latestdoi/92536524)
[![Documentation Status](https://readthedocs.org/projects/symbulation/badge/?version=latest)](https://symbulation.readthedocs.io/en/latest/?badge=latest)
[<img src="https://img.shields.io/badge/template-cookiecutter-blueviolet.svg?logo=LOGO">](https://github.com/anyaevostinar/SymbulationProjectTemplate)

Try out our [browser-based GUI](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html).

Documentation at [https://symbulation.readthedocs.io](https://symbulation.readthedocs.io).

Interested in starting a new project with Symbulation? We have a cookiecutter template here: [https://github.com/anyaevostinar/SymbulationProjectTemplate](https://github.com/anyaevostinar/SymbulationProjectTemplate).
Symbulation is built with the [Empirical platform](https://github.com/devosoft/Empirical) and the cookiecutter includes our recommended directory layout and example analysis files. 

## Usage
Install the latest Cookiecutter:

```
pip install -U cookiecutter
```

Generate an Symbulation project:

```
cookiecutter https://github.com/anyaevostinar/SymbulationProjectTemplate.git
```

Move into the new project directory and the `SymbulationEmp` directory and make:
```
cd SymbulationProject/SymbulationEmp
make
```

Make any changes that you wish to the file `SymSettings.cfg` and then run:
```
./symbulation
```

By default, your data will be output to the files `HostVals_data__SEED10.data` and `SymVals_data__SEED10.data`.

We recommend that you copy your `symbulation` executable to your `Data` folders:
```
cp symbulation ../Data/sample_treatment
cd ../Data/sample_treatment
```

You can then use the provided Python script to run several replicates:
```
python3 simple_repeat.py
```

You can also then use the provided Python script to transform your data into a format more easily used by R:
```
cd ../../Analysis/sample_treatment
python3 munge_data.py
```

And then open our provided R script `SampleAnalysis.R` in R, set your working directory to the `Analysis` directory and then run all lines to observe the effect of vertical transmission rate on the interaction value evolved.

Created part of my free logo at LogoMakr.com.
