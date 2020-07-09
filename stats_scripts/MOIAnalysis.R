require(ggplot2)
require(scales)
fullcubeHelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#96FFF7", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")


temp <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
setwd("~/Desktop/SymbulationEmp/PartPop")
pop_cap <- 1000

initial_data <- read.table("munged_data.dat", h=T)
initial_data <-cbind(initial_data, MOI=initial_data$sym_count / initial_data$host_count, survival=((initial_data$host_count/pop_cap)*100), SMOI=factor(initial_data$smoi), cfu_perc=((initial_data$cfu/pop_cap)*100))

low_moi <- subset(initial_data, smoi<20)

sym_int3 <- subset(low_moi, sym_int==-0.3)
sym_int1 <- subset(low_moi, sym_int==-1)

zeroed <- initial_data
zeroed[is.na(zeroed)] <- 0

early <- subset(initial_data, update<=40)



ggplot(data=low_moi, aes(x=update, y=survival, group=SMOI, colour=SMOI)) + ylab("Host Survival (infected and non-infected) %") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) +xlim(0,100)

ggplot(data=initial_data, aes(x=update, y=cfu_perc, group=SMOI, colour=SMOI)) + ylab("CFU %") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + xlim(0, 100)

ggplot(data=low_moi, aes(x=update, y=sym_count, group=SMOI, colour=SMOI)) + ylab("Bacteriophage Count") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + scale_y_continuous(labels=comma) + xlim(0,100)

ggplot(data=initial_data, aes(x=update, y=host_count, group=SMOI, colour=SMOI)) + ylab("Host Count") + xlab("Updates") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + xlim(0, 100) + ylim(0, 2500)

ggplot(data=low_moi, aes(x=update, y=MOI, group=SMOI, colour=SMOI)) + ylab("Actual MOI") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) +xlim(0,100)

ggplot(data=low_moi, aes(x=update, y=burst_size, group=SMOI, colour=SMOI)) + ylab("Mean Burst Size") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + xlim(0, 100)

ggplot(data=initial_data, aes(x=update, y=burst_size, group=SMOI, colour=SMOI)) + ylab("Mean Burst Size") + xlab("Time") + geom_point() + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp) + xlim(0, 100)

ggplot(data=initial_data, aes(x=update, y=sym_val, group=SMOI, colour=SMOI)) + ylab("Mean Symbiont Interaction Value") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

ggplot(data=initial_data, aes(x=update, y=host_val, group=SMOI, colour=SMOI)) + ylab("Mean Host Interaction Value") + xlab("Time") + stat_summary(aes(color=SMOI, fill=SMOI),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +scale_colour_manual(values=temp) + scale_fill_manual(values=temp)

update_1 <- subset(initial_data, update==1)
update_10 <- subset(initial_data, update==10)
update_25 <- subset(initial_data, update==25)
update_30 <- subset(initial_data, update==30)
update_35 <- subset(initial_data, update==35)
update_40 <- subset(initial_data, update==40)
update_60 <- subset(initial_data, update==60)
update_50 <- subset(initial_data, update==50)
update_55 <- subset(initial_data, update==55)

ggplot(data=update_25, aes(x=MOI, y=burst_size, group=SMOI, colour=SMOI)) +ylab("Average Burst Size") + xlab("Actual MOI") + geom_point()

ggplot(data=update_10, aes(x=as.numeric(as.character(SMOI)), y=cfu_perc, colour=as.factor(sym_int))) +ylab("CFU %") + xlab("Starting MOI") + geom_point() +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + xlim(0, 15) 

ggplot(data=update_1, aes(x=as.numeric(as.character(SMOI)), y=cfu_perc)) +ylab("CFU %") + xlab("Starting MOI") + geom_point() +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + xlim(0, 15) 

# Peak Actual MOI

peak_data <- read.table("munged_peak_moi.dat", h=T)
peak_data <-cbind(peak_data, MOI=peak_data$sym_count / peak_data$host_count, survival=((peak_data$host_count/pop_cap)*100), SMOI=factor(peak_data$smoi), uninfected_perc=((peak_data$uninfected/pop_cap)*100))
ggplot(data=peak_data, aes(x=as.numeric(as.character(SMOI)), y=MOI)) +ylab("Peak Actual MOI") + xlab("Starting MOI") + geom_point() +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) +xlim(0, 20)