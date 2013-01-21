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

#ifndef CSV_H
#define CSV_H
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <locale.h>

#ifndef MAX_LINE_LENGTH
    #ifdef WINDOWS
        #define MAX_LINE_LENGTH 10240
    #else
        #define MAX_LINE_LENGTH 1000000
    #endif
#endif

#ifndef MAX_CELL_SIZE
    #ifdef WINDOWS
        #define MAX_CELL_SIZE 10240
    #else
        #define MAX_CELL_SIZE 5000000
    #endif
#endif

typedef struct Row {
    // array of char* strings where each char* is a cell
    char **row;
    // array of int where each int is length of a cell, used to avoid overuse of strlen
    unsigned int* cell_length;

    // number of cells
    unsigned int num_items;
}Row;

/**
 * designed for experimental wchar_t support
 * currently not used
 */
typedef struct WideRow {
    // array of char* strings where each char* is a cell
    wchar_t **row;
    // array of int where each int is length of a cell, used to avoid overuse of strlen
    unsigned int* cell_length;

    // number of cells
    unsigned int num_items;
}WideRow;

/*
typedef struct CSVReader {
    FILE * file_in
}CSVReader

typedef struct CSVWriter {
    FILE * file_out
}CSVWriter
*/

/**
 * gets a row from file_in
 * assumes that the current pointer is pointed at beginning of a row
 * when function finished, file ptr will be pointing at beginning of next row
 *
 * returns NULL if data is unavailable 
 */
Row* csv_get_row(FILE *file_in, char delim, unsigned int max_cell_count);

void csv_free_row(Row* row);

/**
 * write a csv to given file pointer
 */
unsigned int csv_write_row(FILE *file_out, Row *row, char delim);
#endif
