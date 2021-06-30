#Alison Cameron

library(ggplot2)
library(readr)
library(dplyr)
library(stringr)
library(Hmisc)

get_filenames <- function(metric, treatments, seeds){
  num_filenames <- length(treatments)*length(seeds)
  all_filenames <- rep(NA, num_filenames)
  k <- 1
  for (i in c(1:length(seeds))){
    for (j in c(1:length(treatments))){
      treatment_string <- ""
      if(treatments[j] == 0.0){
        treatment_string <- "0.0"
      } else {
        treatment_string = toString(treatments[j])
      }
      filename <- str_c(c(metric, 'PLR', treatment_string, 
                          "_Seed", toString(seeds[i]), ".data"), 
                        sep="", collapse="")
      all_filenames[k] <- filename
      k <- k + 1
    }
  }
  all_filenames
}

combine_data <- function(filenames){
  full_filename <- str_c(c("Lysogeny_Test_Data/", filenames[1]),
                         sep="", collapse="")
  plr_val <- str_extract(filenames[1], "[:digit:].[:digit:]")
  
  all_data <- read_csv(full_filename) %>% mutate(PLR = plr_val)
  
  for (i in c(2:length(filenames))){
    full_filename <- str_c(c("Lysogeny_Test_Data/", filenames[i]),
                           sep="", collapse="")
    plr_val <- str_extract(filenames[i], "[:digit:].[:digit:]")
    add_data <- read_csv(full_filename) %>% mutate(PLR = plr_val)
    all_data <- full_join(all_data, add_data)
  }
  
  all_data %>%
    mutate(Prophage_loss_rate = as.factor(PLR))
}

my_treatments = c(0.0, 0.5)
my_seeds = c(10:20)

hostval_filenames <- get_filenames("HostVals", my_treatments, my_seeds)
hostvals <- combine_data(hostval_filenames)

lysischance_filenames <- get_filenames("LysisChance_", my_treatments, my_seeds)
lysischances <- combine_data(lysischance_filenames)

colors <- c("#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", 
            "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", 
            "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")

hostvals_plot <- ggplot(data=hostvals, aes(x=update, y=mean_intval, 
                          group=Prophage_loss_rate, colour=Prophage_loss_rate)) + 
  ylab("Mean Host Interaction Value") + xlab("Updates") + 
  stat_summary(aes(color=Prophage_loss_rate, fill=Prophage_loss_rate),
               fun.data="mean_cl_boot", geom=c("smooth"), se=TRUE) + 
  theme(panel.background = element_rect(fill='white', colour='black')) +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  guides(fill=FALSE) + scale_colour_manual(values=colors) + 
  scale_fill_manual(values=colors)

lysischances_plot <- ggplot(data=lysischances, aes(x=update, y=mean_lysischance,
                                                   group=Prophage_loss_rate, color=Prophage_loss_rate)) +
  ylab("Mean chance of lysis") + xlab("Updates") + 
  stat_summary(aes(color=Prophage_loss_rate, fill=Prophage_loss_rate),
               fun.data="mean_cl_boot", geom=c("smooth"), se=TRUE) + 
  theme(panel.background = element_rect(fill='white', colour='black')) +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  guides(fill=FALSE) + scale_colour_manual(values=colors) + 
  scale_fill_manual(values=colors)

hostvals_plot
lysischances_plot

#ggsave(filename="LysisChance.pdf", plot = lysischances_plot)
  


