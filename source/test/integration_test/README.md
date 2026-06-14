# Integration Testing

This folder contains tests that ensure that the major results of symbulation are still seen, 
even when there are new additions to the softare.

The following list of folders has a link to the folder, a link to the research paper for the correlated experiments, and its abstract.

## [spatial_structure](https://github.com/anyaevostinar/SymbulationEmp/tree/main/source/test/integration_test/spatial_structure)

[Spatial Structure Can Decrease Symbiotic Cooperation](https://doi.org/10.1162/artl_a_00273):
Mutualisms occur when at least two species provide a net fitness benefit to each other. 
These types of interactions are ubiquitous in nature, with more being discovered regularly. 
Mutualisms are vital to humankind: Pollinators and soil microbes are critical in agriculture, bacterial microbiomes regulate our health, and domesticated animals provide us with food and companionship. 
Many hypotheses exist on how mutualisms evolve; however, they are difficult to evaluate without bias, due to the fragile and idiosyncratic systems most often investigated. 
Instead, we have created an artificial life simulation, Symbulation, which we use to examine mutualism evolution based on (1) the probability of vertical transmission (symbiont being passed to offspring) and (2) the spatial structure of the environment. 
We found that spatial structure can lead to less mutualism at intermediate vertical transmission rates. 
We provide evidence that this effect is due to the ability of quasi species to purge parasites, reducing the diversity of available symbionts. 
Our simulation is easily extended to test many additional hypotheses about the evolution of mutualism and serves as a general model to quantitatively compare how different environments affect the evolution of mutualism.

## [lysogeny](https://github.com/anyaevostinar/SymbulationEmp/tree/main/source/test/integration_test/lysogeny)

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

## [endosymbiosis](/endosymbiosis)

[Endosymbiosis or Bust: Influence of Ectosymbiosis on Evolution of Obligate Endosymbiosis](https://doi.org/10.32942/osf.io/yzce3):
Endosymbiosis, symbiosis in which one symbiont lives inside another, is woven throughout the history of life and the story of its evolution. 
From the mitochondrion residing in almost every eukaryotic cell to the gut microbiome found in every human, endosymbiosis is a cornerstone of the biological processes that sustain life on Earth. 
While endosymbiosis is ubiquitous, many questions about its origins remain shrouded in mystery; one question in particular regards the general conditions and possible trajectories for its evolution.
Modern science has hypothesized two possible pathways for the evolution of mutualistic endosymbiosis: one where an obligate antagonism is co-opted into an obligate mutualism (Co-Opted Antagonism Hypothesis), and one where a facultative mutualism evolves into an obligate mutualism (Black Queen Hypothesis). 
We investigated the viability of these pathways under different environmental conditions by expanding on the evolutionary agent-based system Symbulation. 
Specifically, we considered the impact of ectosymbiosis on de novo evolution of obligate mutualistic endosymbiosis. 
We found that introducing a facultative ectosymbiotic state allows endosymbiosis to evolve in a more diverse set of environmental conditions, while also decreasing the evolution of endosymbiosis in conditions where it can evolve independently.

## [dirty_transmission](/dirty_transmission)

[Dirty Transmission Hypothesis: Increased Mutations During Horizontal Transmission Can Select for Increased Levels of Mutualism in Endosymbionts](https://doi.org/10.32942/osf.io/7yskd):
A mutualistic symbiosis occurs when organisms of different species cooperate closely for a net benefit over time. 
Mutualistic relationships are important for human health, food production, and ecosystem maintenance.
However, they can evolve to parasitism or breakdown all together and the conditions that maintain and influence them are not completely understood. 
Vertical and horizontal transmission of mutualistic endosymbionts are two factors that can influence the evolution of mutualism. 
Using the artificial life system, Symbulation, we studied the effects of different rates of mutation during horizontal transmission on mutualistic symbiosis at different levels of vertical transmission.
We propose and provide evidence for the "Dirty Transmission Hypothesis", which states that higher rates of mutation during horizontal transmission can select for increased mutualism to avoid deleterious mutation accumulation.

## [mutualists_constrain](/mutualists_constrain)
Mutualists Constrain Evolution: In progress paper. Will update once published.