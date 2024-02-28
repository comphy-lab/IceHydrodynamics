#!/bin/bash

Rhor_AirOcean="1e-3" 
Mur_AirOcean="1e-2" 
Ga="1e4" 
A0="0.01"
tmax="2.0"
DT="1e-3" 
level="7" 

qcc -fopenmp -Wall -O2 WavesWater.c -o WavesWater -lm


export OMP_NUM_THREADS=8
./WavesWater $Rhor_AirOcean $Mur_AirOcean $Ga $A0 $tmax $DT $level $g

python3 VideoWaterWaves.py
ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4
