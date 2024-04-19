#!/bin/bash

Rhor_IceOcean="1e0"
Rhor_AirOcean="1e-3" 
Ga="1e9"
Mur_IceOcean="1e5"
Mur_AirOcean="1e-2" 
A0_OceanIce="0.01"
A0_IceAir="0.01"
hIce="0.1"
lambda_Ocean="1.0"
lambda_IceAir="1.0"

tmax="20.0"
DT=("1e-4" "1e-5" "1e-6") 
level=("8" "9" "10")

#Repository cases 
folder="2152"

#compile the file
qcc -fopenmp -Wall -O2 ViscousOilOnWaterWaves.c -o ViscousOilOnWaterWaves -lm

#Loop over the cases
for ((i1=0; i1<${#DT[*]}; i1++)); do
    for ((i2=0; i2<${#level[*]}; i2++)); do

    #make a repository
    mkdir -p $folder 
    touch parameters.txt 
    scp -r ViscousOilOnWaterWaves $folder/

    #print out of the variables 
    echo Rhor_IceOcean $Rhor_IceOcean Rhor_AirOcean $Rhor_AirOcean Ga $Ga Mur_IceOcean $Mur_IceOcean \
        Mur_AirOcean $Mur_AirOcean A0_OceanIce $A0_OceanIce A0_IceAir $A0_IceAir hIce $hIce lambda_Ocean $lambda_Ocean \
        lambda_IceAir $lambda_IceAir tmax $tmax DT ${DT[$i1]} level ${level[$i2]} > parameters.txt

    #run the file 
    export OMP_NUM_THREADS=8
    ./ViscousOilOnWaterWaves $Rhor_IceOcean $Rhor_AirOcean $Ga $Mur_IceOcean \
    $Mur_AirOcean $A0_OceanIce $A0_IceAir $hIce $lambda_Ocean \
    $lambda_IceAir $tmax ${DT[$i1]} ${level[$i2]}

    #Make a video
    python3 VideoGeneralCase.py
    ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4

    #Moving files instead of copying
    mv intermediate Video amplitude.dat logfile.dat Movie.mp4 parameters.txt  $folder/
    mv $folder ViscousOilOnWaterWaves_

    #increment folder
    ((folder++))

     done
done
