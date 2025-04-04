// #include "global.h"
#include "../src/gt/input.h"
#define GS_MAIN_SCREEN 0
#define GS_CHARACTER_SELECT_SCREEN 1
#define GS_TABLE_SELECT_SCREEN 2
#define GS_GAME_SCREEN 3
#define GS_FREE_BALL_MOVE 4
#define GS_PAUSE 5

typedef struct{
  signed int x, y;
}point;

typedef struct{
  point pos;
  char l, w;
}hitbox;

extern signed int testint;
extern char game_state;
extern signed char ball_x, ball_y, box_trigger, ball_ym, ball_xm;
extern signed char last_reflect_angle;
extern signed int ball_xf, ball_yf, last_yf, last_xf;//ball_um=0, ball_dm=0, ball_lm, ball_rm;
extern signed int freei;
extern point ball_pos;
extern const char ball_radius;
extern hitbox barriers[10];

extern signed char cos_table[];
extern signed char sin_table[];

extern void init_table();
extern void do_pinball();
extern signed int min(signed int a, signed int b);
extern signed int max(signed int a, signed int b);
extern void move_ball();
extern unsigned int fast_abs(int x);
extern signed char atanfn(char y, char x);
extern int get_reflect_angle(signed char x, signed char y);
void check_collision();
