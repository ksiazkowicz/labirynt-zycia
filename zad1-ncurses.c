#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <locale.h>

#define mapX               120
#define mapY                63
#define L_UP                 1
#define L_LEFT               2
#define L_RIGHT              4
#define L_DOWN               3

#define DRAW_DEBUG           0
#define HARDCORE_MODE        1

#define E_HORIZONTAL_TUNNEL  0
#define E_VERTICAL_TUNNEL    1
#define E_LEFT_DOWN          2
#define E_RIGHT_DOWN         3
#define E_DOWN_LEFT          4
#define E_DOWN_RIGHT         5
#define E_FILLED             6
#define E_ZEROS              7

#define A_SAVE               0
#define A_LOAD               1

#define G_CONTINUE           0
#define G_SAVE               2
#define G_QUIT               3
#define G_RESET              4
#define G_LOAD               5

void title_screen() {
    move(0, 0);
    printw("\n");
    printw("        ███                       ███         ███                                            ██\n");
    printw("        ███                       ███         ███                                          ██████\n");
    printw("       ███                       ███                  ████      ██    ██     ██  ██ █████    ██\n");
    printw("       ███          ██████       ███████      ███    ██  ██  ████     ██     ██  ████   ██   ██\n");
    printw("      ███         ██     ██     ███    ███   ███          █████        ██   ██   ███    ██   ██\n");
    printw("      ███         ██     ███    ██      ██   ███           ███         ██   ██   ██     ██   ██    ██\n");
    printw("     ██████████    ███████ ██  ██████████   ███            ███          █████    ██     ██    ██████\n");
    printw("                                           ███                             ██\n");
    printw("                                          ███                              ██                       ███\n");
    printw("                                         █████████████████████████████████████████████████████████████\n");
    printw("                                                                       ██  ██\n");
    printw("                            /██                                   ██    ████   ██\n");
    printw("                           ███████████████████████████████████████████\n");
    printw("                             ██///                                ██         ██████ ██  ██  ████ ██   ███\n");
    printw("                             ██//   Nacisnij dowolny klawisz      ██            ██   ████  ██    ██  ██ ██\n");
    printw("                             ██/                                  ██           ██     ██   ██    ██ ███████\n");
    printw("                             ██   ruch - klawiatura numeryczna    ██         ██████   ██    ████ ██ ██   ██\n");
    printw("                             ██   r - restart  |  q - koniec      ██\n");
    printw("                             ██   s - zapis    |  l - wczytanie  /██\n");
    printw("                             ██             -------             //██\n");
    printw("                             ██                                ///██\n");
    printw("                           ███████████████████████████████████████████\n");
    printw("                             ██                                   ██/\n");
    refresh();
}

void finished_screen(int width, int height) {
    int start_X = width/2 - 10;
    int start_Y = height/2 - 4;
    
    int cur_Y = start_Y;
    move(cur_Y++, start_X);
    printw("*------------------*");move(cur_Y++, start_X);
    printw("|     *fanfary*    |");move(cur_Y++, start_X);
    printw("|    ZWYCIESTWO!   |");move(cur_Y++, start_X);
    printw("|       ----       |");move(cur_Y++, start_X);
    printw("| nacisnij dowolny |");move(cur_Y++, start_X);
    printw("|     klawisz_     |");move(cur_Y++, start_X);
    printw("*------------------*");move(cur_Y++, start_X);
    refresh();
    
    int h;
    h = getch();
}

void draw_map(int (*map)[mapX][mapY], int width, int height, int x, int y, int endX, int endY) {
    // if map is smaller than screen size, use map height as screen height
    if (mapY-height < 0)
        height = mapY;
    
    // if map is smaller than screen size, use map width as screen width
    if (mapX - (width/2) < 0)
        width = mapX;
    
    int tmpX = 0;
    
    if (x > (width/4)) {
        tmpX = x - (width/4);
    }
    
    int tmpY = y-(height/2);
    
    if (tmpY < 0)
        tmpY = 0;
    
    if (tmpY > mapY-height)
        tmpY = mapY-height;

    int i, j;
    
    for (j = tmpY; j<tmpY+height; j++) { // height 
        move(j-tmpY, 0);
        clrtoeol();
        for (i = tmpX; i<=(tmpX+width/2)-2; i++) {
            if (j==y && i==x) printw("\\/");
            else if (j==endY && i == endX) printw("[]");
            else {
            if ((*map)[i][j] != 0)
                printw("██", (*map)[i][j]);
            else printw("  "); }
        }
    }
    
    refresh();
}

int is_taken(int x, int y, int (*map)[mapX][mapY]) {
    // calculate x range
    int start_x = x*3;
    int end_x = start_x+2;
    
    // calculate y range
    int start_y = y*3;
    int end_y = start_y+2;
    
    // iterate through all blocks
    int i,j;
    for (j = start_y; j <= end_y; j++)
        for (i = start_x; i <= end_x; i++)
            // if any block taken, return 1
            if ((*map)[i][j] != 0) return 1;
    
    return 0;
}

void set_block(int x, int y, int old_direction, int direction, int (*map)[mapX][mapY]) {
    // calculate x range
    int start_x = x*3;
    int mid_x = start_x+1;
    int end_x = start_x+2;
    
    // calculate y range
    int start_y = y*3;
    int mid_y = start_y+1;
    int end_y = start_y+2;
    
    int element = 8;
    switch (direction) {
        case 0:
            if (old_direction == L_LEFT || old_direction == L_RIGHT)  element = E_HORIZONTAL_TUNNEL;
            if (old_direction == L_UP   || old_direction == L_DOWN)   element = E_VERTICAL_TUNNEL;
            element = E_FILLED;
            break;
        case L_UP: 
            element = E_VERTICAL_TUNNEL; 
            if (old_direction == L_LEFT)  element = E_DOWN_LEFT;
            if (old_direction == L_RIGHT) element = E_DOWN_RIGHT;
            break;
        case L_LEFT:
            element = E_HORIZONTAL_TUNNEL;
            if (old_direction == L_DOWN)  element = E_DOWN_RIGHT;
            if (old_direction == L_UP)    element = E_RIGHT_DOWN;
            break;
        case L_RIGHT:
            element = E_HORIZONTAL_TUNNEL;
            if (old_direction == L_DOWN) element = E_DOWN_LEFT;
            if (old_direction == L_UP)   element = E_LEFT_DOWN;
            break;
        case L_DOWN:
            element = E_VERTICAL_TUNNEL;
            if (old_direction == L_LEFT)  element = E_LEFT_DOWN;
            if (old_direction == L_RIGHT) element = E_RIGHT_DOWN;
            break;
        case E_ZEROS:
            element = E_ZEROS;
            break;
    }
    
    switch (element) {
        case E_HORIZONTAL_TUNNEL:
            (*map)[start_x][start_y] = (*map)[mid_x][start_y] = (*map)[end_x][start_y] = 1;  // ###
                                                                                             //
            (*map)[start_x][end_y]   = (*map)[mid_x][end_y]   = (*map)[end_x][end_y]   = 1;  // ###
            break;

        case E_VERTICAL_TUNNEL:
            (*map)[start_x][start_y] = (*map)[end_x][start_y] = 1;                           // # #
            (*map)[start_x][mid_y]   = (*map)[end_x][mid_y]   = 1;                           // # #
            (*map)[start_x][end_y]   = (*map)[end_x][end_y]   = 1;                           // # #
            break;
        case E_LEFT_DOWN:
            (*map)[start_x][start_y] = (*map)[mid_x][start_y] = (*map)[end_x][start_y] = 1;  // ###
            (*map)[start_x][mid_y] = 1;                                                      // # 
            (*map)[start_x][end_y] = (*map)[end_x][end_y] = 1;                               // # #
            break;
        case E_RIGHT_DOWN:
            (*map)[start_x][start_y] = (*map)[mid_x][start_y] = (*map)[end_x][start_y] = 1;  // ###
            (*map)[end_x][mid_y] = 1;                                                        //   #
            (*map)[start_x][end_y] = (*map)[end_x][end_y] = 1;                               // # #
            break;
        case E_DOWN_LEFT:
            (*map)[start_x][start_y] = (*map)[end_x][start_y] = 1;                           // # #
            (*map)[start_x][mid_y] = 1;                                                      // #
            (*map)[start_x][end_y] = (*map)[mid_x][end_y] = (*map)[end_x][end_y] = 1;        // ###
            break;
        case E_DOWN_RIGHT:
            (*map)[start_x][start_y] = (*map)[end_x][start_y] = 1;                           // # #
            (*map)[end_x][mid_y] = 1;                                                        //   #
            (*map)[start_x][end_y] = (*map)[mid_x][end_y] = (*map)[end_x][end_y] = 1;        // ###
            break;
        case E_FILLED:
            (*map)[start_x][start_y] = (*map)[mid_x][start_y] = (*map)[end_x][start_y] = 1;
            (*map)[start_x][mid_y] = (*map)[mid_x][mid_y] = (*map)[end_x][mid_y] = 1;
            (*map)[start_x][end_y] = (*map)[mid_x][end_y] = (*map)[end_x][end_y] = 1;
            break;
        case E_ZEROS:
            (*map)[start_x][start_y] = (*map)[mid_x][start_y] = (*map)[end_x][start_y] = 0;
            (*map)[start_x][mid_y] = (*map)[mid_x][mid_y] = (*map)[end_x][mid_y] = 0;
            (*map)[start_x][end_y] = (*map)[mid_x][end_y] = (*map)[end_x][end_y] = 0;
            break;
    }
    
    // get console data
    struct winsize max;
    ioctl(0, TIOCGWINSZ , &max);
    
    if (DRAW_DEBUG == 1) {
        draw_map(map, max.ws_col, max.ws_row, (x*3)+1, (y*3)+1, -1, -1);
        usleep(50000);
    }
}

int test_gen_direction(int (*map)[mapX][mapY], int direction, int x, int y) {
    int newX = x; int newY = y;
        
    switch (direction) {
        case 0: return 0;
        case L_UP   : newY -= 1; break;
        case L_LEFT : newX -= 1; break; 
        case L_DOWN : newY += 1; break;
        case L_RIGHT: newX += 1; break;
    }
    
    // check if is in range
    if (newX < 0 || newX*3 >= mapX) return 0;
    if (newY < 0 || newY*3 >= mapY) return 0;
        
    // avoid overwriting blocks
    if (is_taken(newX, newY, map)) return 0;
    
    return 1;
    
}

void mapgen(int (*map)[mapX][mapY], int startX, int startY, int (*taken_map)[mapX][mapY], int *endX, int *endY) {
    // calculate length
    int length = (120*63)/100;
    
    // initialize array for storing path
    int path[length];
    
    int x = startX; int y = startY;
        
    int z=0;
    while (z<length) {        
        // starting with 0
        int old_direction = 0;
        
        // if not first element, get old one
        if (z > 0)
            old_direction = path[z-1];
        
        // copy position to new variables
        int newX = x; int newY = y;
        
        int legal_moves[5] = {0, 0, 0, 0, 0}; // count, 1, 2, 3, 4
                
        if (test_gen_direction(taken_map, L_UP, x, y) == 1 && old_direction != L_DOWN) {
            legal_moves[0] += 1;
            legal_moves[legal_moves[0]] = L_UP;
        }
        if (test_gen_direction(taken_map, L_DOWN, x, y) == 1 && old_direction != L_UP) {
            legal_moves[0] += 1;
            legal_moves[legal_moves[0]] = L_DOWN;
        }
        
        if (test_gen_direction(taken_map, L_LEFT, x, y) == 1 && old_direction != L_RIGHT) {
            legal_moves[0] += 1;
            legal_moves[legal_moves[0]] = L_LEFT;
        }
        
        if (test_gen_direction(taken_map, L_RIGHT, x, y) == 1 && old_direction != L_LEFT) {
            legal_moves[0] += 1;
            legal_moves[legal_moves[0]] = L_RIGHT;
        }
        
        int direction = 0;
        
        if (legal_moves[0] > 1) {
            // randomize direction
            int random = (rand()/(float)RAND_MAX)*legal_moves[0];
            random++;
            direction = legal_moves[random];
        } else if (legal_moves[0] == 1) {
            direction = legal_moves[1];
        } else if (legal_moves[0] == 0) {
            // no more moves, stop
            set_block(x, y, old_direction, E_ZEROS, map);
            set_block(x, y, old_direction, direction, map);
            
            // make sure we'll get out of the loop
            direction = 0;
            z = length;
        }
        
        
        switch (direction) {
            case 0: break;
            case L_UP   : newY -= 1; break;
            case L_LEFT : newX -= 1; break; 
            case L_DOWN : newY += 1; break;
            case L_RIGHT: newX += 1; break;
        }
        
        // exit if no direction
        if (direction != 0) {
            // yaay, it worked!
            path[z] = direction;
            z++;       

            // reset element
            set_block(x, y, 0, E_ZEROS, map);

            // set element
            set_block(x, y, old_direction, direction, map);

            (*endX) = (x*3)+1;
            (*endY) = (y*3)+1;

            // make sure blocks are registered on both maps
            if (map != taken_map) {
                set_block(x, y, 0, E_ZEROS, taken_map);
                set_block(x, y, old_direction, direction, taken_map);
            }

            // set old direction to current
            old_direction = direction;

            x = newX; y = newY;
        }
    }
}

void join_paths(int (*map)[mapX][mapY]) {
    int x = 0; int y = 0;
    int przesuniecie = 0;
    
    for (y = 0; y<(mapY/3)-1; y++) {
        for (x = 0; x<mapX/3; x++) {           
            int sx = x*3;
            int sy = y*3;
            for (przesuniecie = 0; przesuniecie <= 2; przesuniecie++) {
                if ((*map)[sx][sy+1+przesuniecie] && (*map)[sx+1][sy+1+przesuniecie] && (*map)[sx+2][sy+1+przesuniecie]) {
                    if ((*map)[sx+1][sy+przesuniecie] == 0 && (*map)[sx+1][sy+2+przesuniecie] == 0) {
                        (*map)[sx+1][sy+1+przesuniecie] = 0;
                    }
                }
            }
        }
    }
    
    for (y = 0; y<mapY/3; y++) {
        for (x = 0; x<(mapX/3)-1; x++) {           
            int sx = x*3;
            int sy = y*3;
            for (przesuniecie = 0; przesuniecie <= 2; przesuniecie++) {
                if ((*map)[sx+1+przesuniecie][sy] && (*map)[sx+1+przesuniecie][sy+1] && (*map)[sx+1+przesuniecie][sy+2]) { // jest pionowa liinia
                    if ((*map)[sx+przesuniecie][sy+1] == 0 && (*map)[sx+2+przesuniecie][sy+1] == 0) {
                        (*map)[sx+1+przesuniecie][sy+1] = 0;
                    }
                }
            }
        }
    }
}

void hardcore_mapgen(int (*map)[mapX][mapY], int *endX, int *endY) {
    int x, y;
    
    // reset map
    for (y=0; y<mapY; y++)
        for (x=0; x<mapX; x++)
            (*map)[x][y] = 0;
    
    // generate path starting from every empty block
    for (y = 0; y<mapY/3; y++)
        for (x = 0; x<mapX/3; x++)
            if (is_taken(x, y, map) == 0) // check if block is empty
                mapgen(map,x,y,map, endX, endY); // generate path
    
    // generate final route
    int final_map[mapX][mapY] = {0};
    mapgen(map, 0, 0, &final_map, endX, endY);
    
    // remove walls between paths to merge them
    join_paths(map);
}

int move_character(int (*map)[mapX][mapY], int *x, int *y) {
    // get direction from user
    int btn = 0;
    btn = getch();
    
    switch (btn) {
        case 56: 
            if ((*map)[(*x)][(*y)-1] == 0) 
                (*y) -= 1; 
            if (HARDCORE_MODE) (*map)[(*x)][(*y)] = 1;
            break;
        case 50: 
            if ((*map)[(*x)][(*y)+1] == 0) 
                (*y) += 1; 
            if (HARDCORE_MODE) (*map)[(*x)][(*y)] = 1;
            break;
        case 52: 
            if ((*map)[(*x)-1][(*y)] == 0) 
                (*x) -= 1; 
            if (HARDCORE_MODE) (*map)[(*x)][(*y)] = 1;
            break;
        case 54: 
            if ((*map)[(*x)+1][(*y)] == 0) 
                (*x) += 1;
            if (HARDCORE_MODE) (*map)[(*x)][(*y)] = 1;
            break;
        case 115:
            return G_SAVE;
            break;
        case 113:
            return G_QUIT;
            break;
        case 108:
            return G_LOAD;
            break;
        case 114:
            return G_RESET;
            break;
    }
    
    return G_CONTINUE;
}

void save_load(int (*map)[mapX][mapY], int *x, int *y, int *endX, int *endY, int action) {
    FILE *fp;
    int x1, y1;
    
    switch (action) {
        case A_SAVE:
            fp = fopen("zapis.txt", "w");
            fprintf(fp, "%d %d %d %d \n", *x, *y, *endX, *endY);

            // save map
            for (y1 = 0; y1<mapY; y1++) {
                for (x1 = 0; x1<mapX; x1++)
                    fprintf(fp, "%d ", (*map)[x1][y1]);
                fprintf(fp, "\n");
            }

            fclose(fp);
            break;
        case A_LOAD:
            printw("Wczytywanie stanu gry...\n");
            fp = fopen("zapis.txt", "r");
            fscanf(fp, "%d", x);
            fscanf(fp, "%d", y);
            fscanf(fp, "%d", endX);
            fscanf(fp, "%d", endY);

            // load map
            for (y1 = 0; y1<mapY; y1++)
                for (x1 = 0; x1<mapX; x1++)
                    fscanf(fp, "%d", &(*map)[x1][y1]);

            fclose(fp);
            break;
    }
}

int main() {
    // set locale
    setlocale(LC_ALL,"");
    
    // randomize
    srand(time(NULL));
    
    // make sure game resets at the beginning
    int restart = 1;
    
    // initialize values
    int x, y, endX, endY;
    int map[mapX][mapY] = {0};
    
    // initialize ncurses
    initscr();
    noecho();
    curs_set(0);
    
    // display title screen
    title_screen();
    
    // wait for user to press the key
    int h; h = getch();
    
    // get console data
    struct winsize max;
    ioctl(0, TIOCGWINSZ , &max);

    // if key is l, load game state
    if (h==108) {
        save_load(&map,&x,&y,&endX,&endY, A_LOAD); // load previous state
        restart = 0; // make sure game won't restart
        usleep(600000); // make sure user got time to read
    }
    
    int is_running = 1;
    
    // game loop
    while (is_running) {
        // check if player got to the end
        if (x == endX && y == endY) {
            finished_screen(max.ws_col-1, max.ws_row-1);
            restart = 1; // make sure game restarts
        }
        
        // if restart is set to 1, reset all values
        if (restart == 1) {
            x = 1; y = 1;
            hardcore_mapgen(&map, &endX, &endY);
            restart = 0;
        }
        
        // draw map on screen
        draw_map(&map,max.ws_col, max.ws_row, x, y, endX, endY);
        
        h = move_character(&map, &x, &y);
        switch (h) {
            case G_SAVE:
                save_load(&map,&x,&y,&endX,&endY, A_SAVE);
                break;
            case G_QUIT:
                is_running = 0;
                break;
            case G_RESET:
                restart = 1;
                break;
            case G_LOAD:
                save_load(&map,&x,&y,&endX,&endY, A_LOAD);
                break;
        }
    }
    
    endwin();
    return 0;
}