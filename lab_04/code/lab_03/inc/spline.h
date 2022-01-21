#ifndef SPLINE_H
#define SPLINE_H

#include "../inc/struct.h"

spline_t *init_spline_coeffs(void);
add_coeff_t *init_spline_add_coeffs(void);

int spline_table(double *result, spline_t **table_spline, add_coeff_t **spline_add_coeff, table_t *table, double input_x);
void free_table_spline(spline_t **table_spline);
void free_spline_add_coeff(add_coeff_t **spline_add_coeff);

#endif