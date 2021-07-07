Symbulation -- Agent based modeling of symbiont ecology and evolution
===================================================================

:Authors: Anya Vostinar 

:GitHub: https://github.com/anyaevostinar/SymbulationEmp

This is a software model of symbiosis which allows for evolution of
parasitism and mutualism.

Go to
https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html to
try it out in your web browser!

To build, you must first download Empirical
(https://github.com/devosoft/Empirical) and place the two repositories
on the same level. Then within SymbulationEmp run: 
.. code-block:: 

make
./symbulation


The script 'stats\_scripts/simple\_repeat.py' can be
used to run many replicates and treatments.

The current settings can be configured in SymSettings.cfg.

To run end-to-end tests run 'make test'. This will overwrite the default
SymSettings.cfg!

Created part of my free logo at LogoMakr.com.

Contents:

\`\`\`{toctree} :caption: Using Empirical :maxdepth: 2

BuiltWithEmpiricalGallery/index QuickStartGuides/index library/index
api/library\_root a \`\`\`

\`\`\`{toctree} :caption: Contributing to Empirical :maxdepth: 0

dev/empirical-development-practices dev/getting-started
dev/contribution-guidelines-and-review dev/adding-documentation
dev/guide-to-testing \`\`\`

{ref}``genindex``
