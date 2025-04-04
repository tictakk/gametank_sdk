#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gen/assets/guy.h"
#include "gt/gfx/gfx_sys.h"
#include "gt/gfx/sprites.h"
#include "gt/feature/text/text.h"
#include "gen/assets/maps.h"
// #include "gen/assets/pinball.h"
#include "gt/input.h"
#include "../include/global.h"
#include "../include/pinball.h"

#define GS_MAIN_SCREEN 0
#define GS_CHARACTER_SELECT_SCREEN 1
#define GS_TABLE_SELECT_SCREEN 2
#define GS_GAME_SCREEN 3
#define GS_FREE_BALL_MOVE 4
#define GS_PAUSE 5

#define FIXED_SHIFT 8
#define FROM_FIXED(x) ((x) >> FIXED_SHIFT)

char camera_x = 0, camera_y = 0, game_state;
// char ball_x = 0, ball_y = 0;
unsigned char camera_sub_counter = 0;
char dx = 1, dy = 1;
int pos[4];
SpriteSlot guy;
SpriteSlot txt;
SpriteSlot map;
SpriteSlot ball;
char loop_divider = 0;
char frame = 0;
char mubee = 0 ;
char pad = 0;
char pad_held = 0;

signed int abs_test = -255, mask = 0;

//font_slot needs to be visible
unsigned int pow(unsigned int base, unsigned int exp)
{
  unsigned int e = exp;
  unsigned int result = 1;
  if(exp == 0)
  {
    return 1;
  }
  while(e-- > 0)
  {
    result = result * base;
  }
  return result;
}

void put_char(unsigned char c, unsigned char x, unsigned char y)
{
  char cr = c;
  queue_draw_sprite(x<<3,y<<3,8,8,(cr&15)<<3,(cr/16)<<3,font_slot);
}

void put_string(char *s, unsigned char x, unsigned char y)
{
  unsigned char i=0;
  unsigned char i_x=0;

  while(s[i++] != 0)
  {
    put_char(s[i-1],x+(i_x++),y);
  }
}

void put_number(int number, int decimals, unsigned char x, unsigned char y)
{
  unsigned int result = 0;
  unsigned char i = decimals;

  while(i > 0)
  {
    result = number / pow(10,(i-1));
    result %= 10;
    put_char((result+48),(x-i),y);
    i--;
  }
}

void put_number_signed(int number, unsigned int decimals, unsigned char x, unsigned char y)
{
  if(number < 0)
  {
    put_char('-',x-decimals-1,y);
    put_number(-number,decimals,x,y);
  }
  else
  {
    put_number(number,decimals,x,y);
  }
}

int main () {

    init_graphics();

    map = allocate_sprite(&ASSET__maps__table_bmp_load_list);
    ball = allocate_sprite(&ASSET__maps__ball_bmp_load_list);


    text_load_font();
    text_init();
    game_state = GS_MAIN_SCREEN;

    while (1)
    {                          //  Run forever
      update_inputs();

      queue_clear_screen(46);
      if(game_state == GS_MAIN_SCREEN)
      {
        signed char x = 71;//43;
        signed char y = 43;//71;
        queue_draw_sprite(0,0,127,127,0,0,map);
        // put_string("ready",2,5);
        put_number_signed(get_reflect_angle(x,y), 3, 15, 6);
        put_number_signed(63-(get_reflect_angle(x,y)+31), 3, 15, 7);
        // put_number_signed(get_angle(101,18), 3, 15, 8);
        if(player1_buttons & INPUT_MASK_START)
        {
          init_table();
        }
      }
      else if(game_state == GS_GAME_SCREEN)
      {
        if(player1_buttons & INPUT_MASK_START && player1_old_buttons != INPUT_MASK_START)
        {
          game_state = GS_PAUSE;
        }
        queue_draw_sprite(0,0,127,127,0,0,map);
        // queue_draw_sprite(barriers[0].pos.x,barriers[0].pos.y,10,10,12,0,ball);
        // queue_draw_sprite(ball_pos.x-ball_radius,ball_pos.y-ball_radius,9,9,0,0,ball);
        // queue_draw_sprite(FROM_FIXED(ball_pos.x)-ball_radius,FROM_FIXED(ball_pos.y)-ball_radius,9,9,0,0,ball);
        queue_draw_sprite(ball_pos.x-ball_radius,ball_pos.y-ball_radius,9,9,0,0,ball);
        // if(loop_divider++ == 5)
        // {
          loop_divider = 0;
          do_pinball();
        // }
        // mask = abs_test >> 15;
        // put_number_signed(fast_abs(ball_xm),5,15,5);
        put_number_signed(testint, 5, 15, 5);
        put_number_signed(ball_xm, 3, 15, 6);
        put_number_signed(ball_ym, 3, 15, 7);
      }
      else if(game_state == GS_PAUSE)
      {
        queue_draw_sprite(0,0,127,127,0,0,map);
        // queue_draw_sprite(FROM_FIXED(ball_pos.x)-ball_radius,FROM_FIXED(ball_pos.y)-ball_radius,9,9,0,0,ball);

        if(player1_buttons & INPUT_MASK_START && player1_old_buttons != INPUT_MASK_START)
        {
          game_state = GS_GAME_SCREEN;
          // box_trigger = 0;
        }
        queue_draw_sprite(0,0,127,127,0,0,map);
        queue_draw_sprite(ball_pos.x-ball_radius,ball_pos.y-ball_radius,9,9,0,0,ball);
        // if(loop_divider++ == 5)
        // {
          // loop_divider = 0;
          // do_pinball();
        // }
        // mask = abs_test >> 15;
        // put_number_signed(fast_abs(ball_xm),5,15,5);
        put_number_signed(testint, 5, 15, 5);
        put_number_signed(ball_xm, 3, 15, 6);
        put_number_signed(ball_ym, 3, 15, 7);
      }
      else if(game_state == GS_FREE_BALL_MOVE)
      {
        move_ball();
        queue_draw_sprite(0,0,127,127,0,0,map);
        queue_draw_sprite(ball_pos.x-ball_radius,ball_pos.y-ball_radius,9,9,0,0,ball);
        // queue_draw_sprite(FROM_FIXED(ball_pos.x)-ball_radius,FROM_FIXED(ball_pos.y)-ball_radius,9,9,0,0,ball);
        put_number(box_trigger, 2, 15, 1);
      }

      queue_clear_border(0);
      await_draw_queue();
      await_vsync(1);
      flip_pages();
    }

  return (0);                                     //  We should never get here!
}
