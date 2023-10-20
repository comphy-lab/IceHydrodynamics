#include "navier-stokes/centered.h"
#define FILTERED
#include "../src-IceHydro/two-phase-IceHydro.h"
#include "../src-IceHydro/reduced-IceHydro.h"

/**
The density and viscosity of the upper fluid is so low that it does not
effect the flow inside the first one, resulting in a surface wave.
*/
#define Rhor (1e-1)
#define MUr (1e-2)

// Galileo number: gL^3/(\mu/\rho)^2
#define Ga 1e4
// initial amplitude of the ocean wave
#define A0 0.1
// thickness of the ice sheet
#define hIce 0.1

#define tmax (4.)
#define tsnap (tmax/100.)

#define fErr (1e-3)
#define VelErr (1e-2)
#define OmegaErr (1e-3)

char nameOut[80], amplitudeFile[80], name[80];
static FILE * fp1 = NULL;
static FILE * fp2 = NULL;
#define MAXlevel 7
#define MINlevel 0
int LEVEL;

uf.n[left]   = 0.;
uf.n[right]  = 0.;
uf.n[top]    = 0.;
uf.n[bottom] = 0.;

/**
The initial condition is a small amplitude plane wave of wavelength ($\lambda$)
unity. This wavelength is the relevant length scale for this problem. Note that $k = 2\pi$ */
event init (t = 0) {
  fraction (f, - (y-hIce) + A0*cos (2.*pi*x));
  fraction (fOcean, - (y) + A0*cos (2.*pi*x));
}

int main() {

  L0 = 2.0;
  Y0 = -L0/2.;
  // f.sigma = 0.0;
  TOLERANCE = 1e-6;
  DT = 1e-2;

  LEVEL = MAXlevel;
  init_grid(1 << LEVEL);
  
  rhoOcean = 1.0; rhoIce = 1.0; rhoAir = Rhor;
  muOcean = 1./sqrt(Ga); muIce = 1./sqrt(Ga); muAir = MUr/sqrt(Ga);

  G.y = -1.; // acceleration due to gravity

  char comm[80];
  sprintf (comm, "mkdir -p intermediate");
  system(comm);

  fp1 = fopen ("logfile.dat", "w");
  fp2 = fopen ("amplitude.dat", "w");

  run();

  fclose (fp1);
  fclose (fp2);
}

scalar omega[], KAPPA[], KAPPAOcean[];
event adapt (i++) {
  curvature(f, KAPPA);
  curvature(fOcean, KAPPAOcean);
  vorticity (u, omega);
  adapt_wavelet ((scalar *){f, u.x, u.y, omega, KAPPA, KAPPAOcean},
     (double[]){fErr, VelErr, VelErr, OmegaErr, 1e-4, 1e-4},
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