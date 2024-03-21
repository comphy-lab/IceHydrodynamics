#!/bin/bash

Rhor_IceOcean="1e0"
Rhor_AirOcean="1e-3" 
Ga="1e8"
Mur_IceOcean=("1e2" "1e3" "1e4" "1e5" "1e6" "1e7" "1e8" "1e9" "1e10" "1e11" "1e12")
Mur_AirOcean="1e-2" 
A0_OceanIce="0.01"
A0_IceAir="0.01"
hIce="0.1"
lambda_Ocean="1.0"
lambda_IceAir="1.0"

tmax="20.0" 
DT="1e-3" 
level="7"

#Repository cases 
start="2036"
end="2046"

#compile the file
qcc -fopenmp -Wall -O2 ViscousOilOnWaterWaves.c -o ViscousOilOnWaterWaves -lm

#Loop over the cases
for i in `seq $start $end`;
do

#make a repository
mkdir -p $i
scp -r ViscousOilOnWaterWaves oil.sh $i/


#print out of the variables 
echo Rhor_IceOcean $Rhor_IceOcean Rhor_AirOcean $Rhor_AirOcean Ga $Ga Mur_IceOcean ${Mur_IceOcean[$i-$start]} \
Mur_AirOcean $Mur_AirOcean A0_OceanIce $A0_OceanIce A0_IceAir $A0_IceAir hIce $hIce lambda_Ocean $lambda_Ocean \
lambda_IceAir $lambda_IceAir tmax $tmax DT $DT level $level


#run the file 
export OMP_NUM_THREADS=8
./ViscousOilOnWaterWaves $Rhor_IceOcean $Rhor_AirOcean $Ga ${Mur_IceOcean[$i-$start]} \
$Mur_AirOcean $A0_OceanIce $A0_IceAir $hIce $lambda_Ocean \
$lambda_IceAir $tmax $DT $level

#Make a video
python3 VideoGeneralCase.py
ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4

# Moving files instead of copying
mv intermediate Video amplitude.dat logfile.dat Movie.mp4 $i/
mv $i ViscousOilOnWaterWaves_

done