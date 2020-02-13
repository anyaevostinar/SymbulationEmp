require(ggplot2)

args = commandArgs(trailingOnly=TRUE)
#args <- c("outlocal2.png", "lin.dat", "quad.dat")


#if (length(args) != 3) {
#  stop("Exactly three arguments must be supplied (host, symb, output).n", call.=FALSE)
#}

output_file <- args[1]
population <- as.numeric(args[2])
low_treatment <- as.numeric(args[3])
high_treatment <- as.numeric(args[4])

graph_data <- data.frame(MOI=numeric(), survival=numeric(), treatment=numeric(), label=character())
for(treatment in 1:((length(args)-3)/2)) {
	host_data <- read.table(args[treatment*2+3], sep=",", header=TRUE)
	sym_data <- read.table(args[treatment*2+4], sep=",", header=TRUE)
	
	host_count <- as.numeric(tail(host_data,n=1)["count"])
	sym_count <- as.numeric(tail(sym_data,n=1)["count"])
	progress = (treatment-1)/((length(args)-3)/2)
	
	print(host_count)
	print(sym_count)
	print(progress)
	
	survival <- host_count / population
	moi <- sym_count / host_count
	treatment <- high_treatment* progress +low_treatment*(1-progress) 
	
	graph_data <- rbind(graph_data,list(MOI=moi, survival=survival, treatment=treatment, label="Label"))
}

print(graph_data)

##Plots graph_data on axes MIO, survival, with color on axis treatment. All with a facet_wrap of label
ggplot(data=graph_data, aes(x=MOI, y=survival, color=treatment)) + scale_colour_gradient(low = "#0000cc", high = "#ff5555", space = "Lab", na.value = "grey50", guide = "colourbar", aesthetics = "colour") + ylab("% Host Survival") + xlab("Multiplicity of Infection") + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + geom_point(size=2) + facet_wrap(~label)

ggsave(output_file)