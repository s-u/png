/* dummy symbols to keep superfluous CRAN checks happy
   (this package uses NAMESPACE C-level symbol registration
   but the checks don't get that) */

#include <stdlib.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>

void dummy(void) {
    R_registerRoutines(NULL, NULL, NULL, NULL, NULL);
    R_useDynamicSymbols(NULL, TRUE);
}
