#include "navier-stokes/centered.h"
#define FILTERED
#include "../src-IceHydro/two-phase-IceHydro.h"
#include "../src-IceHydro/log-conform-IceElastic.h"
#include "../src-IceHydro/reduced-IceHydro.h"

// note that here, ice is just a proxy for the thin layer on top of the bath!!

#define fErr (1e-3)
#define VelErr (1e-2)
#define OmegaErr (1e-3)
#define KAPPAErr (1e-4)

char nameOut[80], amplitudeFile[80], name[80];
static FILE * fp1 = NULL;
static FILE * fp2 = NULL;

double Rhor_IceOcean, Rhor_AirOcean, Ga, Mur_IceOcean, Mur_AirOcean;
double hIce, lambda_Ocean, lambda_IceAir, A0_OceanIce, A0_IceAir;
double ElasticModulus, tmax, step;
#define tsnap (0.01)

int MAXlevel;
#define MINlevel 0 // minimum level of refinement


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
int main(int argc, char const *argv[]) {
  if (argc < 14){
    fprintf(ferr, "Lack of command line arguments. Check! Need %d more arguments\n",14-argc);
    return 1;
  }

  // density and viscosity ratios
  Rhor_IceOcean = atof(argv[1]);
  Rhor_AirOcean = atof(argv[2]);
  Ga = atof(argv[3]);
  Mur_IceOcean = atof(argv[4]);
  Mur_AirOcean = atof(argv[5]);
  A0_OceanIce = atof(argv[6]);
  A0_IceAir = atof(argv[7]);
  hIce = atof(argv[8]);
  lambda_Ocean = atof(argv[9]);
  lambda_IceAir = atof(argv[10]);
  ElasticModulus = atof(argv[11]);
  tmax = atof(argv[12]);
  step = atof(argv[13]);
  MAXlevel = atoi(argv[14]); 

  L0 = 2.0;
  Y0 = -L0/2.;
  DT = step;
  // f.sigma = 0.0;
  //TOLERANCE = 1e-6;
  LEVEL = MAXlevel;
  init_grid(1 << LEVEL);
  
  rhoOcean = 1.0; rhoIce = Rhor_IceOcean; rhoAir = Rhor_AirOcean;
  muOcean = 1./sqrt(Ga); muIce = Mur_IceOcean/sqrt(Ga); muAir = Mur_AirOcean/sqrt(Ga);
  
  // acceleration due to gravity
  G.y = -1.;

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
    GIced[] = (f[]*(1.-fOcean[]) < (1e-6) ? 0.: ElasticModulus); // this is an artificial patch for now. The code has issues with VE terms in the interfacial cells!
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
    ke += sq(Delta)*(sq(u.x[]) + sq(u.y[]))*f[]; //Kinetic energy
  }
  fprintf (fp1, "%g %g %d\n", t, ke, mgp.i);
  fprintf (ferr, "%d %g %g %d\n", i, t, ke, mgp.i);
  fflush (fp1);
}