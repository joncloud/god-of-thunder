// THOR - The God of Thunder
// See LICENSE for details

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DEFINE.H"
#include "PROTO.H"

#define HERMIT_HAS_DOLL setup.f04
//===========================================================================
extern uint8_t objects[NUM_OBJECTS][262];
extern uint8_t object_map[240];
extern uint8_t object_index[240];
extern uint8_t *object_sound[26];
extern int16_t ox, oy, of;
extern LEVEL scrn;
extern uint8_t *bg_pics;
extern uint16_t display_page, draw_page;
extern int16_t rand1, rand2;
extern THOR_INFO thor_info;
extern int16_t current_level;
extern volatile uint16_t magic_cnt;
extern ACTOR *thor, *hammer;
extern ACTOR actor[MAX_ACTORS]; //current actors
extern ACTOR magic_item[];
extern int16_t key_fire, key_up, key_down, key_left, key_right, key_magic, key_select;
extern volatile uint8_t key_flag[100];
extern int16_t hourglass_flag, thunder_flag, shield_on, lightning_used, tornado_used;
extern int16_t apple_flag, bomb_flag;
extern volatile uint16_t timer_cnt;
extern int16_t magic_inform, carry_inform;
extern char *object_names[];
extern int16_t exit_flag;
extern SETUP setup;

int16_t pixel_x[8][25];
int16_t pixel_y[8][25];
uint8_t pixel_p[8][25];
uint8_t pixel_c[8];
void throw_lightning(void);
void not_enough_magic(void);
void cannot_carry_more(void);
//===========================================================================
void show_objects(int16_t level, uint16_t pg)
{
  int16_t i, p;

  level = level;
  //memset(object_pos,255,38);
  memset(object_map, 0, 240);
  memset(object_index, 0, 240);
  for (i = 0; i < 30; i++)
  {
    if (scrn.static_obj[i])
    {
      xfput(scrn.static_x[i] * 16, scrn.static_y[i] * 16, pg,
            objects[scrn.static_obj[i] - 1]);
      p = scrn.static_x[i] + (scrn.static_y[i] * 20);
      object_index[p] = i;
      object_map[p] = scrn.static_obj[i];
    }
  }
}
//===========================================================================
void pick_up_object(int16_t p)
{
  int16_t r, x, y, s;

  switch (object_map[p])
  {
  case 1: //red jewel
    if (thor_info.jewels >= 999)
    {
      cannot_carry_more();
      return;
    }
    add_jewels(10);
    break;
  case 2: //blue jewel
    if (thor_info.jewels >= 999)
    {
      cannot_carry_more();
      return;
    }
    add_jewels(1);
    break;
  case 3: //red potion
    if (thor_info.magic >= 150)
    {
      cannot_carry_more();
      return;
    }
    add_magic(10);
    break;
  case 4: //blue potion
    if (thor_info.magic >= 150)
    {
      cannot_carry_more();
      return;
    }
    add_magic(3);
    break;
  case 5: //good apple
    if (thor->health >= 150)
    {
      cannot_carry_more();
      return;
    }
    play_sound(GULP, 0);
    s = 1;
    add_health(5);
    break;
  case 6: //bad apple
    play_sound(OW, 0);
    s = 1;
    add_health(-10);
    break;
  case 7: //key (reset on exit)
          //           if(scrn.reset) r=0;
    add_keys(1);
    break;
  case 8: //treasure
    if (thor_info.jewels >= 999)
    {
      cannot_carry_more();
      return;
    }
    add_jewels(50);
    break;
  case 9: //trophy
    add_score(100);
    break;
  case 10: //crown
    add_score(1000);
    break;
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
    if (object_map[p] == 13 && HERMIT_HAS_DOLL)
      return;
    thor->num_moves = 1;
    hammer->num_moves = 2;
    actor[2].used = 0;
    shield_on = 0;
    tornado_used = 0;
    thor_info.inventory |= 64;
    thor_info.item = 7;
    thor_info.object = object_map[p] - 11;
    display_item();
    thor_info.object_name = object_names[thor_info.object - 1];
    odin_speaks((object_map[p] - 12) + 501, object_map[p] - 1);
    break;
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
    hourglass_flag = 0;
    thunder_flag = 0;
    shield_on = 0;
    lightning_used = 0;
    tornado_used = 0;
    hammer->num_moves = 2;
    thor->num_moves = 1;
    actor[2].used = 0;
    s = 1 << (object_map[p] - 27);
    thor_info.inventory |= s;
    odin_speaks((object_map[p] - 27) + 516, object_map[p] - 1);
    s = 1;
    thor_info.item = object_map[p] - 26;
    display_item();
    add_magic(150);
    fill_score(5);
    break;
  }
  x = p % 20;
  y = p / 20;

  ox = x * 16;
  oy = y * 16;
  of = 1;
  xfput(ox, oy, PAGE2, (uint8_t *)(bg_pics + (scrn.bg_color * 262)));
  xfput(ox, oy, PAGE2, (uint8_t *)(bg_pics + (scrn.icon[y][x] * 262)));
  xcopyd2d(ox, oy, ox + 16, oy + 16, ox, oy, PAGE2, draw_page, 320, 320);
  //xcopyd2d(ox,oy,ox+16,oy+16,ox,oy,PAGE2,draw_page,320,320);

  r = 1;
  s = 0;
  if (!s)
    play_sound(YAH, 0);
  object_map[p] = 0;
  if (r)
  { //reset so it doesn't reappear on reentry to screen
    if (object_index[p] < 30)
      scrn.static_obj[object_index[p]] = 0;
    object_index[p] = 0;
  }
}
//===========================================================================
int16_t drop_object(ACTOR *actr)
{
  int16_t o, rnd1, rnd2;

  rnd1 = rnd(100);
  rnd2 = rnd(100);

  if (rnd1 < 25)
    o = 5; //apple
  else if (rnd1 & 1)
  { //jewels
    if (rnd2 < 10)
      o = 1; //red
    else
      o = 2; //blue
  }
  else
  { //potion
    if (rnd2 < 10)
      o = 3; //red
    else
      o = 4; //blue
  }
  _drop_obj(actr, o);
  return 1;
}
//===========================================================================
int16_t _drop_obj(ACTOR *actr, int16_t o)
{
  int16_t x, y, p;

  p = (actr->x + (actr->size_x / 2)) / 16 + (((actr->y + (actr->size_y / 2)) / 16) * 20);
  if (!object_map[p] && scrn.icon[p / 20][p % 20] >= 140)
  { //nothing there and solid
    object_map[p] = o;
    object_index[p] = 27 + actr->actor_num; //actor is 3-15
    x = (p % 20) * 16;
    y = (p / 20) * 16;
    xfput(x, y, PAGE2, (uint8_t *)objects[o - 1]);
    xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, draw_page, 320, 320);
    xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, display_page, 320, 320);
    return 1;
  }
  return 0;
}
//===========================================================================
int16_t use_apple(int16_t flag)
{

  if (thor->health == 150)
    return 0;

  if (flag && thor_info.magic > 0)
  {
    if (!apple_flag)
    {
      magic_cnt = 0;
      add_magic(-2);
      add_health(1);
      play_sound(ANGEL, 0);
    }
    else if (magic_cnt > 8)
    {
      magic_cnt = 0;
      add_magic(-2);
      add_health(1);
      if (!sound_playing())
        play_sound(ANGEL, 0);
    }
    apple_flag = 1;
    return 1;
  }
  else
  {
    apple_flag = 0;
    if (flag)
      not_enough_magic();
  }
  return 0;
}
//===========================================================================
int16_t use_thunder(int16_t flag)
{

  if (flag && thor_info.magic > 29)
  {
    if (!thunder_flag)
    {
      add_magic(-30);
      play_sound(THUNDER, 0);
      thunder_flag = 60;
    }
    return 1;
  }
  else if (flag && !thunder_flag)
    not_enough_magic();

  if (thunder_flag)
    return 1;
  return 0;
}
//===========================================================================
int16_t use_hourglass(int16_t flag)
{
  int16_t hour_time[] = {0, 60, 120, 180, 240, 300, 360, 420, 480,
                         510, 540, 570, 600, 630, 660, 690};

  if (hourglass_flag)
  {
    if (magic_cnt > hour_time[hourglass_flag])
    {
      hourglass_flag++;
      if (hourglass_flag == 16)
      {
        hourglass_flag = 0;
        music_resume();
      }
      play_sound(WOOP, 1);
    }
    return 1;
  }

  if (flag && thor_info.magic > 29)
  {
    if (!hourglass_flag)
    {
      magic_cnt = 0;
      add_magic(-30);
      music_pause();
      play_sound(WOOP, 1);
      hourglass_flag = 1;
      return 1;
    }
  }
  else
  {
    hourglass_flag = 0;
    if (flag)
      not_enough_magic();
  }
  return 0;
}
//===========================================================================
int16_t use_boots(int16_t flag)
{

  if (flag)
  {
    if (thor_info.magic > 0)
    {
      if (thor->num_moves == 1)
      {
        magic_cnt = 0;
        add_magic(-1);
      }
      else if (magic_cnt > 8)
      {
        magic_cnt = 0;
        add_magic(-1);
      }
      thor->num_moves = 2;
      hammer->num_moves = 3;
      return 1;
    }
    else
    {
      not_enough_magic();
      thor->num_moves = 1;
      hammer->num_moves = 2;
    }
  }
  else
  {
    thor->num_moves = 1;
    hammer->num_moves = 2;
  }
  return 0;
}
//===========================================================================
int16_t use_shield(int16_t flag)
{
  int16_t f;

  f = 0;
  if (flag)
  {
    if (thor_info.magic)
    {
      if (!shield_on)
      {
        magic_cnt = 0;
        add_magic(-1);
        setup_magic_item(1);
        memcpy(&actor[2], &magic_item[1], sizeof(ACTOR));
        setup_actor(&actor[2], 2, 0, thor->x, thor->y);
        actor[2].speed_count = 1;
        actor[2].speed = 1;
        shield_on = 1;
      }
      else if (magic_cnt > 8)
      {
        magic_cnt = 0;
        add_magic(-1);
      }
      return 1;
    }
    else
    {
      f = 1;
      not_enough_magic();
    }
  }
  else
    f = 1;
  if (f == 1)
  {
    if (shield_on)
    {
      actor[2].dead = 2;
      actor[2].used = 0;
      shield_on = 0;
    }
  }
  return 0;
}
//===========================================================================
int16_t use_lightning(int16_t flag)
{

  if (flag)
  {
    if (thor_info.magic > 14)
    {
      add_magic(-15);
      throw_lightning();
    }
    else
    {
      not_enough_magic();
      return 0;
    }
  }
  return 1;
}
//===========================================================================
int16_t use_tornado(int16_t flag)
{

  if (flag)
  {
    if (thor_info.magic > 10)
    {
      if (!tornado_used && !actor[2].dead && magic_cnt > 20)
      {
        magic_cnt = 0;
        add_magic(-10);
        setup_magic_item(0);
        memcpy(&actor[2], &magic_item[0], sizeof(ACTOR));
        setup_actor(&actor[2], 2, 0, thor->x, thor->y);
        actor[2].last_dir = thor->dir;
        actor[2].move = 16;
        tornado_used = 1;
        play_sound(WIND, 0);
      }
    }
    else if (tornado_used == 0)
    {
      not_enough_magic();
      return 0;
    }
    if (magic_cnt > 8)
    {
      if (tornado_used)
      {
        magic_cnt = 0;
        add_magic(-1);
      }
    }
    if (thor_info.magic < 1)
    {
      actor_destroyed(&actor[2]);
      tornado_used = 0;
      not_enough_magic();
      return 0;
    }
    return 1;
  }
  return 0;
}
//===========================================================================
int16_t use_object(int16_t flag)
{

  if (!flag)
    return 0;
  if (!thor_info.inventory & 64)
    return 0;

  //xdisplay_actors(&actor[MAX_ACTORS-1],draw_page);
  xshowpage(draw_page);
  xerase_actors(actor, display_page);
  xdisplay_actors(&actor[MAX_ACTORS - 1], display_page);
  xshowpage(display_page);

  odin_speaks((thor_info.object - 1) + 5501, thor_info.object - 1);
  xdisplay_actors(&actor[MAX_ACTORS - 1], draw_page);
  return 1;
}
//===========================================================================
void use_item(void)
{
  static int16_t flag = 0;
  int16_t kf, ret, mf;

  kf = key_flag[key_magic];

  if (!kf && tornado_used)
  {
    actor_destroyed(&actor[2]);
    tornado_used = 0;
  }

  mf = magic_inform;
  switch (thor_info.item)
  {
  case 1:
    ret = use_apple(kf);
    break;
  case 2:
    ret = use_lightning(kf);
    break;
  case 3:
    ret = use_boots(kf);
    break;
  case 4:
    ret = use_tornado(kf);
    break;
  case 5:
    ret = use_shield(kf);
    break;
  case 6:
    ret = use_thunder(kf);
    break;
  case 7:
    ret = use_object(kf);
    break;
  }
  if (kf && thor_info.item)
  {
    if ((!ret) && (!flag))
    {
      if (mf)
        play_sound(BRAAPP, 0);
      flag = 1;
    }
  }
  else
    flag = 0;
}
//===========================================================================
void place_pixel(int16_t dir, int16_t num)
{

  switch (dir)
  {
  case 0:
    pixel_y[dir][num] = pixel_y[dir][num - 1] - 1;
    pixel_x[dir][num] = pixel_x[dir][num - 1] + (1 - (rnd(3)));
    break;
  case 1:
    if (rnd(2))
    {
      pixel_x[dir][num] = pixel_x[dir][num - 1] + 1;
      pixel_y[dir][num] = pixel_y[dir][num - 1] + (0 - (rnd(2)));
    }
    else
    {
      pixel_y[dir][num] = pixel_y[dir][num - 1] - 1;
      pixel_x[dir][num] = pixel_x[dir][num - 1] + (1 - (rnd(2)));
    }
    break;
  case 2:
    pixel_x[dir][num] = pixel_x[dir][num - 1] + 1;
    pixel_y[dir][num] = pixel_y[dir][num - 1] + (1 - (rnd(3)));
    break;
  case 3:
    if (rnd(2))
    {
      pixel_x[dir][num] = pixel_x[dir][num - 1] + 1;
      pixel_y[dir][num] = pixel_y[dir][num - 1] + (1 - (rnd(2)));
    }
    else
    {
      pixel_y[dir][num] = pixel_y[dir][num - 1] + 1;
      pixel_x[dir][num] = pixel_x[dir][num - 1] + (1 - (rnd(2)));
    }
    break;
  case 4:
    pixel_y[dir][num] = pixel_y[dir][num - 1] + 1;
    pixel_x[dir][num] = pixel_x[dir][num - 1] + (1 - (rnd(3)));
    break;
  case 5:
    if (rnd(2))
    {
      pixel_x[dir][num] = pixel_x[dir][num - 1] - 1;
      pixel_y[dir][num] = pixel_y[dir][num - 1] + (1 - (rnd(2)));
    }
    else
    {
      pixel_y[dir][num] = pixel_y[dir][num - 1] + 1;
      pixel_x[dir][num] = pixel_x[dir][num - 1] + (0 - (rnd(2)));
    }
    break;
  case 6:
    pixel_x[dir][num] = pixel_x[dir][num - 1] - 1;
    pixel_y[dir][num] = pixel_y[dir][num - 1] + (1 - (rnd(3)));
    break;
  case 7:
    if (rnd(2))
    {
      pixel_x[dir][num] = pixel_x[dir][num - 1] - 1;
      pixel_y[dir][num] = pixel_y[dir][num - 1] + (0 - (rnd(2)));
    }
    else
    {
      pixel_y[dir][num] = pixel_y[dir][num - 1] - 1;
      pixel_x[dir][num] = pixel_x[dir][num - 1] + (0 - (rnd(2)));
    }
    break;
  default:
    return;
  }
  if (point_within(pixel_x[dir][num], pixel_y[dir][num], 0, 0, 319, 191))
  {
    pixel_p[dir][num] = xpoint(pixel_x[dir][num], pixel_y[dir][num], draw_page);
    xpset(pixel_x[dir][num], pixel_y[dir][num], draw_page, pixel_c[dir]);
  }
}
//===========================================================================
void replace_pixel(int16_t dir, int16_t num)
{

  if (point_within(pixel_x[dir][num], pixel_y[dir][num], 0, 0, 319, 191))
  {
    xpset(pixel_x[dir][num], pixel_y[dir][num], draw_page, pixel_p[dir][num]);
  }
}
//===========================================================================
void throw_lightning(void)
{
  int16_t i, r, loop, x, y, ax, ay;

  for (i = 0; i < MAX_ACTORS; i++)
    actor[i].show = 0;

  xdisplay_actors(&actor[MAX_ACTORS - 1], draw_page);
  xshowpage(draw_page);
  play_sound(ELECTRIC, 1);
  for (loop = 0; loop < 10; loop++)
  {
    for (i = 0; i < 8; i++)
    {
      pixel_x[i][0] = thor->x + 7;
      pixel_y[i][0] = thor->y + 7;
      pixel_c[i] = 14 + rnd(2);
    }
    for (r = 0; r < 8; r++)
    {
      for (i = 1; i < 25; i++)
      {
        place_pixel(r, i);
      }
    }
    timer_cnt = 0;
    // TODO is this just a delay?
    // while (timer_cnt < 3)
    //   ;
    for (r = 7; r >= 0; r--)
    {
      for (i = 1; i < 25; i++)
      {
        replace_pixel(r, i);
      }
    }
    timer_cnt = 0;
    // TODO is this just a delay?
    // while (timer_cnt < 3)
    //   ;
  }
  x = thor->x + 7;
  y = thor->y + 7;
  for (i = 3; i < MAX_ACTORS; i++)
  {
    if (!actor[i].used)
      continue;
    ax = actor[i].x + (actor[i].size_x / 2);
    ay = actor[i].y + (actor[i].size_y / 2);
    if ((abs(ax - x) < 30) && (abs(ay - y) < 30))
    {
      actor[i].magic_hit = 1;
      actor[i].vunerable = 0;
      actor_damaged(&actor[i], 254);
    }
  }
}
//===========================================================================
void not_enough_magic(void)
{

  if (!magic_inform)
    odin_speaks(2006, 0);
  xdisplay_actors(&actor[MAX_ACTORS - 1], draw_page);
  magic_inform = 1;
}
//===========================================================================
void cannot_carry_more(void)
{

  if (!carry_inform)
    odin_speaks(2007, 0);
  carry_inform = 1;
}
//===========================================================================
void delete_object(void)
{

  thor_info.inventory &= 0xbf;
  thor_info.item = 1;
  display_item();
}
