#include <stdio.h>
#include "../inc/defines.h"
#include "../inc/struct.h"
#include "../inc/table.h"
#include "../inc/spline.h"

int main(void)
{
    int error = ERR_OK;
    double input_x = 0;
    
    table_t *table;
    spline_t *table_spline;
    add_coeff_t *spline_add_coeff;

    table = init_table();

    error = read_table(&table);

    if (error == ERR_OK){
    	print_table(table);
        read_input_x(&input_x);
    }

    if (error == ERR_OK)
    {
        double result = 0;
        table_spline = init_spline_coeffs();
        spline_add_coeff = init_spline_add_coeffs();
        error = spline_table(&result, &table_spline, &spline_add_coeff, table, input_x);
        
        if (error == ERR_OK)
            printf("Резульат = %lf\n", result);
        free_table_spline(&table_spline);
    }
    return error;
}
