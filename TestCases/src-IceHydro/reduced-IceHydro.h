#ifndef BASILISK_HEADER_1
#define BASILISK_HEADER_1
#line 1 "./../src-IceHydro/reduced-IceHydro.h"
/**
# Reduced gravity 

We re-express gravity in [two-phase flows](two-phase.h) as an
[interfacial force](iforce.h) as
$$
-\nabla p + \rho\mathbf{g} = 
-\nabla p' - [\rho]\mathbf{g}\cdot\mathbf{x}\mathbf{n}\delta_s
$$
with $p'= p - \rho\mathbf{g}\cdot\mathbf{x}$ the dynamic pressure and
$\rho\mathbf{g}\cdot\mathbf{x}$ the hydrostatic pressure. The corresponding 
potential is
$$
\phi = [\rho]\mathbf{G}\cdot(\mathbf{x} - \mathbf{Z})
$$
with $\mathbf{G}$ the gravity vector and $\mathbf{Z}$ an optional
reference level. */

coord G = {0.,0.,0.}, Z = {0.,0.,0.};

/**
We need the interfacial force module as well as some
functions to compute the position of the interface. */

#include "iforce.h"
#include "curvature.h"

/**
We overload the acceleration() event to add the contribution of
gravity to the interfacial potential $\phi$.

If $\phi$ is already allocated, we add the contribution of gravity,
otherwise we allocate a new field and set it to the contribution of
gravity. */
  
event acceleration (i++)
{
  scalar phi = f.phi;
  scalar phiOcean = fOcean.phi;

  coord G1, G2;
  foreach_dimension(){
    G1.x = (rhoAir - rhoIce)*G.x; // associated with ice - air interface
    G2.x = (rhoIce - rhoOcean)*G.x; // associated with ocean - ice interface
  }
  
  if (phi.i)
    position (f, phi, G1, Z, add = true);
  else {
    phi = new scalar;
    position (f, phi, G1, Z, add = false);
    f.phi = phi;
  }

  if (phiOcean.i)
    position (fOcean, phiOcean, G2, Z, add = true);
  else {
    phiOcean = new scalar;
    position (fOcean, phiOcean, G2, Z, add = false);
    fOcean.phi = phiOcean;
  }

}
#endif
