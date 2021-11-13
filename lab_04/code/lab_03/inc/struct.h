#ifndef STRUCT_H
#define STRUCT_H

typedef struct table table_t;
struct table
{
    int *table_x;
    int *table_y;
    int count; 
};

typedef struct spline spline_t;
struct spline
{
    double *c;
    double *ksi;
    double *eta;
    double *f;
    double *h;
};

typedef struct add_coeff add_coeff_t;
struct add_coeff
{
    double *a;
    double *b;
    double *d;
};

#endif