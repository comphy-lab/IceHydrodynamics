/* Title: Getting Facets
# Author: Vatsal Sanjay
# vatsalsanjay@gmail.com
# Physics of Fluids
*/
#include "navier-stokes/centered.h"
#include "fractions.h"

scalar fOcean[];
char filename[80];
int main(int a, char const *arguments[])
{
  sprintf (filename, "%s", arguments[1]);
  restore (file = filename);
  #if TREE
    fOcean.prolongation = fraction_refine;
  #endif

  FILE * fp = ferr;
  output_facets(fOcean,fp);
  fflush (fp);
  fclose (fp);
}
