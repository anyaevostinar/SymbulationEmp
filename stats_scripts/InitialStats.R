require(ggplot2)

fullcubeHelix <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
shorthelix <- c("#A7000F", "#E401E7","#5E8EFF","#86E9FE","#B2FCE3")
elevenhelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")

setwd("~/Desktop")

initial_data <- read.table("munged_basic.dat", h=T)

vert0 <- cbind(subset(initial_data, treatment== 0.0), Treatment= "0%")
vert01 <- cbind(subset(initial_data, treatment== 0.1), Treatment= "10%")
vert02 <- cbind(subset(initial_data, treatment== 0.2), Treatment= "20%")
vert03 <- cbind(subset(initial_data, treatment== 0.3), Treatment= "30%")
vert04 <- cbind(subset(initial_data, treatment== 0.4), Treatment= "40%")
vert05 <- cbind(subset(initial_data, treatment== 0.5), Treatment= "50%")
vert06 <- cbind(subset(initial_data, treatment== 0.6), Treatment= "60%")
vert07 <- cbind(subset(initial_data, treatment== 0.7), Treatment= "70%")
vert08 <- cbind(subset(initial_data, treatment== 0.8), Treatment= "80%")
vert09 <- cbind(subset(initial_data, treatment== 0.9), Treatment= "90%")
vert1 <- cbind(subset(initial_data, treatment== 1.0), Treatment= "100%")

vert_sweep <- rbind(vert0, vert01, vert02, vert03, vert04, vert05, vert06, vert07, vert08, vert09, vert1)

ggplot(data=initial_data, aes(x=as.factor(treatment), y=donate, colour=partner)) + ylab("Final Mean Interaction Value") + xlab("Vertical Transmission Rate") +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())+ geom_boxplot() + ylim(c(-1.0,1.0)) +scale_colour_manual(values=shorthelix)

vert_sweep <- subset(vert_sweep, update < 10000)

ggplot(data=vert_sweep, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Interaction Behavior") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~Treatment) + ylim(c(-0.5,0.5)) +scale_colour_manual(values=shorthelix) + scale_fill_manual(values=shorthelix) + theme(axis.text.x = element_text(angle=90, hjust=1))


ggplot(data=initial_data, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Donation Rate") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~treatment) + ylim(c(-1,1))

##one to twenty
initial_data <- read.table("munged_alltogether.dat", h=T)
final_data = subset(initial_data, update ==99900)


mut01_mult5_vert01_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.01_start0."), Treatment= "1%")
mut01_mult5_vert02_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.02_start0."), Treatment= "2%")
mut01_mult5_vert03_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.03_start0."), Treatment= "3%")
mut01_mult5_vert04_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.04_start0."), Treatment= "4%")
mut01_mult5_vert05_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.05_start0."), Treatment= "5%")
mut01_mult5_vert06_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.06_start0."), Treatment= "6%")
mut01_mult5_vert07_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.07_start0."), Treatment= "7%")
mut01_mult5_vert08_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.08_start0."), Treatment= "8%")
mut01_mult5_vert09_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.09_start0."), Treatment= "9%")
mut01_mult5_vert11_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.11_start0."), Treatment= "11%")
mut01_mult5_vert12_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.12_start0."), Treatment= "12%")
mut01_mult5_vert13_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.13_start0."), Treatment= "13%")
mut01_mult5_vert14_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.14_start0."), Treatment= "14%")
mut01_mult5_vert15_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.15_start0."), Treatment= "15%")
mut01_mult5_vert16_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.16_start0."), Treatment= "16%")
mut01_mult5_vert17_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.17_start0."), Treatment= "17%")
mut01_mult5_vert18_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.18_start0."), Treatment= "18%")
mut01_mult5_vert19_start0 <- cbind(subset(final_data, treatment== "mut0.001_mult5_vert0.19_start0."), Treatment= "19%")

final_vert <- rbind(mut01_mult5_vert0_start05, mut01_mult5_vert01_start0, mut01_mult5_vert02_start0, mut01_mult5_vert03_start0, mut01_mult5_vert04_start0, mut01_mult5_vert05_start0, mut01_mult5_vert06_start0, mut01_mult5_vert07_start0, mut01_mult5_vert08_start0, mut01_mult5_vert09_start0, mut01_mult5_vert10_start05, mut01_mult5_vert11_start0, mut01_mult5_vert12_start0, mut01_mult5_vert13_start0, mut01_mult5_vert14_start0, mut01_mult5_vert15_start0, mut01_mult5_vert16_start0, mut01_mult5_vert17_start0, mut01_mult5_vert18_start0, mut01_mult5_vert19_start0, mut01_mult5_vert20_start05)

ggplot(data=final_vert, aes(x=Treatment, y=donate, colour=Partner)) + ylab("Final Mean Interaction Value") + xlab("Vertical Transmission Rate") +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())+ theme(axis.text.x = element_text(angle=90, hjust=1)) +geom_boxplot() + ylim(c(-1.0,1.0)) +scale_colour_manual(values=shorthelix)

outlier.colour=NULL

ggplot(data=vert_sweep, aes(x=update, y=donate, group=Partner, colour=Partner)) + ylab("Mean Resource Behavior") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + facet_wrap(~Treatment) + ylim(c(-1,1))

ggplot(data=vert_sweep, aes(x=update, y=count, group=Partner, colour=Partner)) + ylab("Mean Count") + xlab("Evolutionary time (in updates)") + stat_summary(aes(color=Partner, fill=Partner),fun.data="mean_cl_boot", geom=c("smooth")) + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + ylim(c(0, 10000)) + facet_wrap(~Treatment)

ggplot(data=surviving, aes(x=host_donate, y=sym_donate, group=treatment, colour=treatment)) + ylab("Symbiont Donation") + xlab("Host Donation") + geom_point() + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + ylim(c(0, 1)) + xlim(c(0,1))
