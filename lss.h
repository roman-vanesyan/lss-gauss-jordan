#ifndef LSS_GUARD_H
#define LSS_GUARD_H

#define EPS 1e-9

/* Flag is used to indicate if error mode is enabled. */
int error_mode;

/* Flag is used to indicate if debug mode is enabled. */
int debug_mode;

int lss(int n, double *A, double *B, double *X, double *tmp);

size_t lss_memsize(int n);

#endif
