#saving this script to easily grab out an evenly spaced set of colors from the full cube helix set

fullcubeHelix <- c("#673F03", "#7D3002", "#891901", "#A7000F", "#B50142", "#CD0778", "#D506AD", "#E401E7", "#AB08FF","#7B1DFF", "#5731FD","#5E8EFF", "#4755FF" ,"#6FC4FE", "#86E9FE", "#96FFF7", "#B2FCE3", "#BBFFDB", "#D4FFDD", "#EFFDF0")
num_lines <- 7 #make this how many things you plan to plot
m <- length(fullcubeHelix) 
step <- m/num_lines
step <- (step)-(step%%1)
a <- 1
colors <- c(fullcubeHelix[a])
for (i in 1:(num_lines-1))
{
  a <- a + step
  colors <- append(colors, temp[a])
}
print(colors)
