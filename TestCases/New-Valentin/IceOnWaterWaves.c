#include "navier-stokes/centered.h"
#define FILTERED
#include "../src-IceHydro/two-phase-IceHydro.h"
#include "../src-IceHydro/log-conform-IceElastic.h"
#include "../src-IceHydro/reduced-IceHydro.h"

// note that here, ice is just a proxy for the thin layer on top of the bath!!

// densities
#define Rhor_IceOcean (1e0) // this is the density ratio of thin layer on top of ocean and ocean...
#define Rhor_AirOcean (1e-1) // this is the density ratio of air to that of the ocean...

// viscosities
#define Ga 1e4 // Galileo number: gL^3/(\mu/\rho)^2 -- this is based on the viscosity of ocean
#define Mur_IceOcean 0. // (the sheet shouldn't be viscous) this is the viscosity ratio of thin layer on top of ocean and ocean...
#define Mur_AirOcean 1e-2  // this is the viscosity ratio of air to that of the ocean...

// initial amplitudes
#define A0_OceanIce 0.1 // initial amplitude of the ocean-ice interface
#define A0_IceAir 0.1 // initial amplitude of the ice-air interface

// thickness of the ice sheet
#define hIce 0.1

// Elastic properties of the ice sheet
#define ElasticNumber 1.0 // this is the ratio of the elastic modulus of the ice sheet to gravitational head: rho_Ocean*g*Lambda_Ocean

// wave length of the surface waves.. lambda_Ocean is always 1. change lambda_IceAir to control the asymmetry of the standing waves
#define lambda_Ocean 1.0 // fix this to 1 always.. this is the length scale of the problem
#define lambda_IceAir 1.0 // this is the ratio of the wave length of the ice-air interface to that of the ocean-ice interface

#define tmax (20.)
#define tsnap (tmax/100.)

#define fErr (1e-3)
#define VelErr (1e-2)
#define OmegaErr (1e-3)
#define KAPPAErr (1e-4)

char nameOut[80], amplitudeFile[80], name[80];
static FILE * fp1 = NULL;
static FILE * fp2 = NULL;

// grid resolution 
#define MAXlevel 9 // maximum level of refinement
#define MINlevel 0 // minimum level of refinement

int LEVEL;

/*Boundary conditions:*/
u.n[top] = neumann(0.);
p[top] = dirichlet(0.);
/*default: 
u.n[wall] = dirichlet(0.); 
EVERYTHING_ELSE[wall] = neumann(0.);*/

/**
The initial condition is a small amplitude plane wave of wavelength ($\lambda$)
unity. This wavelength is the relevant length scale for this problem. Note that $k = 2\pi$ */
event init (t = 0) {
  fraction (f, - (y-hIce) + A0_IceAir*cos (2.*pi*x/lambda_IceAir));
  fraction (fOcean, - (y) + A0_OceanIce*cos (2.*pi*x/lambda_Ocean));
}

scalar GIced[];
int main() {

  L0 = 2.0;
  Y0 = -L0/2.;
  // f.sigma = 0.0;
  //TOLERANCE = 1e-6;
  DT = 1e-3;

  LEVEL = MAXlevel;
  init_grid(1 << LEVEL);
  
  rhoOcean = 1.0; rhoIce = Rhor_IceOcean; rhoAir = Rhor_AirOcean;
  muOcean = 1./sqrt(Ga); muIce = Mur_IceOcean/sqrt(Ga); muAir = Mur_AirOcean/sqrt(Ga);

  G.y = -1.; // acceleration due to gravity
  // Ice-sheet
  GIce = GIced;

  char comm[80];
  sprintf (comm, "mkdir -p intermediate");
  system(comm);

  fp1 = fopen ("logfile.dat", "w");
  fp2 = fopen ("amplitude.dat", "w");

  run();

  fclose (fp1);
  fclose (fp2);
}

event properties (i++) {
  foreach () {
    GIced[] = (f[]*(1.-fOcean[]) < (1e-6) ? 0.: ElasticNumber); // this is an artificial patch for now. The code has issues with VE terms in the interfacial cells!
  }
}

scalar omega[], KAPPA[], KAPPAOcean[];
event adapt (i++) {
  curvature(f, KAPPA);
  curvature(fOcean, KAPPAOcean);
  vorticity (u, omega);
  adapt_wavelet ((scalar *){f, u.x, u.y, omega, KAPPA, KAPPAOcean},
     (double[]){fErr, VelErr, VelErr, OmegaErr, KAPPAErr, KAPPAErr},
      maxlevel = MAXlevel, minlevel = MINlevel);
}

/**
## Writing Output files
*/

event writingFiles (t = 0; t += tsnap; t <= tmax) {
  dump (file = "dump");
  sprintf (nameOut, "intermediate/snapshot-%6.5f", t);
  dump(file=nameOut);
}

/**
The calculation of amplitude of the surface wave is same as done [(here)](http://basilisk.fr/src/test/capwave.c).<br/>
By default tracers are defined at $t-\Delta t/2$. We use the *first*
keyword to move VOF advection before the *amplitude* output i.e. at
$t+\Delta/2$. This improves the results. */

event vof (i++, first);

/**
We output the amplitude of the standing surface wave.
*/

event amplitude (i++) {

  /**
  To get an accurate amplitude, we reconstruct interface position
  (using height functions) and take the corresponding maximum. */

  scalar pos[], pofOcean[];
  position (f, pos, {0,1});
  position (fOcean, pofOcean, {0,1});
  double max = statsf(pos).max;
  double maxOcean = statsf(pofOcean).max;

  /**
  We output the corresponding evolution in a file indexed with the
  case number. */
  if (i == 0) {
    fprintf (fp2, "t AmpOcean AmpIce\n");
  }
  fprintf (fp2, "%g %g %g\n", t, maxOcean, max-hIce);
  fflush (fp2);

}

event logfile (i++) {
  double ke = 0.;
  foreach (reduction(+:ke)){
    ke += sq(Delta)*(sq(u.x[]) + sq(u.y[]))*f[];
  }
  fprintf (fp1, "%g %g %d\n", t, ke, mgp.i);
  fprintf (ferr, "%d %g %g %d\n", i, t, ke, mgp.i);
  fflush (fp1);
}