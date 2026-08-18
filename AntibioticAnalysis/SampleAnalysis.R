require(ggplot2)
install.packages("viridis")
library(viridis)

#Set your working directory to the Analysis folder for your project

#Read in the data
initial_data <- read.table("munged_nand_count.dat", h=T)
lookup <- c(
  "NoAnti_Seed" = "No Antibiotics",
  "Anti_Seed"   = "Antibiotics"
)

initial_data$Treatment <- lookup[initial_data$treatment]

final_update <- subset(initial_data, update == "2000")

#Plot the symbiont nand counts
ggplot(data=final_update, aes(x=treatment, y=sym_nand_count, color=treatment)) + geom_boxplot(alpha=0.5, outlier.size=0) + ylab("Final Nand Count") + xlab("Treatment") + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + scale_color_manual(name="Treatment", values=viridis(2))

# Over time

ggplot(data = initial_data, aes(x = update, y = sym_nand_count, group = Treatment, color = Treatment, fill = Treatment)) + stat_summary(fun.data = "mean_cl_boot", geom = "ribbon", alpha = 0.2, color = NA) + stat_summary(fun.data = "mean_cl_boot", geom = "line") + scale_color_manual(values = viridis(2)) + scale_fill_manual(values = viridis(2)) + labs(x = "Evolutionary time (in updates)", y = "Mean Antiobitic Resistance in Plasmids") + theme_bw() + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill = "none")

# By rep
ggplot(data = initial_data, aes(x = update, y = sym_nand_count, group = Treatment, color = Treatment, fill = Treatment)) + stat_summary(fun.data = "mean_cl_boot", geom = "ribbon", alpha = 0.2, color = NA) + stat_summary(fun.data = "mean_cl_boot", geom = "line") + scale_color_manual(values = viridis(2)) + scale_fill_manual(values = viridis(2)) + labs(x = "Evolutionary time (in updates)", y = "Mean Antiobitic Resistance in Plasmids") + theme_bw() + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill = "none") + facet_wrap(~rep)

