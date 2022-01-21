#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../inc/spline.h"
#include "../inc/struct.h"
#include "../inc/defines.h"
#include "../inc/table.h"

void straight_walk(spline_t **table_spline, table_t *table);
void reverse_walk(spline_t **table_spline, int count);
void calculate_add_coeffs(add_coeff_t **spline_add_coeff, spline_t *table_spline, table_t *table);

void calculate_dx(double **h, int *table_x, int count);
double find_result(add_coeff_t *spline_add_coeff, double *spline_c, table_t *table, double input_x);
int find_near_x(int *table_x, double input_x, int count);

int allocate_spline(spline_t **table_spline, int count_nodes);
int allocate_spline_add_coeff(add_coeff_t **spline_add_coeff, int count_nodes);
void free_spline_c(double **table_spline_c);
void free_spline_eta(double **table_spline_eta);
void free_spline_ksi(double **table_spline_ksi);
void free_spline_f(double **table_spline_f);



spline_t *init_spline_coeffs(void)
{
    spline_t *spline = NULL;
    spline = malloc(sizeof(spline_t));
    spline->c = NULL;
    spline->ksi = NULL;
    spline->eta = NULL;
    spline->f = NULL;
    spline->h = 0;

    return spline;
}

add_coeff_t *init_spline_add_coeffs(void)
{
    add_coeff_t *spline_add = NULL;
    spline_add = malloc(sizeof(add_coeff_t));
    spline_add->a = NULL;
    spline_add->b = NULL;
    spline_add->d = NULL;

    return spline_add;
}

int spline_table(double *result, spline_t **table_spline, add_coeff_t **spline_add_coeff, table_t *table, double input_x)
{
    int error = ERR_OK;
    error = allocate_spline(table_spline, (*table).count);
    if (error == ERR_OK)
        error = allocate_spline_add_coeff(spline_add_coeff, (*table).count);

    if (error == ERR_OK)
    {
        straight_walk(table_spline, table);
        reverse_walk(table_spline, table->count);
        calculate_add_coeffs(spline_add_coeff, *table_spline, table);
        *result = find_result(*spline_add_coeff, (*table_spline)->c, table, input_x);
    }
    return error;
}

int allocate_spline(spline_t **table_spline, int count_nodes)
{
    int error = ERR_OK;
    (*table_spline)->c = (double *)calloc(count_nodes, sizeof(double));

    if (!(*table_spline)->c)
    {
        free_spline_c(&(*table_spline)->c);
        error = ERR_ALLOCATE;
    }
    else
    {
        (*table_spline)->eta = (double *)calloc(count_nodes, sizeof(double));
        if (!(*table_spline)->eta)
        {
            free_spline_eta(&(*table_spline)->eta);
            error = ERR_ALLOCATE;
        }
        else
        {
            (*table_spline)->ksi = (double *)calloc(count_nodes, sizeof(double));
            if (!(*table_spline)->ksi)
            {
                free_spline_ksi(&(*table_spline)->ksi);
                error = ERR_ALLOCATE;
            }
        }
    }

    if (error == ERR_OK)
    {
        (*table_spline)->f = (double *)calloc(count_nodes, sizeof(double));
        if (!(*table_spline)->f)
        {
            free_spline_f(&(*table_spline)->f);
            error = ERR_ALLOCATE;
        }
    }

    if (error == ERR_OK)
    {
        (*table_spline)->h = (double *)calloc(count_nodes, sizeof(double));
        if (!(*table_spline)->h)
        {
            free_spline_f(&(*table_spline)->h);
            error = ERR_ALLOCATE;
        }
    }
    
    return error;
}

int allocate_spline_add_coeff(add_coeff_t **spline_add_coeff, int count_nodes)
{
    int error = ERR_OK;
    (*spline_add_coeff)->a = (double *)calloc(count_nodes, sizeof(double));
    if (!(*spline_add_coeff)->a)
    {
        free_spline_f(&(*spline_add_coeff)->a);
        error = ERR_ALLOCATE;
    }

    if (error == ERR_OK)
    {
        (*spline_add_coeff)->b = (double *)calloc(count_nodes, sizeof(double));
        if (!(*spline_add_coeff)->b)
        {
            free_spline_f(&(*spline_add_coeff)->b);
            error = ERR_ALLOCATE;
        }
    }

    if (error == ERR_OK)
    {
        (*spline_add_coeff)->d = (double *)calloc(count_nodes, sizeof(double));
        if (!(*spline_add_coeff)->d)
        {
            free_spline_f(&(*spline_add_coeff)->d);
            error = ERR_ALLOCATE;
        }
    }

    return error;
}

void free_table_spline(spline_t **table_spline)
{
    if (table_spline)
    {
        free_spline_c(&(*table_spline)->c);
        free_spline_eta(&(*table_spline)->eta);
        free_spline_ksi(&(*table_spline)->ksi);
        free_spline_f(&(*table_spline)->f);
        free(*table_spline);
    }
}

void free_spline_add_coeff(add_coeff_t **spline_add_coeff)
{
    if (*spline_add_coeff)
    {
        free_spline_f(&(*spline_add_coeff)->a);
        free_spline_f(&(*spline_add_coeff)->b);
        free_spline_f(&(*spline_add_coeff)->d);
        free(*spline_add_coeff);
    }
}

void free_spline_c(double **table_spline_c)
{
    if (table_spline_c)
    {
        free(*table_spline_c);
        *table_spline_c = NULL;
    }
}

void free_spline_eta(double **table_spline_eta)
{
    if (table_spline_eta)
    {
        free(*table_spline_eta);
        *table_spline_eta = NULL;
    }
}

void free_spline_ksi(double **table_spline_ksi)
{
    if (table_spline_ksi)
    {
        free(*table_spline_ksi);
        *table_spline_ksi = NULL;
    }
}

void free_spline_f(double **table_spline_f)
{
    if (table_spline_f)
    {
        free(*table_spline_f);
        *table_spline_f = NULL;
    }
}

void straight_walk(spline_t **table_spline, table_t *table)
{
    calculate_dx(&(*table_spline)->h, table->table_x, table->count);
    for (int i = 3; i < table->count; i++)
    {
        (*table_spline)->ksi[i] = -(*table_spline)->h[i - 1] / ((*table_spline)->h[i - 2] * (*table_spline)->ksi[i - 1] +
                                    2 * ((*table_spline)->h[i - 2] + (*table_spline)->h[i - 1]));
        (*table_spline)->f[i - 1] = 3 * ((table->table_y[i - 1] - table->table_y[i - 2]) / (*table_spline)->h[i - 1] -
                         (table->table_y[i - 2] - table->table_y[i - 3]) / (*table_spline)->h[i - 2]);
        (*table_spline)->eta[i] = ((*table_spline)->f[i - 1] - (*table_spline)->h[i - 2] * (*table_spline)->eta[i - 1]) /
                         ((*table_spline)->h[i - 2] * (*table_spline)->ksi[i - 1] + 2 * ((*table_spline)->h[i - 2] + (*table_spline)->h[i - 1]));
    }
}

void calculate_dx(double **h, int *table_x, int count)
{
    for (int i = 0; i < count; i++)
    {
        (*h)[i] = table_x[i + 1] - table_x[i];
    }
}

void reverse_walk(spline_t **table_spline, int count)
{
    for (int i = count - 2 ; i >= 0; i--)
    {
        (*table_spline)->c[i] = (*table_spline)->ksi[i + 1] * (*table_spline)->c[i + 1] + (*table_spline)->eta[i + 1];
    }
}

void calculate_add_coeffs(add_coeff_t **spline_add_coeff, spline_t *table_spline, table_t *table)
{
    for (int i = 1 ; i < table->count - 1; i++)
    {
        (*spline_add_coeff)->a[i] = table->table_y[i - 1];
        (*spline_add_coeff)->b[i] = (table->table_y[i] - table->table_y[i - 1]) / table_spline->h[i] -
                                    (table_spline->h[i] / 3 * (table_spline->c[i + 1] + 2 * table_spline->c[i]));
        (*spline_add_coeff)->d[i] = (table_spline->c[i + 1] - table_spline->c[i]) / (3 * table_spline->h[i]);
    }

    (*spline_add_coeff)->b[table->count - 1] = (table->table_y[table->count - 1] - table->table_y[table->count - 2] / table_spline->h[table->count - 1]) -
                                    (table_spline->h[table->count - 1] * (2 * table_spline->c[table->count - 1]) / 3);
    (*spline_add_coeff)->d[table->count - 1] = -(table_spline->c[table->count - 1]) / (3 * table_spline->h[table->count - 1]);
}

double find_result(add_coeff_t *spline_add_coeff, double *spline_c, table_t *table, double input_x)
{
    double result = 0;
    int i_near = find_near_x(table->table_x, input_x, table->count);
    result = spline_add_coeff->a[i_near] + spline_add_coeff->b[i_near] * (input_x - table->table_x[i_near - 1])
                        + spline_c[i_near] * pow((input_x - table->table_x[i_near - 1]), 2) 
                        + spline_add_coeff->d[i_near] * pow((input_x - table->table_x[i_near - 1]), 3);
    return result;
}

int find_near_x(int *table_x, double input_x, int count)
{
    int i_near = 0;
    double min = fabs(table_x[0] - input_x);
    double delta = 0; 
    for (int i = 1; i < count; i++)
    {   
        delta = fabs(table_x[i] - input_x);
        if (delta < min)
        {
            i_near = i;
            min = delta;
        }
    }

    return i_near;
}