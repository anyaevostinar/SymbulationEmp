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
:: 
  make
  ./symbulation


The script 'stats\_scripts/simple\_repeat.py' can be
used to run many replicates and treatments.

The current settings can be configured in SymSettings.cfg.

To run end-to-end tests run 'make test'. This will overwrite the default
SymSettings.cfg!

Created part of my free logo at LogoMakr.com.

Contents:

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Using Symbulation
==================

* :ref:`QuickStartGuide/index.rst<Quick start guide>`
* :ref:`Using Symbulation <library/index.rst>`_
* :ref:`Library <api/library_root.rst>`_


.. toctree::
  :caption: Contributing to Symbulation
  :maxdepth: 0

Contributing to Symbulation
==================
* :ref:`dev/empirical-development-practices`
* :ref:`dev/getting-started`
* :ref:`dev/contribution-guidelines-and-review`
* :ref:`dev/adding-documentation`
* :ref:`dev/guide-to-testing`

{ref}`genindex`

