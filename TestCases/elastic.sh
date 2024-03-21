#!/bin/bash

Rhor_IceOcean="1e0"
Rhor_AirOcean="1e-3" 
Ga="1e8" 
Mur_IceOcean="0" #Sheet is inviscid
Mur_AirOcean="1e-2" 
A0_OceanIce="0.01"
A0_IceAir="0.01"
hIce="0.1"
lambda_Ocean="1.0"
lambda_IceAir="1.0"
ElasticModulus=("1e-5" "1e-4" "1e-3" "1e-2" "1e-1" "1e0")

tmax="20.0"
DT="1e-4"
level="8" 
L_box="6"

#Repository cases 
start="3003"
end="3008"

#compile the file
qcc -fopenmp -Wall -O2 IceOnWaterWaves.c -o IceOnWaterWaves -lm

#Loop over the cases
for i in `seq $start $end`;
do

#make a repository
mkdir -p $i
scp -r IceOnWaterWaves elastic.sh $i/

echo Rhor_IceOcean $Rhor_IceOcean Rhor_AirOcean $Rhor_AirOcean Ga $Ga Mur_IceOcean $Mur_IceOcean \
Mur_AirOcean $Mur_AirOcean A0_OceanIce $A0_OceanIce A0_IceAir $A0_IceAir hIce $hIce lambda_Ocean $lambda_Ocean \
ElasticModulus ${ElasticModulus[$i-$start]} lambda_IceAir $lambda_IceAir tmax $tmax DT $DT level $level L_box $L_box

#run the file 

export OMP_NUM_THREADS=8
./IceOnWaterWaves $Rhor_IceOcean $Rhor_AirOcean $Ga $Mur_IceOcean \
$Mur_AirOcean $A0_OceanIce $A0_IceAir $hIce $lambda_Ocean \
${ElasticModulus[$i-$start]} $lambda_IceAir $tmax $DT $level $L_box

#Make a video
python3 VideoGeneralCase.py
ffmpeg -framerate 30 -pattern_type glob -i 'Video/*.png' -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -c:v libx264 -pix_fmt yuv420p Movie.mp4

# Moving files instead of copying
mv intermediate Video amplitude.dat logfile.dat Movie.mp4 $i/
mv $i IceOnWaterWaves_

done



