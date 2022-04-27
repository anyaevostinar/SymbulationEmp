# Integration Testing

This folder contains tests that ensure that the major results of symbulation are still seen, 
even when there are new additions to the softare.

The following list of folders has a link to the folder, a link to the research paper for the correlated experiments, and its abstract.

## [SpatialStructure](https://github.com/anyaevostinar/SymbulationEmp/tree/main/source/test/integration_test/spatial_structure)

[Spatial Structure Can Decrease Symbiotic Cooperation](https://doi.org/10.1162/artl_a_00273):
Mutualisms occur when at least two species provide a net fitness benefit to each other. 
These types of interactions are ubiquitous in nature, with more being discovered regularly. 
Mutualisms are vital to humankind: Pollinators and soil microbes are critical in agriculture, bacterial microbiomes regulate our health, and domesticated animals provide us with food and companionship. 
Many hypotheses exist on how mutualisms evolve; however, they are difficult to evaluate without bias, due to the fragile and idiosyncratic systems most often investigated. 
Instead, we have created an artificial life simulation, Symbulation, which we use to examine mutualism evolution based on (1) the probability of vertical transmission (symbiont being passed to offspring) and (2) the spatial structure of the environment. 
We found that spatial structure can lead to less mutualism at intermediate vertical transmission rates. 
We provide evidence that this effect is due to the ability of quasi species to purge parasites, reducing the diversity of available symbionts. 
Our simulation is easily extended to test many additional hypotheses about the evolution of mutualism and serves as a general model to quantitatively compare how different environments affect the evolution of mutualism.

## [Lysogeny](https://github.com/anyaevostinar/SymbulationEmp/tree/main/source/test/integration_test/lysogeny)

[Keep Your Frenemies Closer: Bacteriophage That Benefit Their Hosts Evolve to be More Temperate](https://doi.org/10.32942/osf.io/3qcwk):
Bacteriophages, also known as phages, are viruses that infect bacteria. 
They are found everywhere in nature, playing vital roles in microbiomes and bacterial evolution due to the selective pressure that they place on their hosts. 
As obligate endosymbionts, phages depend on bacteria for successful reproduction, and either destroy their hosts through lysis or are maintained within the host through lysogeny. 
Lysis involves reproduction within the host cell and ultimately results in the disruption or bursting of the cell to release phage progeny. 
Alternatively, lysogeny is the process by which phage DNA is incorporated into the host DNA or maintained alongside the host chromosome, and thus the phage reproduces when their host reproduces.
Recent work has demonstrated that phages can exist along the parasitism-mutualism spectrum, prompting questions of how phage would evolve one reproductive strategy over the other, and in which conditions.
In this work, we present an agent-based model of bacteriophage/bacterial co-evolution that enables lysogenized phage to directly impact their host's fitness by using the software platform Symbulation.
We demonstrate that a viral population with beneficial lysogenic phage can select against lytic strategies. 
This result has implications for bottom-up control of vital ecosystems. 