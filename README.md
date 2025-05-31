# 2D Multi-Fluid Ice Shelf Simulation

This project models the dynamic interactions in a simplified 2D ice-shelf system within an air–ocean environment. The simulations are implemented in **Basilisk C**, a powerful open-source framework for computational fluid dynamics (CFD).

## Overview

The main objective is to simulate and analyze the coupled behavior of air, ocean water, and glacial ice in a 2D domain, using various physical assumptions for each phase. Three primary model configurations were developed:

1. **Two-Fluid Viscous Model (Air/Ocean)**  
   File: `TestCases/Waveswater.c`  
   Simulates the interaction between air and ocean as incompressible viscous fluids, representing the base environment for ice shelf dynamics.

2. **Three-Fluid Viscous Model (Air/Ice/Ocean)**  
   File: `TestCases/ViscousOilOnWaterWaves.c`  
   Introduces a third viscous fluid (representing ice) between air and ocean to better capture physical layering in the ice shelf system.

3. **Three-Fluid Viscoelastic Model (Air/Ice/Ocean)**  
   File: `TestCases/IceOnWaterWaves.c`  
   Enhances the ice layer by modeling it as a **viscoelastic medium**, reflecting its realistic mechanical behavior under stress and deformation.

Each model uses Basilisk’s **volume-of-fluid (VOF)** method to track fluid interfaces and a **multiphase Navier-Stokes solver** for fluid dynamics. The viscoelastic behavior is captured through custom rheological modules.

## File Structure

- `README.md`
- `TestCases/`
  - `Waveswater.c` – Two-fluid (Air/Ocean) viscous model
  - `ViscousOilOnWaterWaves.c` – Three-fluid (Air/Ice/Ocean) viscous model
  - `IceOnWaterWaves.c` – Three-fluid with viscoelastic ice
- `src-IceHydro/`
  - `log-conform-IceElastic.h` – Log-conformation viscoelastic model
  - `reduced-IceHydro.h` – Simplified viscoelastic model
  - `two-phase-IceHydro.h` – Two-phase hydrodynamics
- `output/`
  - `logfile.dat` – Time series output of simulation logs
  - `amplitude.dat` – Recorded interface wave amplitudes


## Author

**Valentin Rosario**  
*Master’s Thesis Project (2023–2024)*  
**Title:** *Modelling the Ward Hunt Ice Shelf as a Viscoelastic Solid*  
**Institution:** University of Amsterdam (UvA)  
**Supervisors:** Vatsal Sanjay, Mazi Jalaal

## Acknowledgements

Thanks to Vatsal Sanjay, Stéphane Popinet and the Basilisk community for providing the computational tools and insight used in this work.

