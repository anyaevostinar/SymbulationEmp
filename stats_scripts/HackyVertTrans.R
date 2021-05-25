require(ggplot2)

fullcubeHelix <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
shorthelix <- c("#A7000F", "#E401E7","#5E8EFF","#86E9FE","#B2FCE3")
elevenhelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")

setwd("~/Desktop/SymbulationEmp/VertTransSpa")

initial_data <- read.table("munged_data.dat", h=T)

vert0 <- cbind(subset(initial_data, smoi== 0.0), Treatment= "0%")
vert01 <- cbind(subset(initial_data, smoi== 0.1), Treatment= "10%")
vert02 <- cbind(subset(initial_data, smoi== 0.2), Treatment= "20%")
vert03 <- cbind(subset(initial_data, smoi== 0.3), Treatment= "30%")
vert04 <- cbind(subset(initial_data, smoi== 0.4), Treatment= "40%")
vert05 <- cbind(subset(initial_data, smoi== 0.5), Treatment= "50%")
vert06 <- cbind(subset(initial_data, smoi== 0.6), Treatment= "60%")
vert07 <- cbind(subset(initial_data, smoi== 0.7), Treatment= "70%")
vert08 <- cbind(subset(initial_data, smoi== 0.8), Treatment= "80%")
vert09 <- cbind(subset(initial_data, smoi== 0.9), Treatment= "90%")
vert1 <- cbind(subset(initial_data, smoi== 1.0), Treatment= "100%")

vert_sweep <- rbind(vert0, vert01, vert02, vert03, vert04, vert05, vert06, vert07, vert08, vert09, vert1)
vert_sweep <- subset(vert_sweep, update==1000)

ggplot(data=vert_sweep, aes(x=Treatment, y=host_val)) + ylab("Final Mean Interaction Value of Host") + xlab("Vertical Transmission Rate") +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())+ geom_boxplot() + ylim(c(-1.0,1.0)) +scale_colour_manual(values=shorthelix)

ggplot(data=vert_sweep, aes(x=Treatment, y=sym_val)) + ylab("Final Mean Interaction Value of Symbiont") + xlab("Vertical Transmission Rate") +theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())+ geom_boxplot() + ylim(c(-1.0,1.0)) +scale_colour_manual(values=shorthelix)