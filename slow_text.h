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
  //font slow needs to be made visible and this relies on the draw
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
