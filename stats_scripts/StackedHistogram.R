require(ggplot2)
tenhelix <- c("#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")

setwd("~/Research/SymbulationEmp/")

first_try <- read.table("munged_histogram.dat", h=T)

neg1_9 <- cbind(subset(first_try, interval=="-1_-.9"), Interaction_Rate="-1 to -0.8 (Parasitic)")
neg9_8 <- cbind(subset(first_try, interval=="-.9_-.8"), Interaction_Rate="-1 to -0.8 (Parasitic)")
neg8_7 <- cbind(subset(first_try, interval=="-.8_-.7"), Interaction_Rate="-0.8 to -0.6 (Parasitic)")
neg7_6 <- cbind(subset(first_try, interval=="-.7_-.6"), Interaction_Rate="-0.8 to -0.6 (Parasitic)")
neg6_5 <- cbind(subset(first_try, interval=="-.6_-.5"), Interaction_Rate="-0.6 to -0.4 (Detrimental)")
neg5_4 <- cbind(subset(first_try, interval=="-.5_-.4"), Interaction_Rate="-0.6 to -0.4 (Detrimental)")
neg4_3 <- cbind(subset(first_try, interval=="-.4_-.3"), Interaction_Rate="-0.4 to -0.2 (Detrimental)")
neg3_2 <- cbind(subset(first_try, interval=="-.3_-.2"), Interaction_Rate="-0.4 to -0.2 (Detrimental)")
neg2_1 <- cbind(subset(first_try, interval=="-.2_-.1"), Interaction_Rate="-0.2 to 0 (Nearly Neutral)")
neg1_0 <- cbind(subset(first_try, interval=="-.1_0"), Interaction_Rate="-0.2 to 0 (Nearly Neutral)")
pos0_1 <- cbind(subset(first_try, interval=="0_.1"), Interaction_Rate="0 to 0.2 (Nearly Neutral)")
pos1_2 <- cbind(subset(first_try, interval==".1_.2"), Interaction_Rate="0 to 0.2 (Nearly Neutral)")
pos2_3 <- cbind(subset(first_try, interval==".2_.3"), Interaction_Rate="0.2 to 0.4 (Positive)")
pos3_4 <- cbind(subset(first_try, interval==".3_.4"), Interaction_Rate="0.2 to 0.4 (Positive)")
pos4_5 <- cbind(subset(first_try, interval==".4_.5"), Interaction_Rate="0.4 to 0.6 (Positive)")
pos5_6 <- cbind(subset(first_try, interval==".5_.6"), Interaction_Rate="0.4 to 0.6 (Positive)")
pos6_7 <- cbind(subset(first_try, interval==".6_.7"), Interaction_Rate="0.6 to 0.8 (Mutualistic)")
pos7_8 <- cbind(subset(first_try, interval==".7_.8"), Interaction_Rate="0.6 to 0.8 (Mutualistic)")
pos8_9 <- cbind(subset(first_try, interval==".8_.9"), Interaction_Rate="0.8 to 1.0 (Mutualistic)")
pos9_1 <- cbind(subset(first_try, interval==".9_1"), Interaction_Rate="0.8 to 1.0 (Mutualistic)")

combined <- rbind(neg1_9, neg9_8, neg8_7, neg7_6, neg6_5, neg5_4, neg4_3, neg3_2, neg2_1, neg1_0, pos0_1, pos1_2, pos2_3, pos3_4, pos4_5, pos5_6, pos6_7, pos7_8, pos8_9, pos9_1)

temp <- aggregate(list(count = combined$count), list(update=combined$update, rep=combined$rep, Interaction_Rate=combined$Interaction_Rate), sum)


ggplot(temp, aes(update, count)) + geom_area(aes(fill=Interaction_Rate), position='stack') +ylab("Count of Hosts with Phenotype") + xlab("Evolutionary time (in updates)") +scale_fill_manual("Interaction Rate\n Phenotypes",values=tenhelix) +facet_wrap(~rep) + theme(panel.background = element_rect(fill='light grey', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + guides(fill = guide_legend())
