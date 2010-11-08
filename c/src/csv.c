#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "csv.h"
/**
 * frees row of the csv
 */
void csv_free_row(Row* row)
{
    unsigned int i;
    for (i = 0; i < row->num_items; i++)
    {
        free(row->row[i]);
    }
    free(row->row);
    free(row);
}

void csv_free_wide_row(WideRow* row)
{
    csv_free_row((Row*)row);
}

/**
 * reads the FILE for the next row of the csv, note: row could be multi-lined
 */
Row* csv_get_row(FILE *file_in, char delim, unsigned int max_cell_count)
{
    char in_quote = 0;
    char has_escape_quote = 0;

    char current_cell[MAX_CELL_SIZE];
    char * current_line = (char*) malloc(sizeof(char) * 1000000);
    unsigned int cell_len;
    char *itr;
    char *cell_itr = NULL;

    Row * row = (Row *)calloc(1, sizeof(Row));
    if (row == NULL)
    {
        return NULL;
    }

    row->row = (char **)calloc(max_cell_count, sizeof(char*));
    row->cell_length = (unsigned int *)calloc(max_cell_count, sizeof(unsigned int));

    if (row->row == NULL)
    {
        free(row);
        return NULL;
    }
    row->num_items = 0;

    cell_itr = current_cell; 
    while(fgets(current_line, 1000000, file_in) != NULL)
    {
        itr = current_line; 

        while (*itr != 0)
        {

            if (*itr == '"')
            {
                if (in_quote == 1)
                {
                    if (has_escape_quote == 0)
                    {   
                        if (*(itr + 1) == '"' && *(itr + 2) != 0)
                        {
                            has_escape_quote = 1;
                        }
                        else
                        {
                            in_quote = 0;
                        }
                    }
                    else 
                    {
                        *cell_itr = '"'; 
                        ++cell_itr;
                        has_escape_quote = 0;
                    }
                }
                else
                {
                    in_quote = 1;
                }
            }
            else if(*itr == delim)
            {
                if(in_quote)
                {
                    *cell_itr = *itr;
                    ++cell_itr;
                }
                else
                {
                    //append cell to row
                    *cell_itr = 0;
                    cell_len = cell_itr - current_cell;
                    row->row[row->num_items] = (char*) malloc((cell_len + 1) * sizeof(char));
                    if ( row->row[row->num_items] == NULL)
                    {
                        return 0;
                    }
                    strcpy(row->row[row->num_items], current_cell);
                    row->cell_length[row->num_items] = cell_len;
                    row->num_items += 1;
                    cell_itr = current_cell;
                }
            }
            else
            {
                *cell_itr = *itr;
                ++cell_itr;
            }
            ++itr;
        }

        if (in_quote == 0)
        {
            // finish up the last column;
            if (cell_itr != 0 && cell_itr > current_cell)
            {

                *cell_itr = 0;
                cell_len = cell_itr - current_cell;

                if (current_cell[cell_len - 1] == '\n')
                {
                    current_cell[cell_len - 1] = 0;
                    cell_len -= 1;
                }

                row->row[row->num_items] = (char*)malloc((cell_len + 1) * sizeof(char));
                if ( row->row[row->num_items] == NULL)
                {
                    return 0;
                }

                strcpy(row->row[row->num_items], current_cell);
                row->cell_length[row->num_items] = cell_len;
                row->num_items += 1;
            }

            break;
        }
    }

    // if empty then return false
    if (row->num_items == 0)
    {
        free(row->row);
        free(row->cell_length);
        free(row);
        return NULL;
    }

    //trim row
    row->row = (char**)realloc(row->row, sizeof(char*) * row->num_items);
    row->cell_length = (unsigned int*) realloc(row->cell_length, sizeof(unsigned int) * row->num_items);
    return row;
}

