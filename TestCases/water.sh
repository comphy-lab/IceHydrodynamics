#!/bin/bash

#Physical variables
Rhor_AirOcean="1e-3" 
Mur_AirOcean=("1e2" "1e3" "1e4" "1e5" "1e6" "1e7" "1e8" "1e9" "1e10" "1e11" "1e12" "1e13" "1e14" "1e15")
Ga="1e9"
A0="0.01"

tmax="20.0"
DT="1e-3" 
level="7" 

#Repository cases 
folder="1006"

#compile the file
qcc -fopenmp -Wall -O2 WavesWater.c -o WavesWater -lm

#Loop over the cases
for ((i1=0; i1<${#Ga[*]}; i1++)); do
    for ((i2=0; i2<${#Mur_AirOcean[*]}; i2++)); do

    #make a repository
    mkdir -p $folder 
    touch parameters.txt 
    scp -r WavesWater $folder/

    #print out of the variables 
    echo Rhor_AirOcean $Rhor_AirOcean Mur_AirOcean ${Mur_AirOcean[$i2]} \
        Ga ${Ga[$i1]} A0 $A0 \
        tmax $tmax DT $DT level $level > parameters.txt

    #run the file 
    export OMP_NUM_THREADS=8
    ./WavesWater $Rhor_AirOcean ${Mur_AirOcean[$i2]} ${Ga[$i1]} \
    $A0 $tmax $DT $level \
    

    #Make a video
    python3 VideoWaterWaves.py
    ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4

    #Moving files instead of copying
    mv intermediate Video amplitude.dat logfile.dat Movie.mp4 parameters.txt  $folder/
    mv $folder WavesWater_

    #increment folder
    ((folder++))

     done
done