#Alison Cameron


#---------Functions to read and munge data------------
get_treatment_val <- function(filename, treatment_name){
  #Input: a filename string and a string representing which treatment you are looking for
  #       Ex: "HostVals_VT0.6_SEED10.data" and "VT"
  #Output: A string representing the value for the given treatment
  #       Ex: "0.6"
  number_pattern <- "[\\d][._][\\d]*"
  treatment_pattern <- str_c(c(treatment_name, number_pattern), sep="", collapse="")
  treatment <- str_extract(filename, treatment_pattern)
  treatment_val <- NA
  
  decimal_pattern <- "[\\d].[\\d]+"
  integer_pattern <- "[\\d]+"
  if(str_detect(treatment, "\\.")){
    treatment_val <- str_extract(treatment, decimal_pattern)
  } else {
    treatment_val <- str_extract(treatment, integer_pattern)
  }
  
  full_treatment <- str_c(c(treatment_name, " ", treatment_val), sep="", collapse="")
  full_treatment
}

get_data <- function(filename, folder, treatments){
  #Input: A datafile name as a string, the folder where the data is located, and a list of treatment names
  #       Ex: "HostVals_VT0.6_SEED10.data", "VertTrans_Data/", and c("VT")
  #Output: A tibble containing all data in the file with a column added for the Seed and for all treatment names
  #       Ex: The columns added for the above data would be SEED with all values at 10 and 
  #           VT with all values at 0.6
  seed <- str_extract(filename, "SEED[\\d]+.")
  seed_val <- str_extract(seed, "[\\d]+")
  
  full_filename <- str_c(c(folder, filename), sep="", collapse="")
  data <- read_csv(full_filename) %>% mutate(SEED = seed_val)
  
  for (i in c(1:length(treatments))){
    data <- data %>% mutate(!!treatments[i] := as.factor(get_treatment_val(filename, treatments[i])))
  }
  data
}

combine_time_data <- function(filenames, folder, treatments){
  #Input: A list of relevant datafile names, the folder where the data is, and a list of treatment names
  #       Ex: c("HostVals_VT0.6_SEED10.data", "HostVals_VT0.8_SEED10.data"), "VertTrans_Data/", 
  #           and c("VT")
  #Output: A tibble with all data from all datafiles given, with columns added for the seed and 
  #       all treatment names
  all_data <- get_data(filenames[1], folder, treatments)
  
  if(length(filenames) > 1){
    for (i in c(2:length(filenames))){
      add_data <- get_data(filenames[i], folder, treatments)
      all_data <- full_join(all_data, add_data)
    }
  }
  all_data %>% mutate(SEED = as.factor(SEED))
}

combine_histogram_data <- function(time_data, separate_by, bins){
  #Input: The time series data produced by combine_time_data, the column name 
  #       which the stacked histogram will be facet wrapped by,
  #       and 1:1 list for the desired names of histogram bins
  #       Ex: lysischances, "PLR", and
  #           lysis_histogram_bins <- c(Hist_0.0 = "0 to 0.2 (Highly lysogenic)",
                          # Hist_0.1 = "0 to 0.2 (Highly lysogenic)",
                          # Hist_0.2 = "0.2 to 0.4 (Moderately lysogenic)",
                          # Hist_0.3 = "0.2 to 0.4 (Moderately lysogenic)",
                          # Hist_0.4 = "0.4 to 0.6 (Temperate)",
                          # Hist_0.5 = "0.4 to 0.6 (Temperate)",
                          # Hist_0.6 = "0.6 to 0.8 (Moderately lytic)",
                          # Hist_0.7 = "0.6 to 0.8 (Moderately lytic)",
                          # Hist_0.8 = "0.8 to 1.0 (Highly lytic)",
                          # Hist_0.9 = "0.8 to 1.0 (Highly lytic)")
  #Output: A tibble with 4 columns: update, treatment, Histogram_bins, and count. 
  #       This is the format needed to create a stacked histogram.
  
  #collect initial data and pivot longer
  initial_data <- time_data %>%
    pivot_longer(
      cols = starts_with("Hist_"),
      names_to = "Histogram_bin",
      values_to = "Bin_count"
    ) %>%
    mutate(Histogram_bin = as.factor(Histogram_bin))
  
  #collapse the bins
  collapsed_data <- initial_data
  collapsed_data$Histogram_factor <- dplyr::recode(collapsed_data$Histogram_bin, !!!bins)
  
  #aggregate the counts
  collapsed_data <- collapsed_data %>%
    mutate(count = Bin_count,
           treatment = get(separate_by)) %>%
    select(treatment, update, Histogram_factor, count)
  
  final_data <- aggregate(list(count = collapsed_data$count), 
                          list(update = collapsed_data$update,
                               treatment = collapsed_data$treatment,
                               Histogram_bins = collapsed_data$Histogram_factor), sum)
  
  final_data 
}

#------------General settings-----------
folder <- "Data/" #Folder where datafiles are 
treatments <- c("PLR", "COI") #Names of the treatments being tested - should match what is in filenames

#separate_by vals indicate what the stacked histograms should be facet wrapped by
separate_by <- "SEED" 
#How stacked histogram bins should be collapsed and renamed
histogram_bins <- c(Hist_0.0 = "0 to 0.2 (First bin)", 
                    Hist_0.1 = "0 to 0.2 (First bin)",
                    Hist_0.2 = "0.2 to 0.4 (Second bin)",
                    Hist_0.3 = "0.2 to 0.4 (Second bin)",
                    Hist_0.4 = "0.4 to 0.6 (Third bin)",
                    Hist_0.5 = "0.4 to 0.6 (Third bin)",
                    Hist_0.6 = "0.6 to 0.8 (Fourth bin)",
                    Hist_0.7 = "0.6 to 0.8 (Fourth bin)",
                    Hist_0.8 = "0.8 to 1.0 (Fifth bin)",
                    Hist_0.9 = "0.8 to 1.0 (Fifth bin)")

all_filenames <- list.files(folder)

#--------------Read and munge actual data------------
hostval_filenames <- str_subset(all_filenames, "HostVals")
symvals_filenames <- str_subset(all_filenames, "SymVals")

hostvals <- combine_time_data(hostval_filenames, folder, treatments)
symvals <- combine_time_data(phagevals_filenames, folder, treatments)

symval_histdata <- combine_histogram_data(symvals, separate_by, histogram_bins)


#-------------Graph templates----------------
#All values with this format: <VAL_NAME> need to be filled in
timeseries_graph <- ggplot(data= <TIME_SERIES_DATASET_NAME>,
                         aes(x=update, y= <Y_AXIS_VAL>, 
                             group=<GROUP_VAL_NAME>, colour=<GROUP_VAL_NAME>)) + 
  ylab(<Y_AXIS_LABEL>) + xlab("Updates") + 
  stat_summary(aes(color=<GROUP_VAL_NAME>, fill=<GROUP_VAL_NAME>),
               fun.data="mean_cl_boot", geom=c("smooth"), se=TRUE) + 
  theme(panel.background = element_rect(fill='white', colour='black')) +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  guides(fill=FALSE) + 
  scale_colour_manual(values=viridis(nlevels(<TIME_SERIES_DATASET_NAME>$<GROUP_VAL_NAME>))) + 
  scale_fill_manual(values=viridis(nlevels(<TIME_SERIES_DATASET_NAME>$<GROUP_VAL_NAME>)))

#Facet wrapping is optional, but can be used if multiple treatments are being used
timeseries_graph + facet_wrap(<GROUP_VAL2_NAME>) 

#All values with this format: <VAL_NAME> need to be filled in
stackedhistogram <- ggplot(data = <HISTOGRAM_DATASET_NAME>, 
                                       aes(update, count)) + 
  geom_area(aes(fill=Histogram_bins), position='stack') +
  ylab(<Y_AXIS_LABEL>) + xlab("Evolutionary time (in updates)") +
  scale_fill_manual(<LEGEND_TITLE>,values=viridis(nlevels(<HISTOGRAM_DATASET_NAME>$Histogram_bins))) +
  facet_wrap(~treatment) + 
  theme(panel.background = element_rect(fill='light grey', colour='black')) + 
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + 
  guides(fill=FALSE) + guides(fill = guide_legend())

stackedhistogram


#-------------Examples----------------
hostcount_plot <- ggplot(data=hostvals,
                         aes(x=update, y=count, 
                             group=PLR, colour=PLR)) + 
  ylab("Host Count") + xlab("Updates") + 
  stat_summary(aes(color=PLR, fill=PLR),
               fun.data="mean_cl_boot", geom=c("smooth"), se=TRUE) + 
  theme(panel.background = element_rect(fill='white', colour='black')) +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  guides(fill=FALSE) + scale_colour_manual(values=viridis(nlevels(hostvals$PLR))) + 
  scale_fill_manual(values=viridis(nlevels(hostvals$PLR)))

hostcount_plot + facet_wrap(~COI)

symintval_plot <- ggplot(data=symvals, aes(x=update, y=mean_intval,
                                               group=PLR, color=PLR)) +
  ylab("Symbiont Interaction value") + xlab("Updates") + 
  stat_summary(aes(color=PLR, fill=PLR),
               fun.data="mean_cl_boot", geom=c("smooth"), se=TRUE) + 
  theme(panel.background = element_rect(fill='white', colour='black')) +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  guides(fill=FALSE) + scale_colour_manual(values=viridis(nlevels(symvals$PLR))) + 
  scale_fill_manual(values=viridis(nlevels(symvals$PLR)))

symintval_plot + facet_wrap(~COI)

symval_stackedhistogram <- ggplot(symval_histdata, 
                                       aes(update, count)) + 
  geom_area(aes(fill=Histogram_bins), position='stack') +
  ylab("Count of Symbionts with Phenotype") + xlab("Evolutionary time (in updates)") +
  scale_fill_manual("Interaction Val\n Phenotypes",values=viridis(nlevels(symval_histdata$Histogram_bins))) +
  facet_wrap(~treatment) + 
  theme(panel.background = element_rect(fill='light grey', colour='black')) + 
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) + 
  guides(fill=FALSE) + guides(fill = guide_legend())

symval_stackedhistogram