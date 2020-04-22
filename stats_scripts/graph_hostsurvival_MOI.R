require(ggplot2)
require(gridExtra)
#install.packages('dplyr')
library(dplyr)
#require(viridis)


#Load data

data = read.table('data_management/collated_data.data',header=TRUE)

#suppressed = subset(data, select = c(BS, BT, SL, file_name))
#trimmed_data = subset(data, select = -c(BS, BT, SL, file_name))
#trimmed_data
#colnames(data)

#Filter chosen data
form1 = filter(data, VERSION == 'Standard-1.0' && BS >= 999999999 && BT == 1 && SL >= 999999999 && POP == 10000)

form2 = filter(form1, HRR==26)

variable_burst_time_all_slrs = filter(data, HRR==30, POP==10000, VERSION=="Standard-1.0", BS==1000800000)
slr_index = 3
slr_choice = unique(variable_burst_time_all_slrs $SLR)[slr_index]
variable_burst_time_chosen_slr  = filter(variable_burst_time_all_slrs, SLR==slr_choice)

#Plot

#Select data
plot = ggplot(data=form1, aes(x=moi, y=survival, color=SLR)) + facet_wrap(~HRR)

#Label
plot = plot + ylab("% Host Survival") + xlab("Multiplicity of Infection") + ggtitle("Host Survival vs. MOI at various SLR (color, log-scale) and HRR (facet).\nUnlimited burst size and symbionts per host, burst time is 1 timestep, 100 resources per timestep, 1000 timestep simulation.")

#Limit axes
plot = plot + ylim(0,1.25) + xlim(0,15)

#Color axis
plot = plot + scale_color_gradient(low = "#0000cc", high = "#ff4444", trans='log', breaks=unique(data$SLR))

#Apply theme
plot = plot + theme(panel.background = element_rect(fill='white', color='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + geom_point(size=2)

#Display
#plot



#Plot

#Select data
plot2 = ggplot(data=form2, aes(x=moi, y=survival, color=SLR))

#Label
plot2 = plot2 + ylab("% Host Survival") + xlab("Multiplicity of Infection") + ggtitle("HRR = 26")

#Limit axes
plot2 = plot2 + ylim(0,1.25) + xlim(0,100)

#Color axis
plot2 = plot2 + scale_color_gradient(low = "#0000cc", high = "#ff4444", trans='log', breaks=unique(data$SLR))

#Apply theme
plot2 = plot2 + theme(panel.background = element_rect(fill='white', color='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + geom_point(size=2)

#Display
#grid.arrange(plot, plot2)




#Plot variable burst time

#Select data
plot = ggplot(data= variable_burst_time_chosen_slr, aes(x=moi, y=survival)) + facet_wrap(~BT)

#Label
plot = plot + ylab("% Host Survival") + xlab("Multiplicity of Infection") + ggtitle(paste("Host Survival vs. MOI at various BT.\nUnlimited burst size and symbionts per host, 100 resources per timestep,\n1000 timestep simulation, 30 resources for host reproduction \n(fairly close to the 25 resource discrete threshold), SLR=", slr_choice, ".",sep=""))

#Limit axes
plot = plot + ylim(0,1.25) + xlim(0,400)

#Apply theme
plot = plot + theme(panel.background = element_rect(fill='white', color='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + geom_point(size=2)

plot