require(ggplot2)

fullcubeHelix <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
shorthelix <- c("#A7000F", "#E401E7","#5E8EFF","#86E9FE","#B2FCE3")
elevenhelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")
tenhelix <- c("#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")

setwd("~/Desktop")

initial_data <- read.table("munged_alltogether.dat", h=T)
initial_data <- read.table("munged_alltogether_hostspatial.dat", h=T)

initial_data <- subset(initial_data, update < 100000)


final_data <- subset(initial_data, update=="99900")
initial_data <- final_data


mut01_mult10_vert0_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0._start0."), Treatment= "0%")
mut01_mult10_vert01_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.1_start0."), Treatment= "10%")
mut01_mult10_vert02_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.2_start0."), Treatment= "20%")
mut01_mult10_vert03_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.3_start0."), Treatment= "30%")
mut01_mult10_vert04_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.4_start0."), Treatment= "40%")
mut01_mult10_vert05_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.5_start0."), Treatment= "50%")
mut01_mult10_vert06_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.6_start0."), Treatment= "60%")
mut01_mult10_vert07_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.7_start0."), Treatment= "70%")
mut01_mult10_vert08_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.8_start0."), Treatment= "80%")
mut01_mult10_vert09_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.9_start0."), Treatment= "90%")
mut01_mult10_vert1_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert1._start0."), Treatment= "100%")



vert_sweep <- rbind(mut01_mult10_vert0_start05, mut01_mult10_vert01_start05, mut01_mult10_vert02_start05, mut01_mult10_vert03_start05, mut01_mult10_vert04_start05, mut01_mult10_vert05_start05, mut01_mult10_vert06_start05, mut01_mult10_vert07_start05, mut01_mult10_vert08_start05, mut01_mult10_vert09_start05, mut01_mult10_vert1_start05)
vert_sweep <- subset(vert_sweep, update=="500000")

ggplot(data=vert_sweep, aes(x=Treatment, y=donate, colour=Partner)) + ylab("Final Mean Interaction Value") + xlab("Vertical Transmission Rate") +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())+ geom_boxplot() + ylim(c(-1.0,1.0)) +scale_colour_manual(values=shorthelix)

initial_data[is.na(initial_data)] <-0
ggplot(data=vert_sweep, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Interaction Value") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~Treatment) + scale_colour_manual(values=shorthelix) + scale_fill_manual(values=shorthelix) + theme(axis.text.x = element_text(angle=90, hjust=1))

##Tasks
tasks_data <- read.table("munged_tasks.dat", h=T)
tasks_data <- subset(tasks_data, treatment=="mut0.001_mult1_vert0.5_start0.")
tasks_data <- subset(tasks_data, rep=="1003")
tasks_data <- subset(tasks_data, update < 50000)

vert0 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0._start0."), Treatment= "0%")
vert01 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.1_start0."), Treatment= "10%")
vert02 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.2_start0."), Treatment= "20%")
vert03 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.3_start0."), Treatment= "30%")
vert04 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.4_start0."), Treatment= "40%")
vert05 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.5_start0."), Treatment= "50%")
vert06 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.6_start0."), Treatment= "60%")
vert07<- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.7_start0."), Treatment= "70%")
vert08 <- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.8_start0."), Treatment= "80%")
vert09<- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert0.9_start0."), Treatment= "90%")
vert1<- cbind(subset(tasks_data, treatment== "mut0.001_mult1_vert1._start0."), Treatment= "100%")

tasks <- rbind(vert0, vert01, vert02, vert03, vert04, vert05, vert06, vert07, vert08, vert09, vert1)

ggplot(data=tasks, aes(x=update, y=(task_0), group=partner, colour=partner)) + ylab("Mean Amount Of Task A") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=partner, fill=partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~Treatment) + scale_fill_manual("Partner",values=shorthelix) + scale_colour_manual(values=shorthelix)


##No division of labor

hosts_general <- read.table("munged_alltogether.dat", h=T)
initial_data <- subset(hosts_general, update<5000)

mut01_mult10_vert0_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0._start0."), Treatment= "0%")
mut01_mult10_vert01_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.1_start0."), Treatment= "10%")
mut01_mult10_vert02_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.2_start0."), Treatment= "20%")
mut01_mult10_vert03_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.3_start0."), Treatment= "30%")
mut01_mult10_vert04_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.4_start0."), Treatment= "40%")
mut01_mult10_vert05_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.5_start0."), Treatment= "50%")
mut01_mult10_vert06_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.6_start0."), Treatment= "60%")
mut01_mult10_vert07_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.7_start0."), Treatment= "70%")
mut01_mult10_vert08_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.8_start0."), Treatment= "80%")
mut01_mult10_vert09_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert0.9_start0."), Treatment= "90%")
mut01_mult10_vert1_start05 <- cbind(subset(initial_data, treatment== "mut0.001_mult1_vert1._start0."), Treatment= "100%")

vert_sweep <- rbind(mut01_mult10_vert0_start05, mut01_mult10_vert01_start05, mut01_mult10_vert02_start05, mut01_mult10_vert03_start05, mut01_mult10_vert04_start05, mut01_mult10_vert05_start05, mut01_mult10_vert06_start05, mut01_mult10_vert07_start05, mut01_mult10_vert08_start05, mut01_mult10_vert09_start05)

ggplot(data=vert_sweep, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Interaction Value") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~Treatment) + scale_colour_manual(values=shorthelix) + scale_fill_manual(values=shorthelix) + ylim(c(-1, 1))

+ theme(axis.text.x = element_text(angle=90, hjust=1))


ggplot(data=vert_sweep, aes(x=Treatment, y=donate, colour=Partner)) + ylab("Final Mean Interaction Value") + xlab("Vertical Transmission Rate") +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())+ geom_boxplot() + ylim(c(-1.0,1.0)) +scale_colour_manual(values=shorthelix)

##old stuff
host_tasks_data <- cbind(tasks_data, diff = abs(tasks_data$host_0 - tasks_data$host_1), task_0=tasks_data$host_0, task_1 = tasks_data$host_1, partner="Host")
sym_tasks_data <- cbind(tasks_data, diff = abs(tasks_data$sym_0 - tasks_data$sym_1), task_0=tasks_data$sym_0, task_1 = tasks_data$sym_1, partner="Symbiont")

ggplot(data=host_tasks_data, aes(x=update, y=hosts_diff, group=treatment, colour=treatment)) + ylab("Mean Difference between task 0 and task 1 in a replicate") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=treatment, fill=treatment),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) 

ggplot(data=sym_tasks_data, aes(x=update, y=syms_diff, group=treatment, colour=treatment)) + ylab("Mean Difference between task 0 and task 1 in a replicate") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=treatment, fill=treatment),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) 

tasks_comb <- rbind(host_tasks_data, sym_tasks_data)

vert0 <- subset(tasks_comb, treatment=="mut0.001_mult1_vert0._start0.")




