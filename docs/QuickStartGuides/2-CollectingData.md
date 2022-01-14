# Collecting Data with Symbulation 
You are able to run Symbulation with whatever workflow you prefer to run many replicates. 
The following assumes that you used the [Symbulation Cookiecutter](https://github.com/anyaevostinar/SymbulationProjectTemplate) to get setup.

We've provided a [short script](https://github.com/anyaevostinar/SymbulationProjectTemplate/blob/main/%7B%7Bcookiecutter.project_name%7D%7D/Data/sample_treatment/simple_repeat.py) that can be used with `screen` to run several replicates and treatments.

We recommend (and have provided) a workflow where you have a `Data` folder that contains subfolders for each experiment and within each of those subfolders are:

* `simple_repeat.py`
* `SymSettings.cfg`
* Your executable file
* A `README.md` containing the date and the purpose of the experiment

Assuming that you are in the `SymbulationEmp` directory and have already compiled your `symbulation` or `symbulation_default` executable, copy your executable to your `Data` folder and change to that directory:

```
cp symbulation ../Data/sample_treatment
cd ../Data/sample_treatment
```

The `simple_repeat.py` script assumes that you already have a copy of the executable and `SymSettings.cfg` in the same directory. 
Within that directory, you can run `simple_repeat.py`:
```
python3 simple_repeat.py
```

By default, this will run 5 replicates of each treatment specified in `simple_repeat.py` and use the random seeds 21-25 (inclusive). 
You can specify the random seeds (and therefore also the number of replicates) using command line arguments, which are optional. 
The first command line argument is the start of the range of seeds (inclusive), and the second command line argument is the end of the range of seeds (exclusive). 

For example, the input
```shell
python3 simple_repeat.py 10 15
```
will use seeds 10, 11, 12, 13, and 14. 

# Analyzing Data
We've also provided a basic analysis pipeline for visualizing your data.
Once you have let `simple_repeat.py` run, you can change directory to the `Analysis` folder:

```
cd ../../Analysis/sample_treatment
```

and run our provided Python script:
```
python3 munge_data.py
```

These commands will output a file `munged_basic.dat` that contains the average *interaction value* of hosts and symbionts over time in each of your replicates and treatments.

You can then open the R script `SampleAnalysis.R`, set your working directory to the `Analysis` folder and run all of the lines to see a plot of the effect of vertical transmission on the evolved interaction value for hosts and symbionts.