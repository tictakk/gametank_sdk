#include "../include/pinball.h"
#include "gt/gametank.h"
#include "gt/input.h"
#include "tabledata.h"
#include "gt/banking.h"
#include "gen/bank_nums.h" //Generated macros for named bank numbers

#define BALL_START_X 45
#define BALL_START_Y 75
#define BALL_SIZE_X 10
#define BALL_SIZE_Y 10

#define MAX_MOMENTUM 127
#define FALL_SPEED 3

//WANTED:
//0110 0100 >> 0110
//INSTEAD
//xxxx x111 1000 0000

#define TABLE_BOUNDS_X 118
#define TABLE_BOUNDS_Y 100

#define FIXED_SHIFT 8
#define FIXED_ONE (1 << FIXED_SHIFT)
#define TO_FIXED(x) ((x) << FIXED_SHIFT)
#define FROM_FIXED(x) ((x) >> FIXED_SHIFT)
// #define FIXED_MUL(a, b) (((a) * (b)) >> FIXED_SHIFT)
#define FIXED_MUL(a, b) (((((a) >> 4) * (b) >> 4))) >> (FIXED_SHIFT - 8)
#define FIXED_DIV(a, b) (((a) << FIXED_SHIFT) / (b))

#define GRAVITY 5//TO_FIXED(2)
#define DAMPING (FIXED_ONE * 99 / 100)

#define TO_SIGNED_CHAR(x) ((signed char)((x) > 127 ? 127 : ((x) < -128 ? -128: (x))))
#define TO_SIGNED_INT(x) ((int)(x))

signed char box_trigger=0, ball_ym=0, ball_xm=0;
signed int last_x=0, last_y=0, fake_x=0, fake_y=0;
// signed char ball_x=0, ball_y=0, box_trigger=0;
signed int ball_xf=0, ball_yf=0, last_yf, last_xf;//ball_um=0, ball_dm=0, ball_lm, ball_rm;
signed int freei;
point ball_pos;
hitbox barriers[10];
const char ball_radius=4;
signed char last_reflect_angle=0;

signed int testint=0;

// float fy = 0.0f;
char atan[16][16] = {
  0, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
  0, 15, 22, 25, 26, 27, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30,
  0, 9, 15, 19, 22, 24, 25, 26, 26, 27, 27, 28, 28, 28, 28, 29,
  0, 6, 11, 15, 18, 20, 22, 23, 24, 25, 25, 26, 26, 27, 27, 27,
  0, 4, 9, 13, 15, 18, 19, 21, 22, 23, 24, 24, 25, 25, 26, 26,
  0, 3, 7, 10, 13, 15, 17, 19, 20, 21, 22, 23, 23, 24, 24, 25,
  0, 3, 6, 9, 11, 14, 15, 17, 18, 19, 20, 21, 22, 23, 23, 24,
  0, 2, 5, 8, 10, 12, 14, 15, 17, 18, 19, 20, 21, 21, 22, 22,
  0, 2, 4, 7, 9, 11, 13, 14, 15, 17, 18, 19, 19, 20, 21, 21,
  0, 2, 4, 6, 8, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 20,
  0, 2, 3, 5, 7, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 19,
  0, 1, 3, 5, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 18,
  0, 1, 3, 4, 6, 7, 9, 10, 11, 13, 14, 14, 15, 16, 17, 18,
  0, 1, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 15, 15, 16, 17,
  0, 1, 2, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 15, 16,
  0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15
};

// char atan[16][16] = { //(y,x)
//   0, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
//   0, 32, 45, 50, 54, 55, 57, 58, 58, 59, 59, 60, 60, 60, 61, 61,
//   0, 18, 32, 40, 45, 48, 50, 52, 54, 55, 55, 56, 57, 57, 58, 58,
//   0, 13, 23, 32, 37, 41, 45, 47, 49, 50, 52, 53, 54, 54, 55, 55,
//   0, 9, 18, 26, 32, 36, 40, 42, 45, 46, 48, 49, 50, 51, 52, 53,
//   0, 8, 15, 22, 27, 32, 35, 38, 41, 43, 45, 46, 47, 49, 50, 50,
//   0, 6, 13, 18, 23, 28, 32, 35, 37, 40, 41, 43, 45, 46, 47, 48,
//   0, 5, 11, 16, 21, 25, 28, 32, 34, 37, 39, 40, 42, 43, 45, 46,
//   0, 5, 9, 14, 18, 22, 26, 29, 32, 34, 36, 38, 40, 41, 42, 44,
//   0, 4, 8, 13, 17, 20, 23, 26, 29, 32, 34, 36, 37, 39, 40, 41,
//   0, 4, 8, 11, 15, 18, 22, 24, 27, 29, 32, 33, 35, 37, 38, 40,
//   0, 3, 7, 10, 14, 17, 20, 23, 25, 27, 30, 32, 33, 35, 36, 38,
//   0, 3, 6, 9, 13, 16, 18, 21, 23, 26, 28, 30, 32, 33, 35, 36,
//   0, 3, 6, 9, 12, 14, 17, 20, 22, 24, 26, 28, 30, 32, 33, 34,
//   0, 2, 5, 8, 11, 13, 16, 18, 21, 23, 25, 27, 28, 30, 32, 33,
//   0, 2, 5, 8, 10, 13, 15, 17, 19, 22, 23, 25, 27, 29, 30, 32
// };

// signed char sin_table[256] = {
//   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
//   16,17,19,20,20,21,22,23,24,25,26,27,28,28,29,
//   30,31,32,33,33,34,35,35,36,37,37,38,38,39,39,
//   40,40,41,41,41,42,42,43,43,43,43,44,44,44,44,
//   44,44,44,44,44,44,44,44,44,44,44,43,43,43,43,
//   42,42,41,41,41,40,40,39,39,38,38,37,37,36,35,
//   35,34,33,33,32,31,30,29,28,28,27,26,25,24,23,
//   22,21,20,20,19,17,16,15,14,13,12,11,10,9,8,7,
//   6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,
//   -11,-12,-13,-14,-15,-16,-17,-19,-20,-20,-21,
//   -22,-23,-24,-25,-26,-27,-28,-28,-29,-30,-31,
//   -32,-33,-33,-34,-35,-3,-36,-37,-37,-38,-38,
//   -39,-39,-40,-40,-41,-41,-41,-42,-42,-43,-43,
//   -43,-43,-44,-44,-44,-44,-44,-44,-44,-44,-44,
//   -44,-44,-44,-44,-44,-44,-43,-43,-43,-43,-42,
//   -42,-41,-41,-41,-40,-40,-39,-39,-38,-38,-37,
//   -37,-36,-35,-35,-34,-33,-33,-32,-31,-30,-29,
//   -28,-28,-27,-26,-25,-24,-23,-22,-21,-20,-20,
//   -19,-17,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,
//   -6,-5,-4,-3,-2,-1
// };

// signed char cos_table[256] = {
//   44,44,44,44,44,44,44,44,43,43,43,43,42,42,41,
//   41,41,40,40,39,39,38,38,37,37,36,35,35,34,33,
//   33,32,31,30,29,28,28,27,26,25,24,23,22,21,20,
//   20,19,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,
//   2,1,0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,
//   -13,-14,-15,-16,-17,-19,-20,-20,-21,-22,-23,
//   -24,-25,-26,-27,-28,-28,-29,-30,-31,-32,-33,
//   -33,-34,-35,-35,-36,-37,-37,-38,-38,-39,-39,
//   -40,-40,-41,-41,-41,-42,-42,-43,-43,-43,-43,
//   -44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,
//   -44,-44,-44,-44,-43,-43,-43,-43,-42,-42,-41,
//   -41,-41,-40,-40,-39,-3,-38,-38,-37,-37,-36,
//   -35,-35,-34,-33,-33,-32,-31,-30,-29,-28,-28,
//   -27,-26,-25,-24,-23,-22,-21,-20,-20,-19,-17,
//   -16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,
//   -4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,
//   13,14,15,16,17,19,20,20,21,22,23,24,25,26,
//   27,28,28,29,30,31,32,33,33,34,35,35,36,37,
//   37,38,38,39,39,40,40,41,41,41,42,42,43,43,
//   43,43,44,44,44,44,44,44,44
// };

signed char cos_table[127] = { //X?
  0,    6,    12,   18,   24,   31,   37,   43,   48,   54,   60,   65,
  71,   76,   81,   85,   90,   94,   98,   102,  106,  109,  112,  115,
  117,  119,  121,  123,  124,  125,  126,  126,  126,  126,  126,  125,
  124,  122,  120,  118,  116,  113,  111,  107,  104,  100,  96,   92,
  88,   83,   78,   73,   68,   63,   57,   51,   46,   40,   34,   28,
  21,   15,   9,    3,    -3,   -9,   -15,  -21,  -28,  -34,  -40,  -46,
  -51,  -57,  -63,  -68,  -73,  -78,  -83,  -88,  -92,  -96,  -100, -104,
  -107, -111, -113, -116, -118, -120, -122, -124, -125, -126, -126, -126,
  -126, -126, -125, -124, -123, -121, -119, -117, -115, -112, -109, -106,
  -102, -98,  -94,  -90,  -85,  -81,  -76,  -71,  -65,  -60,  -54,  -48,
  -43,  -37,  -31,  -24,  -18,  -12,  -6,
};

signed char sin_table[127] = { //Y?
  127,  126,  126,  125,  124,  123,  121,  119,  117,  114,  111,  108,
  105,  101,  97,   93,   89,   84,   79,   74,   69,   64,   58,   53,
  47,   41,   35,   29,   23,   17,   10,   4,    -1,   -7,   -14,  -20,
  -26,  -32,  -38,  -44,  -50,  -56,  -61,  -67,  -72,  -77,  -82,  -86,
  -91,  -95,  -99,  -103, -106, -110, -113, -115, -118, -120, -122, -123,
  -125, -126, -126, -126, -126, -126, -126, -125, -123, -122, -120, -118,
  -115, -113, -110, -106, -103, -99,  -95,  -91,  -86,  -82,  -77,  -72,
  -67,  -61,  -56,  -50,  -44,  -38,  -32,  -26,  -20,  -14,  -7,   -1,
  4,    10,   17,   23,   29,   35,   41,   47,   53,   58,   64,   69,
  74,   79,   84,   89,   93,   97,   101,  105,  108,  111,  114,  117,
  119,  121,  123,  124,  125,  126,  126,
};

// signed char sin_table[127] = { //Y?
//   127,  126,  126,  125,  124,  123,  121,  119,  117,  114,  111,  108,
//   105,  101,  97,   93,   89,   84,   79,   74,   69,   64,   58,   53,
//   47,   41,   35,   29,   23,   17,   10,   4,    -1,   -7,   -14,  -20,
//   -26,  -32,  -38,  -44,  -50,  -56,  -61,  -67,  -72,  -77,  -82,  -86,
//   -91,  -95,  -99,  -103, -106, -110, -113, -115, -118, -120, -122, -123,
//   -125, -126, -126, -126, -126, -126, -126, -125, -123, -122, -120, -118,
//   -115, -113, -110, -106, -103, -99,  -95,  -91,  -86,  -82,  -77,  -72,
//   -67,  -61,  -56,  -50,  -44,  -38,  -32,  -26,  -20,  -14,  -7,   -1,
//   4,    10,   17,   23,   29,   35,   41,   47,   53,   58,   64,   69,
//   74,   79,   84,   89,   93,   97,   101,  105,  108,  111,  114,  117,
//   119,  121,  123,  124,  125,  126,  126,
// };

signed char atanfn(char y, char x)
{
  return atan[y][x];
}

int get_percentage(int num, int denom)
{
  int hp, hp_p;
  if(denom == 0)
  {
    return 0;
  }
  if(num < 0)
  {
    return 0;
  }

  hp = num * 100;
  hp_p = hp / denom;

  return hp_p;
}

int calc_percentage(int num, int denom)
{
  int p;
  p = num * denom;
  return p / 100;
}

signed char calc_percentagec(int num, int denom)
{
  int p;
  p = num * denom;
  return (signed char)(p / 100);
}

int get_reflect_angle(signed char x, signed char y)
{
  unsigned char offset=0;
  int angle=0;
  if(x > 0)
  {
    offset += 62;//127/2;
  }
  if(y < 0)
  {
    offset += 31;//127/4;
  }
  angle = TO_SIGNED_INT(atan[fast_abs(x)>>3][fast_abs(y)>>3]) + offset;
  return angle;
}

void reflect()
{
  // WHY ISN'T THIS PROPERLY BOUNCING??? FIGURE IT OUT
  int angle;
  int perc  = get_percentage((fast_abs(ball_xm) + fast_abs(ball_ym))/2,127);
  angle = ((get_reflect_angle(ball_ym,ball_xm))+box_trigger)%127;
  testint = get_reflect_angle(ball_ym,ball_xm);
  last_reflect_angle = angle;
  // ball_xm = cos_table[11];
  // ball_ym = sin_table[11];
  // if(perc < 20)
  // {
  //   // ball_xm = cos_table[-box_trigger];
  //   // ball_ym = sin_table[-box_trigger];
  //   ball_xm = TO_SIGNED_CHAR(calc_percentage(20,TO_SIGNED_INT(cos_table[-box_trigger])));
  //   ball_ym = TO_SIGNED_CHAR(calc_percentage(20,TO_SIGNED_INT(-sin_table[-box_trigger])));
  // }
  // else
  // {
    ball_xm = TO_SIGNED_CHAR(calc_percentage(perc,TO_SIGNED_INT(cos_table[angle])));
    ball_ym = TO_SIGNED_CHAR(calc_percentage(perc,TO_SIGNED_INT(-sin_table[angle])));
  // }


  // if(ball_pos.x > 63)
  // {
  //   last_reflect_angle = atan[(ball_ym&0b0000001111000000)>>6][(ball_xm&0b0000001111000000)>>6] + 21;
  //   // ball_xm = cos_table[last_reflect_angle+64]; //64 is where we get to negative for inclining slopes! change the pumpers from just "1"
  //   // ball_ym = -sin_table[last_reflect_angle];
  //   ball_xm = TO_SIGNED_CHAR(calc_percentage(perc,TO_SIGNED_INT(cos_table[last_reflect_angle+64])));//(signed char)calc_percentagec(perc, (signed int)cos_table[last_reflect_angle+64]);
  //   ball_ym = TO_SIGNED_CHAR(calc_percentage(perc,TO_SIGNED_INT(-sin_table[last_reflect_angle])));
  //   // last_reflect_angle = TO_SIGNED_CHAR(calc_percentagec(perc,-21));//perc;
  // }
  // else
  // {
  //   last_reflect_angle = atan[(ball_ym&0b0000001111000000)>>6][(ball_xm&0b0000001111000000)>>6];
  //   // ball_xm = -cos_table[last_reflect_angle]; //64 is where we get to negative for inclining slopes! change the pumpers from just "1"
  //   // ball_ym = -sin_table[last_reflect_angle];

  //   ball_xm = TO_SIGNED_CHAR(calc_percentage(perc,TO_SIGNED_INT(-cos_table[last_reflect_angle])));//(signed char)calc_percentagec(perc, (signed int)cos_table[last_reflect_angle+64]);
  //   ball_ym = TO_SIGNED_CHAR(calc_percentage(perc,TO_SIGNED_INT(-sin_table[last_reflect_angle])));
  // }
}


unsigned int fast_abs(int x)
{
  int mask = x>>7;
  return (x + mask) ^ mask;
}

signed int min(signed int a, signed int b)
{
  if(a > b)
  {
    return b;
  }
  return a;
}

signed int max(signed int a, signed int b)
{
  if(a > b)
  {
    return a;
  }
  return b;
}

void init_ball()
{
  ball_pos.x = BALL_START_X;//TO_FIXED(BALL_START_X);
  ball_pos.y = BALL_START_Y;//TO_FIXED(BALL_START_Y);
  last_x = BALL_START_X;
  last_y = BALL_START_Y;

  fake_x = ball_pos.x;
  fake_y = ball_pos.y;
  ball_ym = 5;
  // ball_xm = -64;
}

void init_table()
{
  game_state = GS_GAME_SCREEN;
  // game_state = GS_FREE_BALL_MOVE;
  init_ball();
}

void check_surface_collision()
{
  signed int dx, dy, length, ny, nx, ball_dx, ball_dy, dot;
  dx = 5 - 8;
  dy = 14 - 12;

  // length = FIXED_DIV(FIXED_MUL(dx,dx) + FIXED_MUL(dy, dy), FIXED_ONE);
  length = (dx*dx) + (dy*dy);

  nx = FIXED_DIV(dy, length);
  ny = FIXED_DIV(-dx,length);

  ball_dx = ball_pos.x - 8;
  ball_dy = ball_pos.y - 12;

  dot = (FIXED_MUL(ball_xm, nx)) + (FIXED_MUL(ball_ym,ny));
  // testint = dot;
  testint = ny;
  if(dot < 0)
  {
    ball_xm = (2<<4)*-1;//FIXED_MUL(nx, dot * 2);
    ball_ym = (2<<4)*-1;//FIXED_MUL(ny, dot * 2);
  }
}

void update_ball()
{
  char triggered = 0;
  //make sure our momentum isn't exceeded
  if(ball_ym >= MAX_MOMENTUM)
  {
      ball_ym = MAX_MOMENTUM;
  }
  else if(ball_ym <= -MAX_MOMENTUM)
  {
      ball_ym = -MAX_MOMENTUM;
  }
  if(ball_xm >= MAX_MOMENTUM)
  {
    ball_xm = MAX_MOMENTUM;
  }
  else if(ball_xm <= -MAX_MOMENTUM)
  {
    ball_xm = -MAX_MOMENTUM;
  }
  //we look at precompiled table mask to see if we hit anything, this will
  //also have the angle of the thing we hit in the future, for mafs
  // check_collision();
  //if we're going faster than 1 px a frame in some cases, we can get stuck in the wall
  //since we're not updating actual pos until next frame (we just update the momentum here)
  //so just put ball back where it was and then do reflection mafs

  //updating ball pos using pretend floaties (probably wrong here)
  ball_yf += ball_ym;
  last_y = ball_pos.y;
  ball_pos.y += (ball_yf / 127);
  check_collision();

  if(box_trigger)
  {
    ball_pos.y = last_y;
    triggered = box_trigger;
  }
  ball_yf %= 127;//maybe move this outside?

  ball_xf += ball_xm;
  last_x = ball_pos.x;
  ball_pos.x += (ball_xf / 127);
  check_collision();

  if(box_trigger)
  {
    ball_pos.x = last_x;
    triggered = box_trigger;
  }
  ball_xf %= 127;

  if(triggered)
  {
    box_trigger = triggered;
    reflect();
    box_trigger = 0;
    // check_surface_collision();
  }

  //updating ball pos using pretend floaties (probably wrong here)
  // ball_yf += ball_ym;
  // last_y = ball_pos.y;
  // fake_y += (ball_yf / 127);
  // check_collision();
  // if(box_trigger == 0)
  // {
  //   ball_pos.y += (ball_yf / 127);
  //   fake_y = ball_pos.x;
  //   // ball_yf %= 127;
  // }
  // fake_y = ball_pos.y;
  // ball_yf %= 127;

  // ball_xf += ball_xm;
  // last_x = ball_pos.x;
  // fake_x += (ball_xf / 127);
  // check_collision();
  // if(box_trigger == 0)
  // {
  //   ball_pos.x += (ball_xf / 127);
  //   fake_x = ball_pos.x;
  // }
  // ball_xf %= 127;
  // fake_x = ball_pos.x;


  // check_collision();
  // if(box_trigger)
  // {
  //   ball_pos.x = last_x;
  // }

  // all below is placeholder for the bounds, will also reflect on these 90 degree angle table bounds
  // if(ball_pos.y - BALL_SIZE_Y > TABLE_BOUNDS_Y)
  // {
  //   ball_ym = (ball_ym / 3) * -1;
  // }

  // if(ball_pos.y - ball_radius < 0)
  // {
  //   ball_ym = (ball_ym / 3) * -1;
  // }

  // while(ball_pos.y - BALL_SIZE_Y > TABLE_BOUNDS_Y)
  // {
  //   ball_pos.y--;
  // }

  // while(ball_pos.y - ball_radius < 0)
  // {
  //   ball_pos.y++;
  // }

  // if(ball_pos.x-ball_radius < 0)
  // {
  //   ball_xm = ((ball_xm / 3)) * -1;
  //   last_reflect_angle = atan[(ball_ym&0b0000011110000000)>>7][(ball_xm&0b0000011110000000)>>7];
  //   while(ball_pos.x-ball_radius < 0)
  //   {
  //     ball_pos.x++;
  //   }
  // }

  // if(ball_pos.x + ball_radius > TABLE_BOUNDS_X)
  // {
  //   ball_xm = ((ball_xm / 3)) * -1;
  //   last_reflect_angle = atan[(ball_ym&0b0000011110000000)>>7][(ball_xm&0b0000011110000000)>>7];
  //   while(ball_pos.x + ball_radius > TABLE_BOUNDS_X)
  //   {
  //     ball_pos.x--;
  //   }
  // }
}

void transfer_momentum_A()
{
  if(player1_buttons & INPUT_MASK_A && player1_old_buttons != INPUT_MASK_A)
  {
    ball_ym = -80;
    // ball_xm = 411;
  }
}

void transfer_momentum_B()
{
  if(player1_buttons & INPUT_MASK_B && player1_old_buttons != INPUT_MASK_B)
  {
    // ball_ym = -411;
    ball_xm = 80;
  }
}

void check_barrier_collision(point *dpos, signed char dw, signed char dl)
{
  if(
    ball_pos.x < dpos->x + dw &&
    ball_pos.x > dpos->x &&
    ball_pos.y < dpos->y + dl &&
    ball_pos.y > dpos->y
  )
  {
    box_trigger = 1;
    // ball_xm = ((ball_xm/3)<<1)*-1;//ball_ym;
    ball_xm = ball_xm*-1;
    ball_ym = ball_ym*-1;//ball_xm;
  }
};

void check_collision()
{
  if(ball_pos.x - (ball_radius<<1) <= 0)
  {
    box_trigger = 32;
    return;
  }
  if(ball_pos.x + (ball_radius<<1) >= TABLE_BOUNDS_X)
  {
    box_trigger = 32;
    return;
  }
  push_rom_bank();
  change_rom_bank(BANK_PROG0);
  // if(table_mask[(FROM_FIXED(ball_pos.y)*127+FROM_FIXED(ball_pos.x))])
  if((box_trigger = table_mask[((ball_pos.y+8)*127)+ball_pos.x]))
  // if(table_mask[(fake_y*127)+fake_x])
  {
    box_trigger = table_mask[((ball_pos.y+8)*127)+ball_pos.x];
    // game_state = GS_PAUSE;
  }
  else
  {
    box_trigger = 0;
  }
  pop_rom_bank();
}

void pull()
{
  // if(ball_ym + FALL_SPEED > 0 && ball_ym + FALL_SPEED < 127)
  // {
  if(ball_ym >= 0 && (ball_ym + (signed char)FALL_SPEED) > 127)
  {
    return;
  }
  ball_ym += FALL_SPEED;
  // ball_xm -= FIXED_MUL(ball_xm,2);
  // }
  // ball_ym += (signed char)FALL_SPEED;
  // if(ball_pos.x < 64)
  // {
  //   ball_xm += 1;
  // }
  // else if(ball_pos.x > 64)
  // {
  //   ball_xm += -1;
  // }
}

// def dot_product(p1,p2)
//   return (p1.x * p2.x) + (p1.y * p2.y)
// end

// def reflect(velocity,p1)
//   dot = dot_product(velocity,p1)
//   # puts dot
//   x = velocity.x - 2 * dot * p1.x
//   y = velocity.y - 2 * dot * p1.y
//   Point.new(x,y)
// end

void apply_physics()
{
  // ball_ym += FALL_SPEED;
  pull();
  if(ball_pos.y - 15 >= TABLE_BOUNDS_Y)
  {
    ball_ym = 0;
    // ball_y--;
  }
  // last_x = ball_pos.x;
  // last_y = ball_pos.y;
  transfer_momentum_A();
  transfer_momentum_B();
  update_ball();
  // check_collision();
  // if(box_trigger)
  // {
  //   signed int dx, dy, length, ny, nx, ball_dx, ball_dy, dot;
  //   dx = 5 - 8;
  //   dy = 14 - 12;
  //   //-3/2
  //   //9 + 2 = 11 / 8
  //   length = FIXED_DIV((FIXED_MUL(dx,dx)) + (FIXED_MUL(dy, dy)), FIXED_ONE);
  //   if(length < 0)
  //   {
  //     length = 1;
  //   }

  //   nx = FIXED_DIV(dy, length);
  //   ny = FIXED_DIV(-dx,length);

  //   ball_dx = last_x - 8;
  //   ball_dy = last_y - 12;

  //   dot = (FIXED_MUL(ball_xm, nx)) + (FIXED_MUL(ball_ym,ny));
  //   // freei = dot;
  //   freei = length;
  //   if(dot < 0)
  //   {
  //     // freei = d
  //     ball_pos.x = last_x;
  //     ball_pos.y = last_y;
  //     // freei = -ball_ym;
  //     ball_xm = FIXED_MUL(nx, dot * 2);
  //     //FIXED_MUL(ny,dot*2);
  //     ball_ym = FIXED_MUL(ny, dot * 2);
  //     // freei = ball_ym;
  //     box_trigger = 0;

  //   }
  // }

  // check_surface_collision();
  // freei = FIXED_MUL(16,DAMPING);
  // freei = FIXED_MUL(244,7);
  // apply_vertical_force();
  // apply_horizontal_force();
  // while(
  //   fast_abs(ball_xf+ball_xm) > 256 ||
  //   fast_abs(ball_yf+ball_ym) > 256
  // )
  // {

  // }
}

void do_pinball()
{
  apply_physics();
}

void move_ball()
{
  if(player1_buttons & INPUT_MASK_LEFT)
  {
    ball_pos.x--;
    // ball_xm -= 64;
  }
  if(player1_buttons & INPUT_MASK_RIGHT)
  {
    ball_pos.x++;
    // ball_xm += 127;
  }
  if(player1_buttons & INPUT_MASK_UP)
  {
    ball_pos.y--;
    // ball_ym -= 64;
  }
  if(player1_buttons & INPUT_MASK_DOWN)
  {
    // ball_ym += 64;
    ball_pos.y++;
  }

  // if(ball_xm >= 127)
  // {
  //   ball_pos.x++;
  //   ball_xm = 0;
  // }
  // if(ball_xm <= -127)
  // {
  //   ball_pos.x--;
  //   ball_xm = 0;
  // }
  // if(ball_ym >= 127)
  // {
  //   ball_pos.y++;
  //   ball_ym = 0;
  // }
  // if(ball_ym <= -127)
  // {
  //   ball_pos.y--;
  //   ball_ym = 0;
  // }
  check_collision();
  if(player1_buttons & INPUT_MASK_A)
  {
    game_state = GS_GAME_SCREEN;
    // ball_ym = 0;
    // ball_xm = 0;
  }
}

// if(player1_buttons & INPUT_MASK_RIGHT)
// {
//   ball_x++;
// }
// if(player1_buttons & INPUT_MASK_LEFT)
// {
//   ball_x--;
// }
// if(player1_buttons & INPUT_MASK_UP)
// {
//   ball_y--;
// }
// if(player1_buttons & INPUT_MASK_DOWN)
// {
//   ball_y++;
// }

// put_number(min(4,12) == 4,2,2,5);
// put_number(min(44,16) == 16,2,2,6);
// put_number(max(22,18) == 22,2,2,7);
// put_number(max(37,19) == 37,2,2,8);

// put_number(min(-4,-12) == -12,2,2,10);
// put_number(min(-44,-16) == -44,2,2,11);
// put_number(max(-22,-18) == -18,2,2,12);
// put_number(max(-37,-19) == -19,2,2,13);


// if(ball_xm >= MAX_MOMENTUM)
//   {
//     ball_xm = MAX_MOMENTUM;
//   }
//   else if(ball_xm <= -MAX_MOMENTUM)
//   {
//     ball_xm = -MAX_MOMENTUM;
//   }


  // if(ball_ym >= MAX_MOMENTUM)
  //   {
  //     ball_ym = MAX_MOMENTUM;
  //   }
  //   else if(ball_ym <= -MAX_MOMENTUM)
  //   {
  //     ball_ym = -MAX_MOMENTUM;
  //   }
