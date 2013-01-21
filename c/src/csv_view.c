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
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <ncurses.h>
#include "csv.h"
#define FILE_LEN 255
#define BLOCK_LINE_COUNT 100000
#define DEFAULT_BLOCK_ARRAY_SIZE 1000
#define MIN_CELL_WIDTH 10
#define MAX_CELL_WIDTH 500
#define MAX_CELL_HEIGHT 10 
#define BORDER_SIZE 1
#define MAX_SUPPORTED_CELLS 5000

// A block which can be directly rendered to screen via ncurses
typedef struct Block {
    // each line is an array of characters arrays where each character array is a column
    // each block is an array of lines
    Row *lines[BLOCK_LINE_COUNT];
    unsigned int height_map[BLOCK_LINE_COUNT]; //from beginning of the file in terms of row lines...
    unsigned int lines_used;
} Block;

Block* block_new() {
    Block * blk = malloc(sizeof(Block));
    if (blk != NULL) {
        blk->lines_used = 0;
    }
    return blk;
}

inline int block_isfull(Block* blk) {
    return (blk->lines_used < BLOCK_LINE_COUNT);
}

void block_insert_row(Row* row, Block* blk, unsigned int *max_cell_list) {
    int i;
    blk->lines[blk->lines_used] = row;
    blk->lines_used++;

    for(i = 0; i < row->num_items; ++i) {
        max_cell_list[i] = row->cell_length[i] > max_cell_list[i] ?  row->cell_length[i] :  max_cell_list[i];
    }
}

int get_params(int argc, char** argv, char* file_name, char *delim, int *use_header, unsigned int *max_cell_width, unsigned int *max_cell_height, unsigned int *min_cell_width) {
   int c;
   file_name[0] = '\0';
   *delim = ',';
   *use_header = 0;
     
   opterr = 0;

    while ((c = getopt(argc, argv, "hd:m:n:b:")) != -1) {
        switch(c) {
            case 'd':
                *delim = optarg[0];
                break;
            case 'm':
                fprintf(stdout,"%s\n",  optarg);
                *max_cell_width = atoi(optarg);
                break;
            case 'n':
                *min_cell_width = atoi(optarg);
                break;
            case 'b':
                *max_cell_height = atoi(optarg);
                break;
            case 'h':
                *use_header = 1;
                break;
            case '?':
                if (optopt == 'd') {
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (isprint (optopt)) {
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                }
                else {
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                return 0;
            default:
                return 0;
        }

    }
    
    if (optind + 1 == argc) {
        strncpy(file_name, argv[optind], FILE_LEN);
        return 1;
    }
    else if (optind == argc) {
        return 1;
    }
    else {
        fprintf (stderr, "Too many arguments\n");
        return 0;
    }
}

//1,2,4,8 bitfield
// +-1-+
// |   |
// 8   2
// |   |
// +-4-+
// draws a border - "which_one" indicate which border is to be drawn"

void render_border(
    WINDOW* window,
    int startx,
    int starty,
    int endx,
    int endy,
    char which_one,
    char horiz,
    char vert,
    char corner
) {
    int i;

    //paint the corners
    if ((which_one & (1 + 8)) != 0) {
        mvwaddch(window,starty, startx, corner);
    }

    if ((which_one & (1 + 2)) != 0) {
        mvwaddch(window,starty, endx, corner);
    }

    if ((which_one & (2 + 4)) != 0) {
        mvwaddch(window,endy, endx, corner);
    }

    if ((which_one & (8 + 4)) != 0) {
        mvwaddch(window,endy, startx, corner);
    }

    //paint the lines
    // top
    if ((which_one & 1) == 1) {
        for (i = startx + 1; i < endx; i++) {
            //paint the x
            mvwaddch(window,starty, i, horiz);
        }
    }

    // right
    if (((which_one >> 1) & 1) == 1) {
        for (i = starty + 1; i < endy; i++) {
            //paint the x
            mvwaddch(window,i, endx, vert);
        }

    }

    //bottom
    if (((which_one >> 2) & 1) == 1) {
        for (i = startx + 1; i < endx; i++) {
            //paint the x
            mvwaddch(window,endy, i, horiz);
        }
    }

    // left
    if (((which_one >> 3) & 1) == 1) {
        for (i = starty + 1; i < endy; i++) {
            //paint the x
            mvwaddch(window,i, startx, vert);
        }
    }
}

/* Function used to render a single cell to a block */
unsigned int render_cell(
    WINDOW* window,
    int x,
    int y,
    int max_x,
    int max_y,
    Row* row,
    unsigned int cell_offset,
    unsigned int cell_width,
    unsigned int max_cell_height,
    int alignment
) {
    int j;
    int cur_char = 0;
    int cur_row = 0;
    int os = 0;

    if (alignment == 1) {
        //center   
        os = (cell_width - row->cell_length[cell_offset]) / 2;
    }
    else if (alignment == 2) {
        os = (cell_width - row->cell_length[cell_offset]);
    }

    for (j = 0; j <= row->cell_length[cell_offset]; j++) {
        // for each char in cell
        // if cell is not new line and char is within the width of the cell
        if (row->row[cell_offset][j] == '\n' || j - cur_char == cell_width || j == row->cell_length[cell_offset] || row->row[cell_offset][j] == '\0') {
            //
            int x_coord_offset = x;
            int str_start_pos = cur_char;
            int str_len = j - cur_char;

            if (x_coord_offset < 0) {
                str_start_pos += abs(x_coord_offset);
                str_len  = j - cur_char;
                x_coord_offset = 0;
            }

            if (x_coord_offset + str_len >= max_x) {
                str_len = max_x - x_coord_offset;
            }

            //fprintf(stderr, "xxxxx\n\tstr_start_pos =%d, len = %d str = %s\n", str_start_pos, str_len, row->row[cell_offset] + str_start_pos);
            if (str_len > 0 && str_start_pos < row->cell_length[cell_offset]) {
                mvwaddnstr(window, y + cur_row, x_coord_offset + os, row->row[cell_offset] + str_start_pos, str_len);
            }

            cur_char = j + 1;

            //iterate to next valid code character (utf8 handling)
            while (
                cur_char < row->cell_length[cell_offset] && (unsigned char)row->row[cell_offset][cur_char] > 127 && (unsigned char)row->row[cell_offset][cur_char] < 192
            ) {
                cur_char -= 1; 
            } 

            cur_row += 1;

            //fprintf(stderr, "zzzzz \t ROW = %d\n", cur_row);
            if (cur_row == max_cell_height || cur_row == max_y) {
                break;
            }
        }
    }

    //add border
    return cur_row < max_cell_height ? cur_row : max_cell_height;
}

/**
 * renders a single row
 * returns how many lines are used to render the row (as it could be multilined)
 */
unsigned int render_row(
    WINDOW* window,
    unsigned int start_x,           //x offset from the screen
    unsigned int start_y,           //y offset from the screen
    unsigned int window_max_x,      //max window x
    unsigned int window_max_y,      //max window y
    Row* row, 
    unsigned int* max_cell_list, 
    unsigned int max_cell_height,   //system max cell width
    unsigned int max_cell_width,    //system max cell height
    unsigned int min_cell_width,    //system min cell height
    int alignment,                   //alignment, 0 left, 1 center, 2 right
    int is_header
) {
    //iterate through calculated height of each cell and see if it's below the max_height
    //space padd the alignment if cell is not multilined
    unsigned int i;

    unsigned int pos_from_start = BORDER_SIZE; //offset from beginning of the line including borders

    unsigned int rows_used = 1; //offset from start_y returned to caller

    for (i = 0; i < row->num_items; ++i) {
        // for each cell:
        // calculate the cell width
        // if the cell is out the screen then stop
        unsigned int cell_width = max_cell_list[i] < max_cell_width ? max_cell_list[i] : max_cell_width;
        unsigned int rows_for_cell = 0;

        if(cell_width < min_cell_width) {
            cell_width = min_cell_width;
        }

        // if cell is shifted out side the page then skip cell
        if (pos_from_start - start_x + cell_width < 0) {
            pos_from_start += cell_width + BORDER_SIZE;
            continue;
        }

        // if current cell is out of range, then cancel
        if ( (int)pos_from_start - (int) start_x >=  (int)window_max_x) {
            break;
        }

        rows_for_cell = 
            render_cell(
                window,
                pos_from_start - start_x,
                start_y,
                window_max_x,
                window_max_y,
                row,
                i,
                cell_width,
                max_cell_height,
                alignment
            );

        if (rows_for_cell > rows_used) {
            rows_used = rows_for_cell;
        }

        // increment position
        pos_from_start += cell_width + BORDER_SIZE;
    }

    // render border
    pos_from_start = BORDER_SIZE;
    for (i = 0; i < row->num_items; i++) {
        unsigned int cell_width = max_cell_list[i] < max_cell_width ? max_cell_list[i] : max_cell_width;
        char horiz = '-';
        char vert = '|';
        char corner = '+';
        int size = 2;

        if(cell_width < min_cell_width) {
            cell_width = min_cell_width;
        }

        if (pos_from_start - start_x + cell_width < 0) {
            pos_from_start += cell_width + BORDER_SIZE;
            continue;
        }

        if (i == 0) {
            size = 10;
        }

        if (is_header) {
            horiz = '=';
        }

        render_border(window, pos_from_start - start_x - BORDER_SIZE, start_y, pos_from_start - start_x + cell_width, start_y + rows_used, size, horiz, vert, vert);
            render_border(window, pos_from_start - start_x - BORDER_SIZE, start_y, pos_from_start - start_x + cell_width, start_y + rows_used, 4, horiz, vert, corner);

        pos_from_start += cell_width + BORDER_SIZE;
    }
    //returns border
    return rows_used + 1;
}

typedef struct PageState {
    unsigned int first_row;      //which is the first row
    unsigned int first_row_height; //height of the first row
    unsigned int first_row_offset; //offset currently being displayed

    unsigned int last_row;      //which is the first row
    unsigned int last_row_height; //height of the first row
    unsigned int last_row_offset; //offset currently being displayed
} PageState;

void render_screen(
    int has_header, 
    int alignment, 
    int start_x, 
    int start_y, 
    Block **data, 
    unsigned int max_block, 
    unsigned int* col_list,
    unsigned int min_cell_width,
    unsigned int max_cell_width,
    unsigned int max_cell_height
   // PageState *page_state
) {
    int row,col;
    int row_offset = 0;
    unsigned int blk_cnt = 0;
    unsigned int row_cnt = 0;

    getmaxyx(stdscr, row, col);

    clear();

    // render header
    if (has_header == 1) {
        Row * header = data[0]->lines[0];
        row_offset += render_row(stdscr, start_x, 0, col, row, header, col_list, max_cell_height, max_cell_width, min_cell_width, alignment, has_header);
        if (start_y == 0) {
            start_y += 1;
        }
    }

    // locate start row
    blk_cnt = start_y / BLOCK_LINE_COUNT; 
    row_cnt = start_y % BLOCK_LINE_COUNT;
   
    while (row_offset < col && blk_cnt < max_block) {
        Row * cur = NULL;
        cur = data[blk_cnt]->lines[row_cnt]; 
        row_offset += render_row(stdscr, start_x, row_offset, col, row, cur, col_list, max_cell_height, max_cell_width, min_cell_width, alignment, 0);

        row_cnt += 1;
        if (row_cnt == data[blk_cnt]->lines_used) {
            blk_cnt += 1;
            row_cnt = 0;
        }
    }

    // update page state

    //update window
    refresh();
}

unsigned int get_num_from_cmdline(char cur_char, FILE* tty) {
    unsigned int i = cur_char - 48;
    while((cur_char = fgetc(tty)) != KEY_ENTER) {
        if (cur_char == KEY_ENTER) {
            return i;
        }
        else if(cur_char >=48 && cur_char <= 58) {
            i = i * 10 + cur_char - 48;
        }
        else {
            return 0;
        }
    }
    return i;
}

int main(int argc, char** argv) {
    char file_name[FILE_LEN + 1];

    unsigned int i;
    unsigned int max_block_array_size = DEFAULT_BLOCK_ARRAY_SIZE;
    unsigned int cur_block_array_size = 0;

    FILE* fname = NULL;
    int alignment = 0;
    int use_header = 0;
    char delim = ',';

    Block **block_array;
    Row *current_row = NULL;

    unsigned int max_cell_count = MAX_SUPPORTED_CELLS ;
    unsigned int *max_cell_size = (unsigned int*) calloc(max_cell_count, sizeof(unsigned int));
    unsigned int max_cell_height = MAX_CELL_HEIGHT;
    unsigned int max_cell_width = MAX_CELL_WIDTH;
    unsigned int min_cell_width = MIN_CELL_WIDTH;
    int ch;
    //WINDOW *table_window;
    int row = 0;
    int col = 0;
    int maxx;
    int maxy;
    int total_row_count = 0;
    int first_line = 0;
    int total_cell_count = 0;
    int end_of_line_offset = 0;
    FILE *tty;

    setlocale(LC_ALL, "");
    if (!get_params(argc, argv, file_name, &delim, &use_header, &max_cell_width, &max_cell_height, &min_cell_width)) {
        printf("usage: %s <options> filename\n\t<options>\n\t-d <delim> uses <delim> character as delimter (default ,)\n\t-h use first line as header\n\t-m max cell width\n\t-n min cell width\n\t-b max_cell height\n", argv[0]);
        return 1;
    }

    block_array = (Block**) calloc(max_block_array_size, sizeof(Block*));
    if (block_array == NULL) {
        return false;
    }

    if (strcmp(file_name, "") == 0) {
        printf("reading from stdin...\n");
        fname = stdin;
    }
    else {
        printf("reading from file %s...\n", file_name);
        fname = fopen(file_name, "r");
        if (fname == NULL) {
            free (block_array);
            fprintf(stdout, "error opening file %s\n", file_name);
        }
    }

    while (1) {
        // get next row of csv
        current_row = csv_get_row(fname, delim, MAX_SUPPORTED_CELLS);
        if (current_row == NULL || (current_row->num_items == 0 && feof(fname))) {
            break;
        }

        if (cur_block_array_size == 0 || block_array[cur_block_array_size - 1]->lines_used == BLOCK_LINE_COUNT) {
            // if we need a new block
            if (cur_block_array_size == max_block_array_size) {
                //if we need to realloc the array of block pointers because its full
                block_array = (Block**) realloc(block_array, sizeof(Block*) * max_block_array_size * 2);
                if (block_array == NULL) {
                    return -1;
                }
                max_block_array_size *= 2;
            }

            block_array[cur_block_array_size] = block_new();

            if (block_array[cur_block_array_size] == NULL) {
                return -1;
            }
            cur_block_array_size++;
        }
        block_insert_row(current_row, block_array[cur_block_array_size - 1], max_cell_size);

        total_cell_count = total_cell_count < current_row->num_items ? current_row->num_items : total_cell_count;
    }
   
    // using tty instead of stdin as stdin could be used as data pipe
    tty = fopen("/dev/tty", "r");
    if (tty < 0) {
        tty = (FILE*) 2;
    }

    //stdin = tty;
    if (cur_block_array_size == 0) {
        fprintf(stdout, "No data found, exiting...\n");    
        exit(0);
    }

    total_row_count = (cur_block_array_size - 1) * BLOCK_LINE_COUNT + block_array[cur_block_array_size - 1]->lines_used;

    initscr();
    curs_set(0);
    clear();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, maxy, maxx);
    //int render_screen(int has_header, int alignment, int start_x, int start_y, Block **data)
    if (use_header) {
        first_line = 1;
        col = 1;
    }

    render_screen(use_header, alignment, row, col, block_array, cur_block_array_size, max_cell_size, min_cell_width, max_cell_width, max_cell_height);

    for (i = 0; i < total_cell_count; i++) {
        end_of_line_offset += max_cell_size[i] < max_cell_width ? max_cell_size[i] : max_cell_width;
    }

    //table_window = newwin(row, col, 0, 0);

    // set up header


    //while((ch = getch()) != 'q')
    while((ch = fgetc(tty)) != 'q') {   
        getmaxyx(stdscr, maxy, maxx);
        int is_real = 1;
        switch(ch) {   
            case 'h':
            case KEY_LEFT:
                row = row > 1 ? row - 1 : 0;
                break;
            case 'l':
            case KEY_RIGHT:
                row += 1;
                break;
            case 'k':
            case KEY_UP:
                col = col > 1 ? col - 1 : first_line;
                break;
            case 'j':
            case KEY_DOWN:
                col = col + 1 >=  total_row_count ? col : col + 1;
                break;  
            case 'J':
            case KEY_NPAGE:
                //page down
                col = col + maxy >= total_row_count - 1? col: col + maxy;
                break;
            case 'K':
            case KEY_PPAGE:
                col = col - maxy > first_line ? col - maxy : first_line;
                //page up
                break;
            case 'g':
            case KEY_HOME:
                col = 0;
                //beginning of file
                break;
            case 'G':
            case KEY_END:
                //end of file
                col = total_row_count - 1;
                break;
            case '$':
                // need to take into account of borders
                row = (end_of_line_offset) + total_cell_count;
                break;
            case '^':
                // beginning of row
                row = 0;
                break;
            case 'a':
                // change alignment
                alignment = alignment == 2 ? 0 : alignment + 1;
                break;
            case 'H':
                use_header = (use_header ^ 1);
                first_line = (first_line ^ 1);
                if (use_header == 1 && col == 0) {
                    col = 1;
                }
                else if (use_header == 0 && col == 1) {
                    col = 0;
                }


                // display header
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                int target = get_num_from_cmdline(ch, tty);
                if (target > 0 && target <= total_row_count) {
                    col = target - 1;
                }
            }
                break;
            /*
            case '/':
                // search
                while((ch = getch()) != 'q')
                break;
            */
            default:
                is_real = 0;
                break;
        }
        if (is_real) {
            render_screen(use_header, alignment, row, col, block_array, cur_block_array_size, max_cell_size, min_cell_width, max_cell_width, max_cell_height);
        }
        mvaddch(0,0,ch);
    }

    clear();
    endwin();

    // freeing each block
    for (i = 0; i < cur_block_array_size; i++) {
        unsigned int j = 0;
        for (j = 0; j < block_array[i]->lines_used; j++) {

            char** tmp = block_array[i]->lines[j]->row;
            unsigned int k;
            for (k = 0; k < block_array[i]->lines[j]->num_items; k++) {
                free(tmp[k]);
            }
            free(block_array[i]->lines[j]->row);
            free(block_array[i]->lines[j]->cell_length);
        }
        free(block_array[i]->lines);
    }

    return 0;
}
