// THOR - The God of Thunder
// See LICENSE for details

//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include "DEFINE.H"
#include "PROTO.H"
#include "res_man.h"
#include "utility.h"
#include "sdl_man.h"

//===========================================================================
extern uint8_t *bg_pics;
extern int16_t warp_flag;

extern LEVEL scrn;
extern uint8_t *scrnp;
extern uint8_t *sd_data;
extern int16_t current_level;
extern uint16_t display_page, draw_page;
extern struct sup setup;
extern THOR_INFO thor_info;
extern ACTOR actor[MAX_ACTORS];
extern ACTOR *thor;
extern volatile uint8_t key_flag[100];
extern uint8_t *dig_sound[10];
extern int16_t restore_screen;
extern int16_t key_fire, key_up, key_down, key_left, key_right, key_magic, key_select;
extern uint8_t *tmp_buff;
extern uint8_t text[94][72];
extern volatile uint16_t timer_cnt;
extern uint8_t *bleep;
extern int16_t last_oracle;
extern uint8_t objects[NUM_OBJECTS][262];
extern int16_t lightning_used, tornado_used, thunder_flag;
extern int16_t hourglass_flag, shield_on, bomb_flag;
extern int16_t joystick;
extern uint8_t level_type;
extern uint8_t *song;
extern uint8_t music_current;
extern uint8_t odin[4][262];
extern uint16_t page[3];
extern int16_t boss_dead, boss_active;
extern ACTOR explosion;
extern uint8_t pge, slow_mode, scroll_flag;
extern volatile uint16_t magic_cnt;
extern int16_t exit_flag;
extern uint8_t object_map[240];
extern uint8_t object_index[240];
extern uint8_t warp_scroll;
extern uint8_t startup;
extern uint8_t last_setup[32];
extern uint8_t auto_load;
extern uint8_t area;
extern uint8_t dialog_color[16];

char *object_names[] = {"Shrub", "Child's Doll", "UNUSED", "FUTURE",
                           "FUTURE", "FUTURE", "FUTURE", "FUTURE", "FUTURE",
                           "FUTURE", "FUTURE", "FUTURE", "FUTURE", "FUTURE",
                           "FUTURE"};
char *item_name[] = {"Enchanted Apple", "Lightning Power",
                        "Winged Boots", "Wind Power",
                        "Amulet of Protection", "Thunder Power"};
//===========================================================================
void build_screen(uint16_t pg)
{
  // TODO restore this functionality once graphics is revisited
  return;
  int16_t x, y;

  xfillrectangle(0, 0, 320, 192, pg, 0);
  for (y = 0; y < 12; y++)
  {
    for (x = 0; x < 20; x++)
    {
      if (scrn.icon[y][x] != 0)
      {
        xfput(x * 16, y * 16, pg, (uint8_t *)(bg_pics + ((size_t)scrn.bg_color * 262)));
        xfput(x * 16, y * 16, pg, (uint8_t *)(bg_pics + ((size_t)scrn.icon[y][x] * 262)));
      }
    }
  }
}
//===========================================================================
void show_level(int16_t new_level)
{
  // TODO restore this functionality once graphics is revisited
  return;
  int16_t f, save_d;

  boss_active = 0;
  if (!shield_on)
    actor[2].used = 0;
  bomb_flag = 0;

  save_d = thor->dir;
  if (scrn.icon[thor->center_y][thor->center_x] == 154)
    thor->dir = 0;

  //copy object data to main screen buffer
  memmove(
    sd_data + ((size_t)current_level * 512) + 322, 
    &scrn.static_obj,
    130
  );

  memmove(
    sd_data + ((size_t)current_level * 512),
    &scrn,
    sizeof(LEVEL)
  );

  memmove(
    sd_data + ((size_t)new_level * 512),
    &scrn,
    sizeof(LEVEL)
  );

  scrnp = (uint8_t *)&scrn;
  level_type = scrn.type;

  thor->next = 0;
  xdisplay_actors(&actor[MAX_ACTORS - 1], PAGE2);

  build_screen(draw_page);

  show_objects(new_level, draw_page);
  show_enemies();

  if (scrn.icon[thor->center_y][thor->center_x] == 154)
    thor->dir = 0;
  xdisplay_actors(&actor[MAX_ACTORS - 1], draw_page);
  thor->dir = save_d;

  if (warp_flag)
    current_level = new_level - 5; //force phase
  warp_flag = 0;
  if (warp_scroll)
  {
    warp_scroll = 0;
    if (thor->dir == 0)
      current_level = new_level + 10;
    else if (thor->dir == 1)
      current_level = new_level - 10;
    else if (thor->dir == 2)
      current_level = new_level + 1;
    else if (thor->dir == 3)
      current_level = new_level - 1;
  }
  if (!setup.scroll_flag)
    current_level = new_level; //force no scroll

  if (music_current != level_type)
    music_pause();
  switch (new_level - current_level)
  {
  case 0:
    xshowpage(draw_page);
    xcopyd2d(0, 0, 320, 192, 0, 0, draw_page, display_page, 320, 320);
    break;
  case 1:
  {
    scroll_level_right();
    break;
  }
  case 10:
  {
    scroll_level_down();
    break;
  }
  case -1:
  {
    scroll_level_left();
    break;
  }
  case -10:
  {
    scroll_level_up();
    break;
  }
  default:
    phase_level();
  }
  build_screen(PAGE2);
  show_objects(new_level, PAGE2);
  current_level = new_level;
  thor_info.last_health = thor->health;
  thor_info.last_magic = thor_info.magic;
  thor_info.last_jewels = thor_info.jewels;
  thor_info.last_keys = thor_info.keys;
  thor_info.last_score = thor_info.score;
  thor_info.last_item = thor_info.item;
  thor_info.last_screen = current_level;
  thor_info.last_icon = ((thor->x + 8) / 16) + (((thor->y + 14) / 16) * 20);
  thor_info.last_dir = thor->dir;
  thor_info.last_inventory = thor_info.inventory;
  thor_info.last_object = thor_info.object;
  thor_info.last_object_name = thor_info.object_name;
  memcpy(last_setup, &setup, 32);

  f = 1;
  if (GAME1 && new_level == BOSS_LEVEL1)
  {
    if (!setup.boss_dead[0])
    {
      if (!auto_load)
        boss_level1();
      f = 0;
    }
  }
  if (startup)
    f = 0;
  if (f)
    music_play(level_type, 0);
}
//===========================================================================
void scroll_level_left(void)
{
  int16_t i;

  for (i = 10; i > 0; i--)
  {
    xshowpage(display_page);
    xcopyd2d(0, 0, (i * 32) - 32, 192, ((11 - i) * 32), 0,
             PAGE2, display_page, 320, 320);
    xcopyd2d((i * 32) - 32, 0, 320, 192, 0, 0,
             draw_page, display_page, 320, 320);
  }
}
//===========================================================================
void scroll_level_up(void)
{
  int16_t i;

  for (i = 12; i > 0; i--)
  {
    xshowpage(display_page);
    xcopyd2d(0, 0, 320, (i * 16 - 16), 0, (13 - i) * 16,
             PAGE2, display_page, 320, 320);
    xcopyd2d(0, (i * 16) - 16, 320, 192, 0, 0,
             draw_page, display_page, 320, 320);
  }
}
//===========================================================================
void scroll_level_right(void)
{
  int16_t i;

  for (i = 10; i > 0; i--)
  {
    xshowpage(display_page);
    xcopyd2d((11 - i) * 32, 0, 320, 192, 0, 0,
             PAGE2, display_page, 320, 320);
    xcopyd2d(0, 0, (11 - i) * 32, 192, (i * 32) - 32, 0,
             draw_page, display_page, 320, 320);
  }
}
//===========================================================================
void scroll_level_down(void)
{
  int16_t i;

  for (i = 12; i > 0; i--)
  {
    xshowpage(display_page);
    xcopyd2d(0, (13 - i) * 16, 320, 192, 0, 0,
             PAGE2, display_page, 320, 320);
    xcopyd2d(0, 0, 320, (13 - i) * 16, 0, (i * 16) - 16,
             draw_page, display_page, 320, 320);
  }
}
//===========================================================================
void phase_level(void)
{
  int16_t r, cnt;
  uint8_t done[240];

  memset(done, 0, 240);
  cnt = 0;

  while (cnt < 240)
  {
    r = rnd(240);
    while (done[r])
    {
      r++;
      if (r > 239)
        r = 0;
    }
    cnt++;
    done[r] = 1;
    copy_bg_icon(r, draw_page, display_page);
    delay(2);
  }
}
//===========================================================================
void copy_bg_icon(int16_t num, uint16_t src_page, uint16_t dst_page)
{
  int16_t x, y;

  y = (num / 20) * 16;
  x = (num % 20) * 16;
  xcopyd2d(x, y, x + 16, y + 16, x, y, src_page, dst_page, 320, 320);
}
//===========================================================================
int16_t odin_speaks(int16_t index, int16_t item)
{

  execute_script((int32_t)index, (uint8_t *)odin);
  if (!thor->health)
  {
    thor->show = 0;
    exit_flag = 2;
  }
  item = item;
  return 1;
}
//===========================================================================
int16_t actor_speaks(ACTOR *actr, int16_t index, int16_t item)
{
  uint8_t *p;
  char str[21];
  char s[21];
  int16_t v;
  uint8_t *pic;
  int32_t lind;

  if (actr->type != 4)
    return 0;
  v = parse_decimal_int16_t(actr->name);
  if (v < 1 || v > 20)
    return 0;
  strcpy(str, "FACE");
  sprintf(s, "%d", v);
  strcat(str, s);

  p = (uint8_t*)malloc(1048);
  if (!p)
    return 0;

  if (res_read(str, (uint8_t *)p) < 0)
    pic = (uint8_t *)odin;
  else
    pic = p;

  lind = (int32_t)current_level;
  lind = lind * 1000;
  lind += (int32_t)actr->actor_num;
  execute_script(lind, pic);
  if (!thor->health)
  {
    thor->show = 0;
    exit_flag = 2;
  }
  free(p);
  d_restore();
  index = index;
  item = item;
  return 1;
}
//===========================================================================
int16_t display_speech(int16_t item, uint8_t *pic, int16_t tf)
{
  int16_t l, x, color, lc, pn, pc, key;
  uint16_t pg;
  uint8_t *p;
  uint16_t magic_str;
  uint8_t ch;

  magic_str = magic_cnt;

  pg = display_page;
  last_oracle = current_level;

  xfillrectangle(48, 64, 273, 145, pg, 215);
  xfput(32, 48, pg, (uint8_t *)(bg_pics + (192 * 262)));
  xfput(272, 48, pg, (uint8_t *)(bg_pics + (193 * 262)));
  xfput(32, 144, pg, (uint8_t *)(bg_pics + (194 * 262)));
  xfput(272, 144, pg, (uint8_t *)(bg_pics + (195 * 262)));
  for (l = 0; l < 14; l++)
  {
    xfput(48 + (l * 16), 48, pg, (uint8_t *)(bg_pics + (196 * 262)));
    xfput(48 + (l * 16), 144, pg, (uint8_t *)(bg_pics + (197 * 262)));
  }
  for (l = 0; l < 5; l++)
  {
    xfput(32, 64 + (l * 16), pg, (uint8_t *)(bg_pics + (198 * 262)));
    xfput(272, 64 + (l * 16), pg, (uint8_t *)(bg_pics + (199 * 262)));
  }

  p = tmp_buff;
  x = 40;
  lc = 0;
  l = 0;
  color = 14;
  pn = 0;
  pc = 0;
  xput(152, 65, pg, (pic + ((size_t)pn * 262)));
  if (item)
    xfput(176, 65, pg, (uint8_t *)objects[item]);

  wait_not_response();
  if (!tf)
    play_sound(WOOP, 1);

  while (1)
  {
    key = get_response();
    if (key == ESC)
    {
      wait_not_response();
      //       restore_screen=1;
      magic_cnt = magic_str;
      return 0;
    }
    if (*p == 0)
    {
      xput(152, 65, pg, (pic + (1 * 262)));
      wait_response();
      break;
    }
    if (*p == '~' && isxdigit(*(p + 1)))
    {
      p++;
      ch = *p;
      p++;
      if (isdigit(ch))
        ch -= 48;
      else
        ch = toupper(ch) - 55;
      color = dialog_color[ch];
      continue;
    }
    if (*p == 10)
    {
      x = 40;
      lc++;
      if (lc > 4)
      {
        pc = 0;
        pn = 0;
        xput(152, 65, pg, (pic + (1 * 262)));
        xprint(216, 134, "More...", pg, 15);
        key = wait_response();
        if (key == ESC)
        {
          wait_not_response();
          //           restore_screen=1;
          magic_cnt = magic_str;
          return 0;
        }
        wait_not_response();
        xfillrectangle(48, 84, 273, 145, pg, 215);
        lc = 0;
      }
      p++;
      continue;
    }
    xtext1(x + 12, 83 + (lc * 10), pg, text[(*p) - 32], 0);
    xtext(x + 12, 83 + (lc * 10), pg, text[(*p) - 32], color);
    timer_cnt = 0;
    pc++;
    if (pc > 1)
    {
      pc = 0;
      pn++;
      if (pn > 3)
        pn = 0;
      xput(152, 65, pg, (pic + ((size_t)pn * 262)));
    }
    if (!key_flag[key_fire] && !key_flag[ENTER] && !key_flag[SPACE] && !key_flag[key_magic])
    {
      if (tf)
      {
        play_sound(WOOP, 1);
        // TODO this looks like it had to do with an interrupt to work with VGA.
        // do
        // {
        //   rotate_pal();
        // } while (timer_cnt < 5);
      }
    }
    p++;
    x += 8;
  }
  xput(152, 65, pg, (pic + (1 * 262)));
  wait_not_response();
  //restore_screen=1;
  magic_cnt = magic_str;
  return 1;
}
//===========================================================================
void select_item(void)
{
  int16_t l, b, p, op;
  uint16_t pg;
  char *objn;

#define _HRZSP 24

  if (key_flag[key_magic] || tornado_used || lightning_used || thunder_flag ||
      hourglass_flag || thor->num_moves > 1 || shield_on || restore_screen)
  {
    return;
  }
  pg = display_page;

  xfillrectangle(72, 64, 249, 145, pg, 215);
  xfput(56, 48, pg, (uint8_t *)(bg_pics + (192 * 262)));
  xfput(248, 48, pg, (uint8_t *)(bg_pics + (193 * 262)));
  xfput(56, 144, pg, (uint8_t *)(bg_pics + (194 * 262)));
  xfput(248, 144, pg, (uint8_t *)(bg_pics + (195 * 262)));
  for (l = 0; l < 11; l++)
  {
    xfput(72 + (l * 16), 48, pg, (uint8_t *)(bg_pics + (196 * 262)));
    xfput(72 + (l * 16), 144, pg, (uint8_t *)(bg_pics + (197 * 262)));
  }
  for (l = 0; l < 5; l++)
  {
    xfput(56, 64 + (l * 16), pg, (uint8_t *)(bg_pics + (198 * 262)));
    xfput(248, 64 + (l * 16), pg, (uint8_t *)(bg_pics + (199 * 262)));
  }
  if (thor_info.inventory == 0)
  {
    xprint(104, 100, "No Items Found", pg, 14);
    wait_response();
    wait_not_response();
    restore_screen = 1;
    return;
  }
  b = 1;
  for (l = 0; l < 7; l++)
  {
    if (thor_info.inventory & b)
    {
      if (l < 6)
        xfput(82 + (l * _HRZSP), 72, pg, (uint8_t *)objects[l + 26]);
      else
        xfput(82 + (l * _HRZSP), 72, pg, (uint8_t *)objects[thor_info.object + 10]);
    }
    b = b << 1;
  }
  p = thor_info.item - 1;
  if (p < 1)
    p = 0;
  b = 1 << p;
  while (1)
  {
    if (thor_info.inventory & b)
      break;
    if (p < 7)
      p++;
    else
      p = 0;
    b = 1 << p;
  }
  if (p < 6)
    objn = item_name[p];
  else
    objn = thor_info.object_name;

  xprint((320 - (strlen(objn) * 8)) / 2, 114, objn, pg, 12);
  xbox(78 + (p * _HRZSP), 70, 98 + (p * _HRZSP), 90, pg, 15);
  op = p;
  wait_not_key(key_left);
  wait_not_key(key_right);
  wait_not_key(key_select);
  wait_not_response();
  while (1)
  {
    xshowpage(pg);
    sdl_man_update();
    if (key_flag[ESC])
    {
      wait_not_response();
      restore_screen = 1;
      return;
    }
    if (key_flag[key_select] || key_flag[key_fire] || key_flag[ENTER] || key_flag[SPACE] || key_flag[key_magic])
    {
      wait_not_response();
      restore_screen = 1;
      thor_info.item = p + 1;
      display_item();
      return;
    }
    if (key_flag[key_right])
    {
      while (1)
      {
        if (p < 9)
          p++;
        else
          p = 0;
        b = 1 << p;
        if (thor_info.inventory & b)
          break;
      }
    }
    else if (key_flag[key_left])
    {
      while (1)
      {
        if (p > 0)
          p--;
        else
          p = 8;
        b = 1 << p;
        if (thor_info.inventory & b)
          break;
      }
    }
    else
      continue;
    if (p == op)
      continue;
    xbox(78 + (op * _HRZSP), 70, 98 + (op * _HRZSP), 90, pg, 215);
    xbox(78 + (p * _HRZSP), 70, 98 + (p * _HRZSP), 90, pg, 15);
    op = p;
    xfillrectangle(78, 114, 249, 125, pg, 215);
    if (p < 6)
      objn = item_name[p];
    else
      objn = thor_info.object_name;
    xprint((320 - (strlen(objn) * 8)) / 2, 114, objn, pg, 12);
    timer_cnt = 0;
    play_sound(WOOP, 1);
    // TODO this looks like it had to do with an interrupt to work with VGA.
    // while (timer_cnt < 30)
    //   rotate_pal();
  }
}
//===========================================================================
int16_t switch_icons(void)
{
  int16_t x, y, ix, iy;

  play_sound(WOOP, 0);
  for (y = 0; y < 12; y++)
  {
    for (x = 0; x < 20; x++)
    {
      ix = x * 16;
      iy = y * 16;
      if (scrn.icon[y][x] == 93)
        place_tile(x, y, 144);
      else if (scrn.icon[y][x] == 144)
      {
        place_tile(x, y, 93);
        kill_enemies(iy, ix);
      }
      if (scrn.icon[y][x] == 94)
        place_tile(x, y, 146);
      else if (scrn.icon[y][x] == 146)
      {
        place_tile(x, y, 94);
        kill_enemies(iy, ix);
      }
    }
  }
  return 0;
}
//===========================================================================
int16_t rotate_arrows(void)
{
  int16_t x, y;

  play_sound(WOOP, 0);
  for (y = 0; y < 12; y++)
  {
    for (x = 0; x < 20; x++)
    {
      if (scrn.icon[y][x] == 205)
        place_tile(x, y, 208);
      else if (scrn.icon[y][x] == 206)
        place_tile(x, y, 207);
      else if (scrn.icon[y][x] == 207)
        place_tile(x, y, 205);
      else if (scrn.icon[y][x] == 208)
        place_tile(x, y, 206);
    }
  }
  return 0;
}
//===========================================================================
void kill_enemies(int16_t iy, int16_t ix)
{
  int16_t i, x1, y1, x2, y2;

  for (i = 3; i < MAX_ACTORS; i++)
  {
    if (actor[i].used)
    {
      x1 = actor[i].x;
      y1 = actor[i].y + actor[i].size_y - 2;
      x2 = (actor[i].x + actor[i].size_x);
      y2 = actor[i].y + actor[i].size_y - 1;
      if (point_within(x1, y1, ix, iy, ix + 15, iy + 15))
        actor_destroyed(&actor[i]);
      else if (point_within(x2, y1, ix, iy, ix + 15, iy + 15))
        actor_destroyed(&actor[i]);
      else if (point_within(x1, y2, ix, iy, ix + 15, iy + 15))
        actor_destroyed(&actor[i]);
      else if (point_within(x2, y2, ix, iy, ix + 15, iy + 15))
        actor_destroyed(&actor[i]);
    }
  }
  x1 = thor->x;
  y1 = thor->y + 11;
  x2 = x1 + 13;
  y2 = y1 + 5;
  if (point_within(x1, y1, ix, iy, ix + 15, iy + 15))
    goto dead;
  if (point_within(x2, y1, ix, iy, ix + 15, iy + 15))
    goto dead;
  if (point_within(x1, y2, ix, iy, ix + 15, iy + 15))
    goto dead;
  if (point_within(x2, y2, ix, iy, ix + 15, iy + 15))
    goto dead;
  return;

dead:
  thor->health = 0;
  display_health();
  exit_flag = 2;
}
//===========================================================================
void remove_objects(int16_t y, int16_t x)
{
  int16_t p, ix, iy;

  p = (y * 20) + x;
  ix = x * 16;
  iy = y * 16;

  if (object_map[p] > 0)
  {
    xfput(ix, iy, PAGE2, (uint8_t *)(bg_pics + ((size_t)scrn.bg_color * 262)));
    xfput(ix, iy, PAGE2, (uint8_t *)(bg_pics + ((size_t)scrn.icon[y][x] * 262)));
    xcopyd2d(ix, iy, ix + 16, iy + 16, ix, iy, PAGE2, draw_page, 320, 320);
    object_map[p] = 0;
    object_index[p] = 0;
  }
}
//==========================================================================
void place_tile(int16_t x, int16_t y, int16_t tile)
{
  int16_t ix, iy;

  ix = x * 16;
  iy = y * 16;

  xfput(ix, iy, PAGE2, (uint8_t *)(bg_pics + ((size_t)scrn.bg_color * 262)));
  xfput(ix, iy, PAGE2, (uint8_t *)(bg_pics + ((size_t)tile * 262)));
  xcopyd2d(ix, iy, ix + 16, iy + 16, ix, iy, PAGE2, draw_page, 320, 320);
  xcopyd2d(ix, iy, ix + 16, iy + 16, ix, iy, PAGE2, display_page, 320, 320);
  scrn.icon[y][x] = tile;
  remove_objects(y, x);
}
//==========================================================================
int16_t bgtile(int16_t x, int16_t y)
{

  if (x < 0 || x > 319 || y < 0 || y > 191)
    return 0; //303 , 175

  x = (x + 1) >> 4;
  y = (y + 1) >> 4;
  return scrn.icon[y][x];
}
