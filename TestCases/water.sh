#!/bin/bash

#Physical variables
Rhor_AirOcean="1e-3" 
Mur_AirOcean="1e-2" 
Ga="1e4"
A0="0.1"

tmax="10.0"
DT="1e-3" 
level="7" 

#Repository cases 
start="1005"
end="1005"

#compile the file
qcc -fopenmp -Wall -O2 WavesWater.c -o WavesWater -lm

#Loop over the cases
for i in `seq $start $end`;
do

#make a repository
mkdir -p $i
scp -r WavesWater water.sh $i/

#print out of the variables 
echo Rhor_AirOcean $Rhor_AirOcean Mur_AirOcean $Mur_AirOcean Ga ${Ga[$i-$start]}\
A0 $A0 tmax $tmax DT $DT level $level

#run the file 
export OMP_NUM_THREADS=8
./WavesWater $Rhor_AirOcean $Mur_AirOcean ${Ga[$i-$start]} $A0 $tmax $DT $level

#Make a video
python3 VideoWaterWaves.py
ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4

# Moving files instead of copying
mv intermediate Video amplitude.dat logfile.dat Movie.mp4 $i/
mv $i WavesWater_

done

