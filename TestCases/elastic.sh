#!/bin/bash

Rhor_IceOcean="1e0"
Rhor_AirOcean="1e-3" 
Ga="1e7" 
Mur_IceOcean="1e3" 
Mur_AirOcean="1e-2" 
A0_OceanIce="0.01"
A0_IceAir="0.01"
hIce="0.01"
lambda_Ocean="1.0"
lambda_IceAir="1.0"

tmax="2.0"
DT="1e-3" 
level="7" 

qcc -fopenmp -Wall -O2 IceOnWaterWaves.c -o IceOnWaterWaves -lm


export OMP_NUM_THREADS=8
./IceOnWaterWaves $Rhor_IceOcean $Rhor_AirOcean $Ga $Mur_IceOcean \
$Mur_AirOcean $A0_OceanIce $A0_IceAir $hIce $lambda_Ocean \
$lambda_IceAir $tmax $DT $level


