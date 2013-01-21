/**
 *  Copyright (C) 2010  Daniel Chu, YouGo Software
 *                      email: daniel.chu@yougo.com.au
 *
 *  This file is part of csv_view.
 *
 *  csv_view is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  csv_view is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with csv_view.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "csv.h"
/**
 * frees row of the csv
 */
void csv_free_row(Row* row) {
    unsigned int i;
    for (i = 0; i < row->num_items; i++) {
        free(row->row[i]);
    }
    free(row->cell_length);
    free(row->row);
    free(row);
}

/**
 * reads the FILE for the next row of the csv, note: row could be multi-lined
 */
Row* csv_get_row(FILE *file_in, char delim, unsigned int max_cell_count) {
    char in_quote = 0;
    char has_escape_quote = 0;

    char current_cell[MAX_CELL_SIZE];
    char * current_line = (char*) malloc(sizeof(char) * MAX_LINE_LENGTH);
    unsigned int cell_len;
    char *itr;
    char *cell_itr = NULL;

    Row * row = (Row *)calloc(1, sizeof(Row));
    if (row == NULL) {
        return NULL;
    }

    row->row = (char **)calloc(max_cell_count, sizeof(char*));
    row->cell_length = (unsigned int *)calloc(max_cell_count, sizeof(unsigned int));

    if (row->row == NULL) {
        free(row);
        return NULL;
    }
    row->num_items = 0;

    cell_itr = current_cell; 
    while(fgets(current_line, MAX_LINE_LENGTH, file_in) != NULL) {
        itr = current_line; 

        while (*itr != 0) {

            if (*itr == '"') {
                if (in_quote == 1) {
                    if (has_escape_quote == 0) {   
                        if (*(itr + 1) == '"' && *(itr + 2) != 0) {
                            has_escape_quote = 1;
                        }
                        else {
                            in_quote = 0;
                        }
                    }
                    else  {
                        *cell_itr = '"'; 
                        ++cell_itr;
                        has_escape_quote = 0;
                    }
                }
                else {
                    in_quote = 1;
                }
            }
            else if(*itr == delim) {
                if(in_quote) {
                    *cell_itr = *itr;
                    ++cell_itr;
                }
                else {
                    //append cell to row
                    *cell_itr = 0;
                    cell_len = cell_itr - current_cell;
                    row->row[row->num_items] = (char*) malloc((cell_len + 1) * sizeof(char));
                    if ( row->row[row->num_items] == NULL) {
                        return 0;
                    }
                    strcpy(row->row[row->num_items], current_cell);
                    row->cell_length[row->num_items] = cell_len;
                    row->num_items += 1;
                    cell_itr = current_cell;
                }
            }
            else {
                *cell_itr = *itr;
                ++cell_itr;
            }
            ++itr;
        }

        if (in_quote == 0) {
            // finish up the last column;
            if (cell_itr != 0 && cell_itr > current_cell) {

                *cell_itr = 0;
                cell_len = cell_itr - current_cell;

                if (current_cell[cell_len - 1] == '\n') {
                    current_cell[cell_len - 1] = 0;
                    cell_len -= 1;
                }

                row->row[row->num_items] = (char*)malloc((cell_len + 1) * sizeof(char));
                if ( row->row[row->num_items] == NULL) {
                    return 0;
                }

                strcpy(row->row[row->num_items], current_cell);
                row->cell_length[row->num_items] = cell_len;
                row->num_items += 1;
            }

            break;
        }
    }

    free(current_line);

    // if empty then return false
    if (row->num_items == 0) {
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


unsigned int csv_write_row(FILE *file_out, Row *row, char delim) {
    int i, j, itm;
    unsigned char has_delim = 0;
    unsigned char has_quote = 0 ;
    char new_line = '\n';
    char null = '\0';
    for (i = 0; i < row->num_items; i++) {
        has_delim = 0;
        has_quote = 0;
        itm       = 1;

        //enough memory if every single char has to be escaped
        char char_tmp_str[row->cell_length[i] * 2 + 3]; 
        for(j = 0; j < row->cell_length[i]; j++) {
            if (row->row[i][j] == delim) {
                has_delim = 1;
            }
            else if(row->row[i][j] ==  '"') {
                has_quote = 1;
                char_tmp_str[itm] = '"';
                itm += 1;
            }
            char_tmp_str[itm] = row->row[i][j]; 
            itm += 1;
        }
        if (has_quote == 1 || has_delim == 1) {
            // wrap quote
            char_tmp_str[0] = '"';
            char_tmp_str[itm] = '"';
            fwrite(char_tmp_str, sizeof(char), itm + 1, file_out);
        }
        else {
            fwrite(char_tmp_str + 1, sizeof(char), itm - 1, file_out);
        }

        if (i != row->num_items - 1) {
            fwrite(&delim, sizeof(char), 1, file_out);
        }
    }
    fwrite(&new_line, sizeof(char), 1, file_out);
    fwrite(&null, sizeof(char), 1, file_out);
    return 1;
}

