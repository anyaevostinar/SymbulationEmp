
#Command line arguments
args = commandArgs(trailingOnly=TRUE)
if (length(args) != 1) {
  stop("Exactly 1 argument must be supplied (data file).n", call.=FALSE)
}

#File locations
raw_data_path <- 'raw_data/'
raw_data_file <- args[1]
collated_data_file <- 'collated_data.data'


#Munge data file name into treatment values
treatment <- raw_data_file#substr(raw_data_file,start=2,stop=nchar(raw_data_file))
treatment <- strsplit(treatment, '_')[[1]]
treatment <- strsplit(treatment, '=')
frame = data.frame(lapply(lapply(treatment, '[[', 2), as.numeric))
colnames(frame) <- as.character(lapply(treatment, '[[', 1))

#Set version (version was coerced into numeric and lost)
frame[1,1] <- treatment[[1]][2]

#Append file name
frame$file_name = c(raw_data_file)

#Extract desired values from raw data output from symbulation
host_data <- read.table(paste(raw_data_path, 'HostVals_', raw_data_file, '.data', sep=""), sep=",", header=TRUE)
sym_data <- read.table(paste(raw_data_path, 'SymVals_', raw_data_file, '.data', sep=""), sep=",", header=TRUE)

frame$host_count <- as.numeric(tail(host_data,n=1)["count"])
frame$sym_count <- as.numeric(tail(sym_data,n=1)["count"])

#Compute desired values
frame$survival <- frame$host_count / frame$POP
if(frame$host_count[1] == 0) {
	frame$moi <- c(0)
} else {
	frame$moi <- frame$sym_count / frame$host_count
}

#Note: if the following takes too long, it can be replaced with appending, but this may hinder the capability to keep old data when adding or removing columns. reprocess_data.py may solve this issue.

#Collate new data into old
if(file.exists(collated_data_file) && file.info(collated_data_file)[1,1] != 0) { #file exists and is not empty
	require(plyr)
	collated_data <- rbind.fill(read.table(collated_data_file,h=T),frame)
} else {
	collated_data <- frame
}

#Save collated data
write.table(collated_data, collated_data_file, row.names=FALSE)
