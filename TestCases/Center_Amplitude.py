# Author: Vatsal Sanjay
# vatsalsanjay@gmail.com
# Physics of Fluids
# Last updated: 22-Oct-2023

#Aim: return the amplitude of the interface center 


import numpy as np
import os
import subprocess as sp
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.collections import LineCollection
from matplotlib.ticker import StrMethodFormatter
import pandas as pd
import concurrent.futures
import sys
import argparse


def execute_process(exe):
    p = sp.Popen(exe, stdout=sp.PIPE, stderr=sp.PIPE)
    stdout, stderr = p.communicate()
    return stderr.decode("utf-8").split("\n")

def get_segs(place, Ocean=False):
    if Ocean:
        temp2 = execute_process(["./getFacetsOceanIce", place])
    else:
        temp2 = execute_process(["./getFacets", place])
    segs = []
    skip = False
    temp2 = list(filter(None, temp2))
    for n1 in range(len(temp2)):
        temp3 = temp2[n1].split(" ")
        if not skip:
            temp4 = temp2[n1+1].split(" ")
            x1, y1 = map(float, [temp3[0], temp3[1]])
            x2, y2 = map(float, [temp4[0], temp4[1]])
            segs.extend([((x1, y1),(x2, y2))])
            skip = True
        else:
            skip = False
    return segs


def process_file(t):
    place = f"intermediate/snapshot-{t:6.5f}"

    if not os.path.exists(place):
        print(f"{place} File not found!")
        return
    
    #Get Facets for the Ice and Ocean interface
    Ice_interface = get_segs(place)
    Ocean_interface = get_segs(place, Ocean=True)
    
    #Pick the most central facet and take it's y value which corresponds to Amplitude
    Amp_Ocean = pick_center(Ocean_interface)
    Amp_Ice = pick_center(Ice_interface, Ice =True)
   
    return Amp_Ocean, Amp_Ice
    
    

def pick_center(segs, Ice=False):
    segs_np = np.array(segs)
    x_line = segs_np[:, :, 0].flatten()
    y_line = segs_np[:, :, 1].flatten()
    
    #Picking the facet which has an x value located at the center of the domain (x=1.0)
    Amp_center = y_line[np.abs(x_line - 1).argmin()]
    
    if Ice:
    	return Amp_center - 0.1    
    else:
    	return Amp_center

    
def main():
    # Set up command-line argument parser
    parser = argparse.ArgumentParser(description='Select Amplitude of the interface center')
    parser.add_argument('--output_file', type=str, default='center_amplitude.txt', help='Output file to store interface center amplitudes')
    parser.add_argument('--num_workers', type=int, default=8, help='Number of workers')
    parser.add_argument('--tSnap', type=float, default=0.01, help='Snapshot time interval')

    args = parser.parse_args()

    nGFS = 2000
    num_workers = args.num_workers
    tSnap = args.tSnap
    
    output_file = args.output_file

    with open(output_file, 'w') as file:
        file.write("t AmpOcean AmpIce\n")  # Header
        for ti in range(nGFS):
            t = ti*tSnap
            Amp_Ocean, Amp_Ice = process_file(t)
            if Amp_Ocean is not None and Amp_Ice is not None:
                file.write(f"{t} {Amp_Ocean} {Amp_Ice}\n")
                print(f"Done with Snapshot {t}! Ocean Amplitude - {Amp_Ocean}, Ice Amplitude - {Amp_Ice}")
            else:
                print(f"Skipped Snapshot {t} due to missing file or image")

if __name__ == "__main__":
    main()   
