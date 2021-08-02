# Collecting Data with Symbulation 
After you have code you are ready to collect data with, you should utilize simple_repeat.py. 

In order to run simple repeats, you will first need to create the directory where the data 
files will be sent to, which must be at the top level of Symbulation. 

Then navigate to that directory from the terminal. Once you are inside of the data folder,
you can run simple repeats. For example,

```shell
~/SymbulationEmp/<DATA_FOLDER_NAME>$ python3 ../stats_scripts/simple_repeat.pyt
```
By default, this will use the random seeds 10-20 (inclusive). 
You can specify the random seeds using command line arguments, which are optional. T
he first command line argument is the start of the range of seeds (inclusive), and the second command line argument is the end of the range of seeds (exclusive). 
The second command line argument is also optional, giving the user the ability to use just one seed at a time. 

For example, the input
```shell
~/SymbulationEmp/<DATA_FOLDER_NAME>$ python3 ../stats_scripts/simple_repeat.py 10 15
```
will use seeds 10, 11, 12, 13, and 14. But the input:

```shell
~/SymbulationEmp/<DATA_FOLDER_NAME>$ python3 ../stats_scripts/simple_repeat.py 10
```

will only use seed 10
