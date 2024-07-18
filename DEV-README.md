This is a document for development notes, i.e. a readme but for the development process, which should be removed once this is pulled into main (fingers crossed someday!).


## Current issues

### Main compatibility (and updating Empirical version)
On 7/18/24, Anya tried to merge in changes from main, but everything broke horribly because signalgp-lite is using an old version of Empirical (and conduit, a submodule of SGP-Lite as well) and the vendorizing within sgp-lite is apparently not enough to allow for me to easily update the version that Symbulation is using. 
* Things to still try: update the makefile borrowing from the Digital Evolution project to specify the version of Empirical, since that did work before, I just forgot when I was trying to do the quick update

### Hacky-ness
Much of the disease parasite implementation was hacked in and so needs to be written more modularly to allow for the different kinds of parasitism and mutualism, how to best go about doing this both efficiently and modularly?
* First step: make a list of the hacky functionality here

### Don't lose the ecto code
Kai Johnson implemented support for ectosymbionts with complex genomes in the branch `complex-genomes`, which isn't currently compatible with this branch, but shouldn't be forgotten since it's good stuff.