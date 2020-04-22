#!/bin/sh
(cd ../../ && ./symbulation)
/Library/Frameworks/R.framework/Versions/3.6/Resources/Rscript --vanilla ./collate_data.R $1