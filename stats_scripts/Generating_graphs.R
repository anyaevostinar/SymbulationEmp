require(ggplot2)
require(RColorBrewer)
fullcubeHelix <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
shorthelix <- c("#A7000F", "#E401E7","#5E8EFF","#86E9FE","#B2FCE3")
elevenhelix <- c("#673F03", "#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")
tenhelix <- c("#891901", "#B50142", "#D506AD", "#AB08FF", "#5731FD", "#4755FF", "#86E9FE", "#B2FCE3", "#D4FFDD", "#EFFDF0")

setwd("~/Desktop")

all_data <- read.table("munged_buckets_sym_around10.dat", h=T)
all_data <- read.table("munged_buckets_sym.dat", h=T)
under10k <- subset(all_data, update <=50000)
first_try <- subset(all_data, treatment=="mut0.001_mult5_vert0.1_start0.")
first_try <- all_data
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
pos1 <- cbind(subset(first_try, interval=="1"), Interaction_Rate="0.8 to 1.0 (Mutualistic)")

combined <- rbind(neg1_9, neg9_8, neg8_7, neg7_6, neg6_5, neg5_4, neg4_3, neg3_2, neg2_1, neg1_0, pos0_1, pos1_2, pos2_3, pos3_4, pos4_5, pos5_6, pos6_7, pos7_8, pos8_9, pos9_1, pos1)

vert0 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0._start0."), Rate = "0%")
vert10 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.1_start0."), Rate = "10%")
vert20 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.2_start0."), Rate = "20%")
vert30 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.3_start0."), Rate = "30%")
vert40 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.4_start0."), Rate = "40%")
vert50 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.5_start0."), Rate = "50%")
vert60 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.6_start0."), Rate = "60%")
vert70 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.7_start0."), Rate = "70%")
vert80 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.8_start0."), Rate = "80%")
vert90 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.9_start0."), Rate = "90%")
vert100 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert1._start0."), Rate = "100%")

combined <- rbind(vert0, vert10, vert20, vert30, vert40, vert50, vert60, vert70, vert80, vert90, vert100)

vert08 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.08_start0."), Rate = "8%")
vert09 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.09_start0."), Rate = "9%")
vert10 <- cbind(subset(combined, treatment=="mut0.001_mult5_vert0.1_start0."), Rate="10%")

combined <- rbind(vert08, vert09, vert10)

combined <- vert09

##Reps
temp <- aggregate(list(count = combined$count), list(update=combined$update, rep=combined$rep, Interaction_Rate=combined$Interaction_Rate, Rate=combined$Rate), sum)

ggplot(temp, aes(update, count)) + geom_area(aes(fill=Interaction_Rate), position='stack') +ylab("Count of Symbionts with Phenotype") + xlab("Evolutionary time (in updates)") +scale_fill_manual("Interaction Rate\n Phenotypes",values=tenhelix) + theme(panel.background = element_rect(fill='light grey', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + guides(fill = guide_legend())+ facet_wrap(~rep)

##Averaged
temp <- aggregate(list(count = combined$count), list(update=combined$update, Interaction_Rate=combined$Interaction_Rate, treatment=combined$treatment, Rate=combined$Rate), mean)

ggplot(temp, aes(update, count)) + geom_area(aes(fill=Interaction_Rate), position='stack') +ylab("Count of Symbionts with Phenotype") + xlab("Evolutionary time (in updates)") +scale_fill_manual("Interaction Rate\n Phenotypes",values=tenhelix) + theme(panel.background = element_rect(fill='light grey', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + guides(fill = guide_legend()) + facet_wrap(~Rate)

ggplot(temp, aes(update, count)) + geom_area(aes(fill=Interaction_Rate), position='stack') +ylab("Count of Hosts with Phenotype") + xlab("Evolutionary time (in updates)") +scale_fill_manual("Interaction Rate\n Phenotypes",values=tenhelix) + theme(panel.background = element_rect(fill='light grey', colour='black')) + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + guides(fill=FALSE) + guides(fill = guide_legend()) + facet_wrap(~Rate)

##Hosts
host_data <- read.table("munged_buckets_host.dat", h=T)
first_try <- subset(host_data, update <=10000)
first_try <- subset(first_try, treatment=="mut0.001_mult5_vert0.1_start0.")
first_try <- host_data
all_data <- read.table("munged_buckets_host_around10.dat", h=T)
first_try <- subset(all_data, treatment=="mut0.001_mult5_vert0.09_start0.")
first_try <- subset(all_data, update <= 50000)
neg1_9 <- cbind(subset(first_try, interval=="-1_-.9"), Interaction_Rate="-1 to -0.8 (Defensive)")
neg9_8 <- cbind(subset(first_try, interval=="-.9_-.8"), Interaction_Rate="-1 to -0.8 (Defensive)")
neg8_7 <- cbind(subset(first_try, interval=="-.8_-.7"), Interaction_Rate="-0.8 to -0.6 (Defensive)")
neg7_6 <- cbind(subset(first_try, interval=="-.7_-.6"), Interaction_Rate="-0.8 to -0.6 (Defensive)")
neg6_5 <- cbind(subset(first_try, interval=="-.6_-.5"), Interaction_Rate="-0.6 to -0.4 (Mildly Defensive)")
neg5_4 <- cbind(subset(first_try, interval=="-.5_-.4"), Interaction_Rate="-0.6 to -0.4 (Mildly Defensive)")
neg4_3 <- cbind(subset(first_try, interval=="-.4_-.3"), Interaction_Rate="-0.4 to -0.2 (Mildly Defensive)")
neg3_2 <- cbind(subset(first_try, interval=="-.3_-.2"), Interaction_Rate="-0.4 to -0.2 (Mildly Defensive)")
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
pos8_9 <- cbind(subset(first_try, interval==".8_.9"), Interaction_Rate="0.6 to 1.0 (Mutualistic)")
pos9_1 <- cbind(subset(first_try, interval==".9_1"), Interaction_Rate="0.6 to 1.0 (Mutualistic)")
pos1 <- cbind(subset(first_try, interval=="1"), Interaction_Rate="0.6 to 1.0 (Mutualistic)")


combined <- rbind(neg1_9, neg9_8, neg8_7, neg7_6, neg6_5, neg5_4, neg4_3, neg3_2, neg2_1, neg1_0, pos0_1, pos1_2, pos2_3, pos3_4, pos4_5, pos5_6, pos6_7, pos7_8, pos8_9, pos9_1, pos1)

temp <- aggregate(list(count = combined$count), list(update=combined$update, rep=combined$rep, Interaction_Rate=combined$Interaction_Rate, treatment=combined$treatment), sum)

ggplot(temp, aes(update, count)) + geom_area(aes(fill=Interaction_Rate), position='stack') +ylab("Count of Hosts with Phenotype") + xlab("Evolutionary time (in updates)") +scale_fill_brewer("Interaction Rate\n Phenotypes", palette="Paired") + guides(fill = guide_legend(reverse=TRUE)) + facet_wrap(~treatment) +facet_wrap(~rep)

