require(ggplot2)

setwd("~/Desktop/SymbulationEmp/stats_scripts")
pop_cap <- 10000

initial_data <- read.table("munged_moi.dat", h=T)

initial_data <-cbind(initial_data, MOI=initial_data$sym_count / initial_data$host_count, survival=initial_data$host_count/pop_cap)

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

ggplot(data=initial_data, aes(x=MOI, y=survival)) + ylab("% Host Survival") + xlab("Multiplicity of Infection") + theme(panel.background = element_rect(fill='white', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + geom_point(size=2) + facet_wrap(~treatment)