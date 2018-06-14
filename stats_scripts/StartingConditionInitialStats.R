require(ggplot2)

setwd("~/Desktop")

mut_data <- read.table("munged_start_mut.dat", h=T)
para_data <- read.table("munged_start_para.dat", h=T)
initial_data <- subset(initial_data, update < 50000)
tasks_data <- read.table("munged_tasks.dat", h=T)
tasks_data <- subset(tasks_data, update < 10000)

final_data <- subset(initial_data, update=="99900")




initial_data[is.na(initial_data)] <-0
ggplot(data=mut_data, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Donation Rate Starting Mutualistic") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~treatment) 

ggplot(data=para_data, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Donation Rate Starting Parasitic") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~treatment) 