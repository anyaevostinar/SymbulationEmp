lines = open('collated_data.data').readlines()

header = [x.strip('"') for x in lines[0].split()]

independent_variables = 'HRR', 'HRR', 'SLR', 'BS', 'BT', 'SL', 'SYN', 'POP', 'UPS', 'T'
dependant_variables = 'host_count', 'sym_count', 'survival', 'moi'
omit = ('file_name',)
versions = '"Standard-1.1"', '"Duplicate-1.1"'

data = {}
passes, fails = 0, 0
for line in lines:
    values = line.split()
    if values[0] in versions:
        independent_values = []
        dependant_values = []
        other_values = []
        for name, value in zip(header, values):
            if name in independent_variables:
                independent_values.append(value)
            elif name in dependant_variables:
                dependant_values.append(value)
            elif name not in omit:
                other_values.append(value)
        independent_values = tuple(independent_values)
        dependant_values = tuple(dependant_values)
        other_values = tuple(other_values)
        
        if independent_values in data:
            if data[independent_values][0] != dependant_values:
                print(independent_values)
                print(data[independent_values][0])
                print(dependant_values)#, other_values)
                print()
                fails += 1
            else:
                passes += 1
        else:
            data[independent_values] = [dependant_values]
        data[independent_values].append(other_values)


print('Fail:' if fails else 'Pass:' if passes else 'No Data:', '{}/{}'.format(passes, passes+fails))
