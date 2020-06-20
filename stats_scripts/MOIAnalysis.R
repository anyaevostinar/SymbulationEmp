require(ggplot2)
fullcubeHelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#96FFF7", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")


temp <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
setwd("~/Desktop/SymbulationEmp/VTMOI")
pop_cap <- 10000

initial_data <- read.table("munged_data.dat", h=T)
initial_data <-cbind(initial_data, MOI=initial_data$sym_count / initial_data$host_count, survival=((initial_data$host_count/pop_cap)*100), SMOI=factor(initial_data$smoi), uninfected_perc=((initial_data$uninfected/pop_cap)*100))

zeroed <- initial_data
zeroed[is.na(zeroed)] <- 0

early <- subset(initial_data, update<=40)



ggplot(data=initial_data, aes(x=update, y=survival, group=SMOI, colour=SMOI)) + ylab("Host Survival (infected and non-infected) %") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + facet_wrap(~rep)

ggplot(data=initial_data, aes(x=update, y=uninfected_perc, group=SMOI, colour=SMOI)) + ylab("Host Uninfected %") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

ggplot(data=initial_data, aes(x=update, y=sym_count, group=SMOI, colour=SMOI)) + ylab("Symbiont Count") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + facet_wrap(~rep)

ggplot(data=initial_data, aes(x=update, y=MOI, group=SMOI, colour=SMOI)) + ylab("Actual MOI") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

ggplot(data=initial_data, aes(x=update, y=burst_size, group=SMOI, colour=SMOI)) + ylab("Mean Burst Size") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

ggplot(data=initial_data, aes(x=update, y=sym_val, group=SMOI, colour=SMOI)) + ylab("Mean Symbiont Interaction Value") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

ggplot(data=initial_data, aes(x=update, y=host_val, group=SMOI, colour=SMOI)) + ylab("Mean Host Interaction Value") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

update_5 <- subset(initial_data, update==5)
update_10 <- subset(initial_data, update==10)
update_25 <- subset(initial_data, update==25)
update_30 <- subset(initial_data, update==30)
update_35 <- subset(initial_data, update==35)
update_40 <- subset(initial_data, update==40)
update_60 <- subset(initial_data, update==60)
update_50 <- subset(initial_data, update==50)
update_55 <- subset(initial_data, update==55)

ggplot(data=update_25, aes(x=MOI, y=burst_size, group=SMOI, colour=SMOI)) +ylab("Average Burst Size") + xlab("Actual MOI") + geom_point()

ggplot(data=update_5, aes(x=as.numeric(as.character(SMOI)), y=uninfected_perc)) +ylab("Uninfected Host %") + xlab("Starting MOI") + geom_point() +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~vt)

# Peak Actual MOI

peak_data <- read.table("munged_peak_moi.dat", h=T)
peak_data <-cbind(peak_data, MOI=peak_data$sym_count / peak_data$host_count, survival=((peak_data$host_count/pop_cap)*100), SMOI=factor(peak_data$smoi), uninfected_perc=((peak_data$uninfected/pop_cap)*100))
ggplot(data=peak_data, aes(x=as.numeric(as.character(SMOI)), y=MOI)) +ylab("Peak Actual MOI") + xlab("Starting MOI") + geom_point() +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +xlim(0, 20)