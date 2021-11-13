#include <stdio.h>
#include "../inc/defines.h"
#include "../inc/read.h"
#include "../inc/struct.h"
#include "../inc/io.h"
#include "../inc/interpolation.h"


int main(int argc, char **argv)
{
    int code_error = ERR_OK;
    int table_size = 0;
    double arg = 0;
    double matrix[15] = { 0 };
    double matrix_ermite[60] = { 0 };
    table_t *table = NULL;

    read_table(&table, argv[1], &table_size);
    //print_table(table, table_size);

    read_interpol_argument(&arg);
    printf("Введенный аргумент = %lf\n", arg);

    printf("Результирующая таблица:\n"
            "|     Степень полинома     |     Полином Ньютона     |     Полином Эрмита    |\n"
            "|--------------------------|-------------------------|-----------------------|\n");
    for (int i = 0; i < 5; i++)
    {
        double res_newton = 0, res_ermite = 0;
        int index_low = 0, index_high = 0;
        get_nodes(table->x, i, table_size, arg, &index_low, &index_high);

        //интерполяция функции (ньютон)
        res_newton = newton_interpolate(table->x, table->y, matrix, table_size, i, arg,
                                        index_low, index_high);

        int index_low_ermite = 0, index_high_ermite = 0;
        int need_degree = 0;
        need_degree = (int)((i + 1) % 2 != 0 ? (i + 1) / 2 + 1 : (i + 1) / 2);
        get_nodes(table->x, need_degree - 1, table_size, arg, &index_low_ermite, &index_high_ermite);

        res_ermite = create_ermite_polynome(table, matrix_ermite, i, arg, index_low_ermite, index_high_ermite,
                                table->x[index_low_ermite + 1] - table->x[index_low_ermite]);
        printf("|%26d|%25lf|%23lf|\n", i, res_newton, res_ermite);
    }
    double res_newton = 0;
    //double res_ermite = 0;
    int index_low = 0, index_high = 0;
    get_nodes(table->x, 3, table_size, arg, &index_low, &index_high);

    //интерполяция функции (ньютон)
    res_newton = newton_interpolate(table->x, table->y, matrix, table_size, 3, arg,
                                    index_low, index_high);
    printf("res-newton = %lf\n", res_newton);
    /*//найти корень функции - обратная интерполяция
    int index_low = 0, index_high = 0;
    double matrix_new[15] = { 0 }, result = 0;
    insert_sort_array(&table->y, &table->x, table_size);
    get_nodes(table->y, 4, table_size, arg, &index_low, &index_high);
    
    result = newton_interpolate(table->y, table->x, matrix_new, table_size, 4, ROOT, index_low, index_high);
    printf("Результат обратной интерполяции = %lf\n", result);*/

    return code_error;
}
