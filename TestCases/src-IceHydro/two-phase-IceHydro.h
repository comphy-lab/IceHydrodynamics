#ifndef BASILISK_HEADER_24
#define BASILISK_HEADER_24
#line 1 "./../src-IceHydro/two-phase-IceHydro.h"
#include "vof.h"

scalar f[], fOcean[], * interfaces = {f, fOcean}; // f = 1 is ice+ocean and fOcean = 1 in ocean only

double rhoOcean = 1., rhoIce = 1., rhoAir = 1., muOcean = 0., muIce = 0., muAir = 0.;

/**
Auxilliary fields are necessary to define the (variable) specific
volume $\alpha=1/\rho$ as well as the cell-centered density. */

face vector alphav[];
scalar rhov[];

event defaults (i = 0)
{
  alpha = alphav;
  rho = rhov;

  /**
  If the viscosity is non-zero, we need to allocate the face-centered
  viscosity field. */
  
  mu = new face vector;

  /**
  We add the interface to the default display. */

  display ("draw_vof (c = 'f');");
}

/**
The density and viscosity are defined using arithmetic averages by
default. The user can overload these definitions to use other types of
averages (i.e. harmonic). */

#ifndef rho
# define rho(f1, f2) (f1*f2*rhoOcean + f1*(1.-f2)*rhoIce + (1.-f1)*rhoAir)
#endif
#ifndef mu
# define mu(f1, f2)  (f1*f2*muOcean + f1*(1.-f2)*muIce + (1.-f1)*muAir)
#endif

/**
We have the option of using some "smearing" of the density/viscosity
jump. */

#ifdef FILTERED
scalar sf[];
scalar sfOcean[];
#else
# define sf f
# define sfOcean fOcean
#endif

event tracer_advection (i++)
{
  
  /**
  When using smearing of the density jump, we initialise *sf* with the
  vertex-average of *f*. */

#ifndef sf
#if dimension <= 2
  foreach()
    sf[] = (4.*f[] + 
	    2.*(f[0,1] + f[0,-1] + f[1,0] + f[-1,0]) +
	    f[-1,-1] + f[1,-1] + f[1,1] + f[-1,1])/16.;
#else // dimension == 3
  foreach()
    sf[] = (8.*f[] +
	    4.*(f[-1] + f[1] + f[0,1] + f[0,-1] + f[0,0,1] + f[0,0,-1]) +
	    2.*(f[-1,1] + f[-1,0,1] + f[-1,0,-1] + f[-1,-1] + 
		f[0,1,1] + f[0,1,-1] + f[0,-1,1] + f[0,-1,-1] +
		f[1,1] + f[1,0,1] + f[1,-1] + f[1,0,-1]) +
	    f[1,-1,1] + f[-1,1,1] + f[-1,1,-1] + f[1,1,1] +
	    f[1,1,-1] + f[-1,-1,-1] + f[1,-1,-1] + f[-1,-1,1])/64.;
#endif
#endif // !sf

#ifndef sfOcean
#if dimension <= 2
  foreach()
    sfOcean[] = (4.*fOcean[] + 
	    2.*(fOcean[0,1] + fOcean[0,-1] + fOcean[1,0] + fOcean[-1,0]) +
	    fOcean[-1,-1] + fOcean[1,-1] + fOcean[1,1] + fOcean[-1,1])/16.;
#else // dimension == 3
  foreach()
    sfOcean[] = (8.*fOcean[] +
	    4.*(fOcean[-1] + fOcean[1] + fOcean[0,1] + fOcean[0,-1] + fOcean[0,0,1] + fOcean[0,0,-1]) +
	    2.*(fOcean[-1,1] + fOcean[-1,0,1] + fOcean[-1,0,-1] + fOcean[-1,-1] + 
		fOcean[0,1,1] + fOcean[0,1,-1] + fOcean[0,-1,1] + fOcean[0,-1,-1] +
		fOcean[1,1] + fOcean[1,0,1] + fOcean[1,-1] + fOcean[1,0,-1]) +
	    fOcean[1,-1,1] + fOcean[-1,1,1] + fOcean[-1,1,-1] + fOcean[1,1,1] +
	    fOcean[1,1,-1] + fOcean[-1,-1,-1] + fOcean[1,-1,-1] + fOcean[-1,-1,1])/64.;
#endif
#endif // !sfOcean

#if TREE
  sf.prolongation = refine_bilinear;
  sf.dirty = true; // boundary conditions need to be updated
  sfOcean.prolongation = refine_bilinear;
  sfOcean.dirty = true; // boundary conditions need to be updated
#endif
}

#include "fractions.h"

event properties (i++)
{
  foreach_face() {

    double ff1 = (sf[] + sf[-1])/2.;
    double ff2 = (sfOcean[] + sfOcean[-1])/2.;
    
    ff1 = clamp(ff1, 0., 1.);
    ff2 = clamp(ff2, 0., 1.);

    alphav.x[] = fm.x[]/rho(ff1, ff2);

    face vector muv = mu;
    muv.x[] = fm.x[]*mu(ff1, ff2);
  
  }
  
  foreach()
    rhov[] = cm[]*rho(sf[], sfOcean[]);

#if TREE
  sf.prolongation = fraction_refine;
  sf.dirty = true; // boundary conditions need to be updated
  sfOcean.prolongation = fraction_refine;
  sfOcean.dirty = true; // boundary conditions need to be updated
#endif
}

#endif
