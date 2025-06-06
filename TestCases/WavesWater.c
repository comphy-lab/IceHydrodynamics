#include "navier-stokes/centered.h"
//#define FILTERED
#include "two-phase.h"
#include "reduced.h"


#define fErr (1e-3)
#define VelErr (1e-2)
#define OmegaErr (1e-3)
#define KAPPAErr (1e-4)

char nameOut[80], amplitudeFile[80], name[80];
static FILE * fp1 = NULL;
static FILE * fp2 = NULL;


//Boundary conditions
/*uf.n[left]   = 0.;
uf.n[right]  = 0.;
uf.n[top]    = 0.;
uf.n[bottom] = 0.;*/
u.n[top] = neumann(0.);
p[top] = dirichlet(0.);


double Rhor_AirOcean, Mur_AirOcean, Ga, A0;
double tmax, tsnap, step;
int MAXlevel;
#define MINlevel 0
#define tsnap (0.01)


/**
The initial condition is a small amplitude plane wave of wavelength ($\lambda$)
unity. This wavelength is the relevant length scale for this problem. Note that $k = 2\pi$ */
event init (t = 0) {
  fraction (f, -y + A0*cos (2.*pi*x));
}

int main(int argc, char const *argv[]) {
  if (argc < 7){
    fprintf(ferr, "Lack of command line arguments. Check! Need %d more arguments\n",7-argc);
    return 1;
  }

  // density and viscosity ratios
  Rhor_AirOcean = atof(argv[1]); // this is the density ratio of air to that of the ocean...
  Mur_AirOcean = atof(argv[2]); //OV:1e-2; this is the viscosity ratio of air to that of the ocean...
  Ga = atof(argv[3]); // gL^3/(\mu/\rho)^2 -- this is based on the viscosity of ocean
  A0 = atof(argv[4]); // initial amplitude of the ocean wave
  tmax = atof(argv[5]);
  step = atof(argv[6]);
  MAXlevel = atoi(argv[7]);

  DT = step;
  L0 = 2.0;
  Y0 = -L0/2.;
  init_grid(1 << MAXlevel);

  rho1 = 1.0; rho2 = Rhor_AirOcean; 
  mu1 = 1./sqrt(Ga); mu2 = Mur_AirOcean/sqrt(Ga);
  G.y = -1.;

  char comm[80];
  sprintf (comm, "mkdir -p intermediate");
  system(comm);

  fp1 = fopen ("logfile.dat", "w");
  fp2 = fopen ("amplitude.dat", "w");

  run();

  fclose (fp1);
  fclose (fp2);
}

scalar omega[], KAPPA[];
event adapt (i++) {
  curvature(f, KAPPA);
  vorticity (u, omega);
  adapt_wavelet ((scalar *){f, u.x, u.y, omega, KAPPA},
     (double[]){fErr, VelErr, VelErr, OmegaErr, KAPPAErr},
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

  scalar pos[];
  position (f, pos, {0,1});
  double max = statsf(pos).max;

  /**
  We output the corresponding evolution in a file indexed with the
  case number. */
  if (i == 0) {
    fprintf (fp2, "t AmpOcean\n");
  }
  fprintf (fp2, "%g %g\n", t, max);
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
