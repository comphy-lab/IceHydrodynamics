#!/bin/bash

Rhor_IceOcean="1e0"
Rhor_AirOcean="1e-3" 
Ga="1e7" 
Mur_IceOcean="0" #Sheet is inviscid
Mur_AirOcean="1e-2" 
A0_OceanIce="0.01"
A0_IceAir="0.01"
hIce="0.1"
lambda_Ocean="1.0"
lambda_IceAir="1.0"
ElasticModulus="1.0"

tmax="2.0"
DT="1e-3" 
level="7" 

qcc -fopenmp -Wall -O2 IceOnWaterWaves.c -o IceOnWaterWaves -lm


export OMP_NUM_THREADS=8
./IceOnWaterWaves $Rhor_IceOcean $Rhor_AirOcean $Ga $Mur_IceOcean \
$Mur_AirOcean $A0_OceanIce $A0_IceAir $hIce $lambda_Ocean \
$ElasticModulus $lambda_IceAir $tmax $DT $level

python3 VideoGeneralCase.py
ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4


