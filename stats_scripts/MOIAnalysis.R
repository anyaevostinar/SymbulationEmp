require(ggplot2)
fullcubeHelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#96FFF7", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")


temp <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
setwd("~/Desktop/SymbulationEmp/stats_scripts")
pop_cap <- 10000

initial_data <- read.table("../munged_moi_sweep.dat", h=T)

initial_data <-cbind(initial_data, MOI=initial_data$sym_count / initial_data$host_count, survival=initial_data$host_count/pop_cap, SMOI=factor(initial_data$treatment))

ggplot(data=initial_data, aes(x=update, y=survival, group=SMOI, colour=SMOI)) + ylab("Host Survival %") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=fullcubeHelix) + scale_fill_manual(values=fullcubeHelix)

ggplot(data=initial_data, aes(x=update, y=MOI, group=SMOI, colour=SMOI)) + ylab("Actual MOI") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=fullcubeHelix) + scale_fill_manual(values=fullcubeHelix)