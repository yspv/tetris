#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>


enum {
    block_up    = 8,
    block_right = 4,
    block_down  = 2,
    block_left  = 1,
    block_empty = -1,

    block_row   = 2,
    block_col   = 4,
    block_count = 7,

    box_height  = 20,
    box_width   = 20,

    gameover_width = 95,
    gameover_heigth = 14,

    key_escape  = 27,
    KEY_SPACE   = 32,
};

char g_char[] = 
    "  _______n" 
    " / _____/n"
    "/ /_____ n"
    "|  ___  |n"
    "| |___| |n"
    "\\_______| ";

char a_char[] =
    "     ____ n"
    "    / __ |n"
    "   / / | |n"
    "  / /__| |n"
    " / ____  |n"
    "/_/    |_| ";

char m_char[] =
    " __      __ n"
    "|  \\    /  |n"
    "|   \\  /   |n"
    "| |\\ \\/ /| |n"
    "| | \\__/ | |n"
    "|_|      |_| ";

char e_char[] = 
    "________n"
    "| _____/n"
    "| |___  n"
    "| ___/  n"
    "| |____ n"
    "|______|";

char o_char[] = 
    " _______ n"
    "/  ___  \\n"
    "| |   | |n"
    "| |   | |n"
    "| |___| |n"
    "\\_______/";

char v_char[] =
    "__     _n"
    "\\ \\   | |n"
    " \\ \\  | |n"
    "  \\ \\ | |n"
    "   \\ \\| |n"
    "    \\___|";

char r_char[] =
    " ______ n"
    "|  __  \\n"
    "| |__| | n"
    "|  __  / n"
    "| |  \\ \\ n"
    "|_|   \\_\\";

char *gameover_chars[] = {
    g_char,
    a_char,
    m_char,
    e_char,
    o_char,
    v_char,
    e_char,
    r_char,
};

struct curinfo {
    int x;
    int y;
    int box_y;
    int box_x;
};

typedef void (*Rotate)(int, int, int*, int*);

typedef char Block[block_row][block_col];

Block i_block = {
    {11, 10, 10, 14},
    {-1, -1, -1, -1},
};

Block t_block = {
    { 0, 13,  0, -1},
    {11,  2, 14, -1},
};

Block j_block = {
    {13,  0,  0, -1},
    { 3, 10, 14, -1},
};

Block l_block = {
    {0,  0,  13, -1},
    {11, 10,  6, -1},
};

Block s_block = {
    { 0, 9, 14, -1},
    {11, 6, 0, -1},
};

Block z_block = {
    {11, 12, 0, -1},
    { 0, 3, 14, -1},
};

Block o_block = {
    {9, 12, -1, -1},
    { 3, 6, -1, -1},
};

Block *blocks[] = {
    &i_block,
    &t_block,
    &j_block,
    &l_block,
    &s_block,
    &z_block,
    &o_block,
};

void degree_0(int cr, int cc, int *r, int *c)
{
    *r = cr;
    *c = cc;
}

void degree_90(int cr, int cc, int *r, int *c)
{
    *r = (block_row - 1) - cr;
    *c = cc;
}

void degree_180(int cr, int cc, int *r, int *c)
{
    *r = (block_row - 1) - cr;
    *c = (block_col - 1) - cc;
}

void degree_270(int cr, int cc, int *r, int *c)
{
    *r = cr;
    *c = (block_col - 1) - cc;
}

typedef enum {
    dg0 = 0,
    dg90,
    dg180,
    dg270,
} Degree;

Rotate rotate[] = {
    degree_0,
    degree_90,
    degree_180,
    degree_270,
};

void show_block(Block block, Degree dg, int y, int x)
{
    int r, c, ty, tx, flag;

    flag = ty = tx = 0;

    if(dg != dg0 && dg != dg180)
        flag = 1;

    for(int i = 0; i < block_row; i++) {    
        if(flag) ty = 0;
        else tx = 0;
        for(int l = 0; l < block_col; l++) {
            (rotate[dg])(i, l, &r, &c);
            move(y + ty, x + tx);
            if(block[r][c] == block_empty)
                continue;
            if(block[r][c])
                addstr("[]");
            if(flag) ty += 1;
            else tx += 2;
        }
        if(flag) tx += 2;
        else ty += 1;
    }
    refresh();
}

void hide_block(Block block, Degree dg, int y, int x)
{
    int r, c, ty, tx, flag;

    flag = ty = tx = 0;

    if(dg != dg0 && dg != dg180)
        flag = 1;

    for(int i = 0; i < block_row; i++) {
        if(flag) ty = 0;
        else tx = 0;
        for(int l = 0; l < block_col; l++) {
            (rotate[dg])(i, l, &r, &c);
            move(y + ty, x + tx);
            if(block[r][c] == block_empty)
                continue;
            if(block[r][c])
                addstr("  ");
            if(flag) ty += 1;
            else tx += 2;
        }
        if(flag) tx += 2;
        else ty += 1;
    }
    refresh();
}

void move_block(Block block, Degree dg, struct curinfo *cur, int y, int x)
{
    hide_block(block, dg, cur->y, cur->x);
    cur->y += y;
    cur->x += x;
    show_block(block, dg, cur->y, cur->x);
}

static int identify_side(Degree dg, int block_side)
{
    int side = 0;

    switch(dg) {
    case dg0:
        side = block_side;
        break;
    case dg90:
        side = block_side << 1;
        if(side > 8) side = 1;
        break;
    case dg180:
        side = block_side << 2;
        if(side > 16) side = 2;
        if(side > 8)  side = 1;
        break;
    case dg270:
        side = block_side >> 1;
        if(side < 1) side = 8;
        break;
    }

    return side;
}

int check_block(Block block, Degree dg, int block_side,
                struct curinfo *cur, int y, int x)
{
    int r, c, bl, ty, tx, flag, side;
    flag = ty = tx = 0;
    
    side = identify_side(dg, block_side);

    if(dg != dg0 && dg != dg180)
        flag = 1;

    for(int i = 0; i < block_row; i++) {
        if(flag) ty = 0;
        else tx = 0;
        for(int l = 0; l < block_col; l++) {
            (rotate[dg])(i, l, &r, &c);
            bl = block[r][c];
            if(bl == block_empty)
                continue;
            if(bl & side) {
                if(mvinch(cur->y + y + ty, cur->x + x + tx) != ' ')
                    return 1;
            }
            if(flag) ty += 1;
            else tx += 2;
        }
        if(flag) tx += 2;
        else ty += 1;
    }

    return 0;
}

int is_empty(Block block, Degree dg, struct curinfo *cur)
{
    int bl, r, c, ty, tx, flag;
    if(dg != dg0 && dg != dg180)
        flag = 1;
    for(int i = 0; i < block_row; i++) {
        if(flag) ty = 0;
        else tx = 0;
        for(int l = 0; l < block_col; l++) {
            (rotate[dg])(i, l, &r, &c);
            bl = block[r][c];
            if(bl == block_empty)
                continue;
            if(bl) {
                if(mvinch(cur->y + ty, cur->x + tx) != ' ')
                    return 0;
            }
            if(flag) ty += 1;
            else tx += 2;
        }
        if(flag) tx += 2;
        else ty += 1;
    }
    return 1;
}


void remove_line(int line, int x)
{
    for(int i = 1; i < box_width + 1; i++) {
        move(line, x + i);
        addch(' ');
    }
    refresh();
}

void down_line(int line, int x)
{
    int ch;
    for(int i = 1; i < box_width + 1; i++) {
        ch = mvinch(line - 1, x + i);
        move(line, x + i);
        addch(ch);
    }
    refresh();
}

void check_line(struct curinfo *cur, int *line)
{
    int i, l;
    for(i = box_height; i > 1; i--) {
        int count = 0;
        for(l = 1; l < box_width; l += 2) {
            if(mvinch(i + cur->box_y, cur->box_x + l) != ' ')
                count++;
        }
        if(count == 10) {
            for(int h = i + cur->box_y; h > cur->box_y + 1; h--) {
                remove_line(h, cur->box_x);
                down_line(h, cur->box_x);
            }
            *line += 1;
            i += 1;
        }
    }
}

void block_control(int key, Block *block, Degree *dg, 
                    struct curinfo *cur, int *score)
{
    switch(key) {
    case KEY_UP:
        hide_block(*block, *dg, cur->y, cur->x);
        *dg = *dg + 1;
        if(*dg > dg270)
            *dg = dg0;
        //if(!is_empty(*block, *dg, cur))
          // *dg = *dg != dg0 ? *dg- 1 : dg270;
        show_block(*block, *dg, cur->y, cur->x);
        break;
    case KEY_DOWN:
        if(check_block(*block, *dg, block_down, cur, 1, 0))
            break;
        move_block(*block, *dg, cur, 1, 0);
        *score += 1;
        break;
    case KEY_LEFT:
        if(check_block(*block, *dg, block_left, cur, 0, -2))
            break;
        move_block(*block, *dg, cur, 0, -2);
        break;
    case KEY_RIGHT:
        if(check_block(*block, *dg, block_right, cur, 0, 2))
            break;
        move_block(*block, *dg, cur, 0, 2);
        break;
    case KEY_SPACE:
        while(!check_block(*block, *dg, block_down, cur, 1, 0)) {
            move_block(*block, *dg, cur, 1, 0);
            *score += 1;
        }
        break;
    }
}

void print_box(struct curinfo *cur)
{
    int i;
    move(cur->box_y, cur->box_x-1);
    addch('|');
    for(i = 1; i < box_height + 1; i++) {
        move(cur->box_y + i, cur->box_x - 1);
        addstr("||");
    }
    move(cur->box_y + i, cur->box_x - 1);
    addch('|');
    for(i = 0; i < box_width + 2; i++) {
        move(cur->box_y, cur->box_x + i);
        addch('=');
    }
    move(cur->box_y, cur->box_x + i);
    addch('|');
    for(i = 1; i < box_height + 1; i++) {
        move(cur->box_y + i, cur->box_x + box_width + 1);
        addstr("||");
    }
    move(cur->box_y + i, cur->box_x + box_width + 1);
    addch('|');
    for(i = 0; i < box_width + 2; i++) {
        move(cur->box_y + box_height + 1, cur->box_x + i);
        addch('=');
    }
    move(cur->box_y + box_height + 1, cur->box_x + i);
    addch('|');
}

void print_gameover(int y, int x)
{
    int ty, tx;
    char *cur_char;
    for(int i = 0; i < 8; i++) {
        cur_char = gameover_chars[i];
        ty = tx = 0;
        if(i == 4) {
            y += gameover_heigth / 2;
            x -= gameover_width / 2;
        }
        for(int l = 0; cur_char[l] != '\0'; l++) {
            if(cur_char[l] == 'n') {
                ty += 1;
                tx = 0;
                continue;
            }
            mvaddch(y + ty, x + tx++, cur_char[l]);
        }
        x += tx + 2;
    }
    refresh();
}

void print_line(int line, struct curinfo *cur)
{
    char linestr[12];
    sprintf(linestr, "%d", line);
    move(cur->box_y - 2, cur->box_x);
    addstr("Line: ");
    addstr(linestr);
    refresh();
}

void print_score(int score, struct curinfo *cur)
{
    char scorestr[12];
    sprintf(scorestr, "%d", score);
    move(cur->box_y - 2, cur->box_x + (box_width - sizeof("Score: ")));
    addstr("Score: ");
    addstr(scorestr);

}

int main()
{   
    struct curinfo cur;
    int rowsrc, colsrc, n;
    int key = 0;
    int line = 0;
    int score = 0;
    Block *block;
    Degree degree;

    initscr();
    cbreak();
    keypad(stdscr, 1);
    noecho();
    curs_set(0);
    getmaxyx(stdscr, rowsrc, colsrc);
    cur.y = 0;
    cur.x = 0;
    cur.box_y = (rowsrc - box_height) / 2;
    cur.box_x = (colsrc - box_width) / 2;
    timeout(500);
    srand(time(NULL));
    print_box(&cur);
    while(key != key_escape) {
        cur.y = cur.box_y + 1;
        cur.x = cur.box_x + (box_width - block_col - 1) / 2;

        n = rand() % block_count;
        block = blocks[n];
        degree = dg0;
        print_line(line, &cur);
        print_score(score, &cur);
        show_block(*block, degree, cur.y, cur.x);
        while((key = getch()) != key_escape) {
            block_control(key, block, &degree, &cur, &score);
            if(check_block(*block, degree, block_down, &cur, 1, 0))
                break;
            move_block(*block, degree, &cur, 1, 0);
        }
        check_line(&cur, &line);
        if(cur.y == cur.box_y + 1) {
            clear();
            print_gameover((rowsrc - gameover_heigth) / 2, 
                           (colsrc - (gameover_width / 2)) / 2);
            getchar();
            endwin();
            return 0;
        }
    }
    endwin();
    return 0;
}
