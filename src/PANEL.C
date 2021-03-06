// THOR - The God of Thunder
// See LICENSE for details

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "res_man.h"
#include "DEFINE.H"
#include "PROTO.H"

#define STAT_COLOR 206

extern ACTOR *thor;
extern THOR_INFO thor_info;
extern uint8_t *tmp_buff;
extern uint8_t objects[NUM_OBJECTS][262];
extern uint16_t page[3];
extern volatile uint8_t key_flag[100];
extern int16_t key_fire, key_up, key_down, key_left, key_right, key_magic, key_select;
extern uint16_t display_page, draw_page;
extern uint8_t *bg_pics;
extern int16_t restore_screen;
extern uint8_t hampic[4][262];
extern volatile uint16_t timer_cnt, extra_cnt;
extern uint8_t level_type, slow_mode;
extern struct sup setup;
extern int16_t music_flag, sound_flag, pcsound_flag, boss_active;
extern char *options_yesno[3];
char *options_onoff[] = {"On", "Off", NULL};
char *options_sound[] = {"None", "PC Speaker", "Digitized", NULL};
char *options_skill[] = {"Easy Enemies", "Normal Enemies", "Tough Enemies", NULL};
char *options_slow[] = {"On  (slow computer)", "Off (fast computer)", NULL};
char *options_menu[] = {"Sound/Music", "Skill Level", "Save Game", "Load Game",
                           "Die", "Turbo Mode", "Help", "Quit", NULL};
char *options_quit[] = {"Continue Game", "Quit to Opening Screen", "Quit to DOS", NULL};
extern uint8_t *scr;
extern uint8_t last_setup[32];
extern int16_t exit_flag;
extern uint8_t cheat;
//===========================================================================
//void status_panel(void){
//int16_t i;
//int16_t c[]={23,25,27,29,20};

//for(i=0;i<5;i++) xfillrectangle(0+i,0+i,320-i,48-i,PAGES,c[i]);
//xprint(8,6,"Health",PAGES,6);
//xfillrectangle(64,6,165,14,PAGES,0);
//display_health();
//xprint(8,16,"Jewels",PAGES,6);
//xprint(24,28,"Keys",PAGES,6);

//}
//===========================================================================
void display_health(void)
{
  int16_t b;

  b = 59 + thor->health;
  xfillrectangle(59, 200, b, 204, PAGES, 32);
  xfillrectangle(b, 200, 209, 204, PAGES, STAT_COLOR);
}
//===========================================================================
void display_magic(void)
{
  int16_t b;

  b = 59 + thor_info.magic;
  xfillrectangle(59, 212, b, 216, PAGES, 96);
  xfillrectangle(b, 212, 209, 216, PAGES, STAT_COLOR);
}
//===========================================================================
void display_jewels(void)
{
  char s[21];
  int16_t x, l;

  sprintf(s, "%d", thor_info.jewels);
  l = strlen(s);

  if (l == 1)
    x = 70;
  else if (l == 2)
    x = 66;
  else
    x = 62;

  xfillrectangle(59, 224, 85, 234, PAGES, STAT_COLOR);
  xprint(x, 224, s, PAGES, 14);
}
//===========================================================================
void display_score(void)
{
  char s[21];
  int16_t x, l;

  sprintf(s, "%d", thor_info.score);

  l = strlen(s);
  x = 276 - (l * 8);

  xfillrectangle(223, 224, 279, 234, PAGES, STAT_COLOR);
  xprint(x, 224, s, PAGES, 14);
}
//===========================================================================
void display_keys(void)
{
  char s[21];
  int16_t x, l;

  sprintf(s, "%d", thor_info.keys);
  l = strlen(s);

  if (l == 1)
    x = 150;
  else if (l == 2)
    x = 146;
  else
    x = 142;

  xfillrectangle(139, 224, 165, 234, PAGES, STAT_COLOR); //215
  xprint(x, 224, s, PAGES, 14);
}
//===========================================================================
void display_item(void)
{

  xfillrectangle(280, 200, 296, 216, PAGES, STAT_COLOR);
  if (thor_info.item)
  {
    if (thor_info.item == 7)
      xfput(282, 200, PAGES, (uint8_t *)objects[thor_info.object + 10]);
    else
      xfput(282, 200, PAGES, (uint8_t *)objects[thor_info.item + 25]);
  }
}
//===========================================================================
int16_t init_status_panel(void)
{
  uint8_t *sp;

  sp = res_falloc_read("STATUS");
  if (!sp)
    return 0;

  xfarput(0, 192, PAGES, sp);
  //xfillrectangle(61,32,87,42,PAGES,STAT_COLOR);
  //xfillrectangle(183,32,209,42,PAGES,STAT_COLOR);
  //xfillrectangle(222,16,272,33,PAGES,STAT_COLOR);
  display_item();
  free(sp);
  return 1;
}
//===========================================================================
void add_jewels(int16_t num)
{
  int16_t n;

  n = thor_info.jewels + num;
  if (n > 999)
    n = 999;
  else if (n < 0)
    n = 0;
  thor_info.jewels = n;
  display_jewels();
}
//===========================================================================
void add_score(int16_t num)
{
  int32_t n;

  n = thor_info.score + (int32_t)num;
  if (n > 999999l)
    n = 999999l;
  else if (n < 0)
    n = 0;
  thor_info.score = n;
  display_score();
}
//===========================================================================
void add_magic(int16_t num)
{
  int16_t n;

  n = thor_info.magic + num;
  if (n > 150)
    n = 150;
  else if (n < 0)
    n = 0;
  thor_info.magic = n;
  display_magic();
}
//===========================================================================
void add_health(int16_t num)
{
  int16_t n;

  n = thor->health + num;
  if (n > 150)
    n = 150;
  else if (n < 0)
    n = 0;
  thor->health = n;
  display_health();
  if (thor->health < 1)
    exit_flag = 2;
}
//===========================================================================
void add_keys(int16_t num)
{
  int16_t n;

  n = thor_info.keys + num;
  if (n > 99)
    n = 99;
  else if (n < 0)
    n = 0;
  thor_info.keys = n;
  display_keys();
}
//===========================================================================
void fill_health(void)
{

  //while(thor->health<150){
  //   if(!sound_playing()) play_sound(ANGEL,1);
  add_health(150);
  //   pause(4);
  //}
}
//===========================================================================
void fill_magic(void)
{

  //while(thor_info.magic<150){
  //   if(!sound_playing()) play_sound(ANGEL,1);
  add_magic(150);
  //   pause(4);
  //}
}
//===========================================================================
void fill_score(int16_t num)
{

  while (num)
  {
    num--;
    play_sound(WOOP, 1);
    add_score(1000);
    pause(8);
  }
}
//===========================================================================
void score_for_inv(void)
{

  while (thor->health)
  {
    thor->health--;
    play_sound(WOOP, 1);
    add_health(-1);
    add_score(10);
    pause(8);
  }
  while (thor_info.magic)
  {
    thor_info.magic--;
    play_sound(WOOP, 1);
    add_magic(-1);
    add_score(10);
    pause(8);
  }
  while (thor_info.jewels)
  {
    thor_info.jewels--;
    play_sound(WOOP, 1);
    add_jewels(-1);
    add_score(10);
    pause(8);
  }
}
//===========================================================================
void boss_status(int16_t health)
{
  int16_t rep, i, c;

  if (health == -1)
  {
    REPEAT(3)
    {
      xfillrectangle(304, 2, 317, 81, page[rep], 0);
      xfillrectangle(305, 3, 316, 80, page[rep], 28);
      xfillrectangle(306, 4, 315, 79, page[rep], 26);
      xfillrectangle(307, 5, 314, 78, page[rep], 24);
    }
    health = 100;
  }
  for (i = 10; i > 0; i--)
  {
    if (i * 10 > health)
      c = 0;
    else
      c = 32;
    REPEAT(3)
    {
      xfillrectangle(308, 7 + (7 * (10 - i)), 313, 13 + (7 * (10 - i)), page[rep], c);
    }
  }
}
//===========================================================================
int16_t select_option(char *option[], const char *title, int16_t ipos)
{
  int16_t num_opts, x1, y1, x2, y2, w, h;
  int16_t s, i, pic, pos, key, y, kf, ret;
  uint16_t pg;
  char **op;

  show_all_actors();
  num_opts = 0;
  w = strlen(title);
  op = option;
  while (*op)
  {
    if (strlen(*op) > w)
      w = strlen(*op);
    num_opts++;
    op++;
  }
  if (w & 1)
    w++;
  w = (w * 8) + 32;
  s = w / 16;
  h = (num_opts * 16) + 32;
  x1 = (320 - w) / 2;
  x2 = (x1 + w) - 1;
  y1 = (192 - h) / 2;
  y2 = (y1 + h) - 1;
  if (x1 & 1)
    x1++;
  if (x2 & 1)
    x2++;

  pg = display_page;

  xfillrectangle(x1, y1, x2, y2, pg, 215);

  xfput(x1 - 16, y1 - 16, pg, (uint8_t *)(bg_pics + (192 * 262)));
  xfput(x2, y1 - 16, pg, (uint8_t *)(bg_pics + (193 * 262)));
  xfput(x1 - 16, y2, pg, (uint8_t *)(bg_pics + (194 * 262)));
  xfput(x2, y2, pg, (uint8_t *)(bg_pics + (195 * 262)));
  for (i = 0; i < s; i++)
  {
    xfput(x1 + (i * 16), y1 - 16, pg, (uint8_t *)(bg_pics + (196 * 262)));
    xfput(x1 + (i * 16), y2, pg, (uint8_t *)(bg_pics + (197 * 262)));
  }
  for (i = 0; i < (num_opts + 2); i++)
  {
    xfput(x1 - 16, y1 + (i * 16), pg, (uint8_t *)(bg_pics + (198 * 262)));
    xfput(x2, y1 + (i * 16), pg, (uint8_t *)(bg_pics + (199 * 262)));
  }
  s = strlen(title) * 8;
  i = (320 - s) / 2;
  xprint(i, y1 + 4, title, pg, 54);

  op = option;
  for (i = 0; i < num_opts; i++)
  {
    xprint(x1 + 32, (y1 + 28) + (i * 16), *op, pg, 14);
    op++;
  }

  pos = ipos;
  pic = 0;
  kf = 0;
  y = y1 + 24 + (pos * 16);
  wait_not_response();
  wait_not_key(UP);
  wait_not_key(DOWN);
  extra_cnt = 0;
  ret = 0;

  while (1)
  {
    if (extra_cnt > 15)
    {
      kf = 0;
      extra_cnt = 0;
    }
    xfillrectangle(x1 + 8, y, x1 + 24, y + 16, pg, 215);
    y = y1 + 24 + (pos * 16);
    xput(x1 + 8, y, pg, hampic[pic]);
    pic++;
    if (pic > 3)
    {
      pic = 0;
    }
    timer_cnt = 0;
    // TODO this looks like it had to do with an interrupt to work with VGA.
    // while (timer_cnt < 10)
    //   rotate_pal();
    key = get_response();
    if (key == ENTER || key == SPACE || key == key_fire || key == key_magic)
    {
      hammer_smack(x1 + 8, y);
      ret = pos + 1;
      break;
    }
    if (key == ESC)
      break;
    if (key_flag[UP])
      key = UP;
    else if (key_flag[DOWN])
      key = DOWN;
    if (key == UP || key == DOWN)
    {
      if (!kf)
      {
        if (key == UP)
        {
          pos--;
          if (pos < 0)
            pos = num_opts - 1;
        }
        if (key == DOWN)
        {
          pos++;
          if (pos >= num_opts)
            pos = 0;
        }
        play_sound(WOOP, 1);
        kf = 1;
        extra_cnt = 0;
      }
    }
    else
    {
      kf = 0;
    }
  }
  wait_not_response();
  restore_screen = 1;
  return ret;
}
//===========================================================================
int16_t option_menu(void)
{

  return select_option(options_menu, "Options Menu", 0);
}
//===========================================================================
int16_t ask_exit(void)
{

  return select_option(options_quit, "Quit Game?", 0);
}
//===========================================================================
int16_t select_sound(void)
{
  int16_t ret, sel;

  sel = 0;
  if (setup.pc_sound)
    sel = 1;
  else if (setup.dig_sound)
    sel = 2;

  ret = select_option(options_sound, "  Set Sound  ", sel);
  d_restore();
  if (!ret)
    return 0;
  if (ret == 1)
  {
    setup.pc_sound = 0;
    setup.dig_sound = 0;
  }
  if (ret == 2)
  {
    setup.pc_sound = 1;
    setup.dig_sound = 0;
  }
  if (ret == 3)
  {
    if (!sound_flag)
    {
      odin_speaks(2001, 0);
      return 1;
    }
    setup.pc_sound = 0;
    setup.dig_sound = 1;
  }
  memcpy(last_setup, &setup, 32);
  return 1;
}
//===========================================================================
int16_t select_music(void)
{
  int16_t ret;

  if (!music_flag)
    return 1;
  //if(!setup.music) return 1;
  //if(!music_flag){
  //  odin_speaks(2002,0);
  //  return 1;
  //}
  ret = select_option(options_onoff, "Set Music", 1 - setup.music);
  if (!ret)
    return 0;
  if (ret == 1)
  {
    if (setup.music)
      return 1;
    setup.music = 1;
    if (!boss_active)
      music_play(level_type, 1);
    else
      music_play(4, 1);
  }
  else if (ret == 2)
  {
    music_pause();
    setup.music = 0;
  }
  memcpy(last_setup, &setup, 32);
  return 1;
}
//===========================================================================
int16_t select_slow(void)
{
  int16_t ret;

  ret = select_option(options_slow, "Fast Mode", 1 - slow_mode);
  if (!ret)
    return 0;
  if (ret == 1)
    slow_mode = 1;
  if (ret == 2)
    slow_mode = 0;
  setup.speed = slow_mode;
  memcpy(last_setup, &setup, 32);
  return 1;
}
//===========================================================================
int16_t select_scroll(void)
{
  int16_t ret;

  ret = select_option(options_yesno, "Scroll Between Screens?", 1 - setup.scroll_flag);
  if (!ret)
    return 0;
  if (ret == 1)
    setup.scroll_flag = 1;
  if (ret == 2)
    setup.scroll_flag = 0;
  memcpy(last_setup, &setup, 32);
  return 1;
}
//===========================================================================
void select_fastmode(void)
{

  if (select_slow())
    select_scroll();
}
//===========================================================================
void select_skill(void)
{
  int16_t ret, sel;

  sel = setup.skill;
  ret = select_option(options_skill, "  Set Skill Level ", sel);
  if (!ret)
    return;
  if (ret)
    setup.skill = ret - 1;
  memcpy(last_setup, &setup, 32);
}
//===========================================================================
void hammer_smack(int16_t x, int16_t y)
{
  int16_t i;

  for (i = 0; i < 4; i++)
  {
    xfillrectangle(x - 4, y - 4, x + 16, y + 16, display_page, 215);
    x += 2;
    xput(x, y, display_page, hampic[0]);
    pause(3);
  }

  play_sound(CLANG, 1);

  for (i = 0; i < 4; i++)
  {
    xfillrectangle(x, y, x + 16, y + 16, display_page, 215);
    x -= 2;
    xput(x, y, display_page, hampic[0]);
    pause(3);
  }
}
