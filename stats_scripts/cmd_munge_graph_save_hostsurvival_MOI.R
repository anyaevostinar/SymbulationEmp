require(ggplot2)

args = commandArgs(trailingOnly=TRUE)

##Input takest the form
# Output_file population number_of_trials (treatment_1 host_data_file_name_1 sym_data_file_name_1)*
# Where * means the sequence of arguments in parentheses can be repeated any number of times (one for each treatment).


output_file <- args[1]
population <- as.numeric(args[2])
trials <- as.numeric(args[3])

graph_data <- data.frame(MOI=numeric(), survival=numeric(), treatment=numeric(), label=character())
for(treatment in 1:((length(args)-2)/3)) {
	treatment_name <- as.numeric(args[treatment*3+1])#A numeric name
	
	for(trial in 0:(trials-1)) {
		host_data <- read.table(paste(args[treatment*3+2], trial, '.data', sep=""), sep=",", header=TRUE)
		sym_data <- read.table(paste(args[treatment*3+3], trial, '.data', sep=""), sep=",", header=TRUE)
		
		host_count <- as.numeric(tail(host_data,n=1)["count"])
		sym_count <- as.numeric(tail(sym_data,n=1)["count"])
				
		survival <- host_count / population
		if(host_count != 0)
			moi <- sym_count / host_count
		else
			moi <- 0
		
		graph_data <- rbind(graph_data,list(MOI=moi, survival=survival, treatment=treatment_name, label="Label"))
	}
}

##Used to plot graph_data on axes MIO, survival, with color on axis treatment. All with a facet_wrap of label
##Now plots graph_data on axes MIO, survival, with facet_wrap on axis treatment.
ggplot(data=graph_data, aes(x=MOI, y=survival)) + scale_colour_gradient(low = "#0000cc", high = "#ff5555", space = "Lab", na.value = "grey50", guide = "colourbar", aesthetics = "colour") + ylab("% Host Survival") + xlab("Multiplicity of Infection") + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + geom_point(size=2) + facet_wrap(~treatment) + ylim(0,1.25) + xlim(0,15)

ggsave(output_file)