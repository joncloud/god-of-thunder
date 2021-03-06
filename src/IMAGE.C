// THOR - The God of Thunder
// See LICENSE for details

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DEFINE.H"
#include "PROTO.H"
//===========================================================================
extern uint8_t pge;
extern uint16_t draw_page, display_page, page3_offset;
extern int16_t current_level, new_level;
extern uint8_t *ami_buff;
extern uint8_t abuff[AMI_LEN];
extern uint8_t *mask_buff;
extern uint8_t *mask_buff_start;
extern ACTOR actor[MAX_ACTORS];  //current actors
extern ACTOR enemy[MAX_ENEMIES]; //current enemies
extern ACTOR shot[MAX_ENEMIES];  //current shots
extern uint8_t enemy_type[MAX_ENEMIES];
int16_t etype[MAX_ENEMIES];
uint16_t latch_mem;
uint8_t *enemy_mb;
uint16_t enemy_lm;
uint8_t *enemy_ami;

extern int16_t thor_x1, thor_y1, thor_x2, thor_y2;
extern ACTOR *thor;
extern ACTOR *hammer;
extern ACTOR explosion;
extern ACTOR sparkle;
extern THOR_INFO thor_info;
extern uint8_t *tmp_buff;

extern LEVEL scrn;
extern uint8_t *sd_data;
extern uint8_t sd_header[128];
extern int16_t current_level;
extern SETUP setup;
extern uint8_t play_speed;
extern int16_t max_shot;
extern ACTOR magic_item[];
extern uint8_t magic_pic[][1024];

uint8_t *magic_ami;
uint8_t *magic_mask_buff;

uint8_t *ami_store1, *ami_store2;
uint8_t *mask_store1, *mask_store2;
//===========================================================================
uint16_t make_mask(MASK_IMAGE *new_image,
                   uint16_t page_start, uint8_t *Image, int16_t image_width,
                   int16_t image_height)
{
   uint16_t page_offset, size;
   int16_t align, set;
   ALIGNED_MASK_IMAGE *work_ami;
   int16_t scan_line, bit_num, temp_image_width;
   uint8_t mask_temp;
   uint8_t *new_mask_ptr;
   uint8_t *old_mask_ptr;

   page_offset = page_start;
   set = 0;
   for (align = 0; align < 3; align += 2)
   {

      work_ami = new_image->alignments[set++] = (ALIGNED_MASK_IMAGE *)ami_buff;
      ami_buff += sizeof(ALIGNED_MASK_IMAGE);

      work_ami->image_width = (image_width + align + 3) / 4;
      work_ami->image_ptr = page_offset; /* image dest */

      /* Download this alignment of the image */
      xcopys2d(0, 0, image_width, image_height, align, 0,
               Image, page_offset, image_width,
               work_ami->image_width * 4);

      /* Calculate the number of bytes needed to store the mask in
         nibble (Map Mask-ready) form, then allocate that space */
      size = work_ami->image_width * image_height;
      work_ami->mask_ptr = (uint8_t *)mask_buff;
      mask_buff += size;

      /* Generate this nibble oriented (Map Mask-ready) alignment of
         the mask, one scan line at a time */
      old_mask_ptr = Image;
      new_mask_ptr = work_ami->mask_ptr;
      for (scan_line = 0; scan_line < image_height; scan_line++)
      {
         bit_num = align;
         mask_temp = 0;
         temp_image_width = image_width;
         do
         {

            /* Set the mask bit for next pixel according to its alignment */
            mask_temp |= (*old_mask_ptr != 15 && *old_mask_ptr != 0) << bit_num;
            old_mask_ptr++;
            if (++bit_num > 3)
            {
               *new_mask_ptr = mask_temp;
               new_mask_ptr++;
               mask_temp = bit_num = 0;
            }
         } while (--temp_image_width);

         /* Set any partial final mask on this scan line */
         if (bit_num != 0)
         {
            *new_mask_ptr = mask_temp;
            new_mask_ptr++;
         }
      }
      page_offset += size; /* mark off the space we just used */
   }
   return page_offset - page_start;
}
//===========================================================================
void setup_actor(ACTOR *actr, uint8_t num, uint8_t dir, int16_t x, int16_t y)
{

   actr->next = 0; //next frame to be shown
   actr->frame_count = actr->frame_speed;
   actr->dir = dir;      //direction of travel
   actr->last_dir = dir; //last direction of travel
   if (actr->directions == 1)
      actr->dir = 0;
   if (actr->directions == 2)
      actr->dir &= 1;
   if (actr->directions == 4 && actr->frames == 1)
   {
      actr->dir = 0;
      actr->next = dir;
   }

   actr->x = x;         //actual X coor
   actr->y = y;         //actual Y coor
   actr->width = 16;    //actual X coor
   actr->height = 16;   //actual Y coor
   actr->center = 0;    //center of object
   actr->last_x[0] = x; //last X coor on each page
   actr->last_x[1] = x;
   actr->last_y[0] = y; //last Y coor on each page
   actr->last_y[1] = y;
   actr->used = 1;            //1=active, 0=not active
   actr->speed_count = 8;     //count down to movement
   actr->vunerable = STAMINA; //count down to vunerability
   actr->shot_cnt = 20;       //count down to another shot
   actr->num_shots = 0;       //# of shots currently on screen
   actr->creator = 0;         //which actor # created this actor
   actr->pause = 0;           //pause must be 0 to move
   actr->show = 0;
   actr->actor_num = num;
   actr->counter = 0;
   actr->move_counter = 0;
   actr->edge_counter = 20;
   actr->hit_thor = 0;
   actr->rand = rnd(100);
   actr->temp1 = 0;
   actr->init_health = actr->health;
}
//===========================================================================
void make_actor_mask(ACTOR *actr)
{
   int16_t d, f;

   for (d = 0; d < actr->directions; d++)
   {
      for (f = 0; f < actr->frames; f++)
      {
         make_mask(&actr->pic[d][f], latch_mem, &tmp_buff[256 * ((d * 4) + f)], 16, 16);
         latch_mem += 144;
         if (latch_mem > 65421u)
         {
            exit_code(0);
            printf("Too Many Actor Frames\r\n");
            exit(0);
         }
      }
   }
}
//===========================================================================
int16_t load_standard_actors(void)
{

   latch_mem = 50160u;
   mask_buff = mask_buff_start;
   ami_buff = abuff;

   load_actor(0, 100 + thor_info.armor); //load thor
   memcpy(&actor[0], (tmp_buff + 5120), 40);
   setup_actor(&actor[0], 0, 0, 100, 100);
   thor = &actor[0];

   ami_store1 = ami_buff;
   mask_store1 = mask_buff;
   make_actor_mask(&actor[0]);

   thor_x1 = thor->x + 2;
   thor_y1 = thor->y + 2;
   thor_x2 = thor->x + 14;
   thor_y2 = thor->y + 14;

   load_actor(0, 103 + thor_info.armor); //load hammer
   memcpy(&actor[1], (tmp_buff + 5120), 40);
   setup_actor(&actor[1], 1, 0, 100, 100);
   actor[1].used = 0;
   hammer = &actor[1];

   ami_store2 = ami_buff;
   mask_store2 = mask_buff;
   make_actor_mask(&actor[1]);

   load_actor(0, 106); //load sparkle
   memcpy(&sparkle, (tmp_buff + 5120), 40);
   setup_actor(&sparkle, 20, 0, 100, 100);
   sparkle.used = 0;
   make_actor_mask(&sparkle);

   load_actor(0, 107); //load  explosion
   memcpy(&explosion, (tmp_buff + 5120), 40);
   setup_actor(&explosion, 21, 0, 100, 100);
   explosion.used = 0;
   make_actor_mask(&explosion);

   load_actor(0, 108); //load tornado
   memcpy(&magic_item[0], (tmp_buff + 5120), 40);
   memcpy(&magic_pic[0], tmp_buff, 1024);
   setup_actor(&magic_item[0], 20, 0, 0, 0);
   magic_item[0].used = 0;

   load_actor(0, 1099); //load shield
   memcpy(&magic_item[1], (tmp_buff + 5120), 40);
   memcpy(&magic_pic[1], tmp_buff, 1024);
   setup_actor(&magic_item[1], 20, 0, 0, 0);
   magic_item[1].used = 0;

   actor[2].used = 0;
   magic_ami = ami_buff;
   magic_mask_buff = mask_buff;

   make_actor_mask(&magic_item[0]); //to fool next lines

   enemy_mb = mask_buff;
   enemy_ami = ami_buff;
   enemy_lm = latch_mem;
   return 1;
}
//===========================================================================
void show_enemies(void)
{
   int16_t i, d, r;

   for (i = 3; i < MAX_ACTORS; i++)
      actor[i].used = 0; //was i=3
   for (i = 0; i < MAX_ENEMIES; i++)
      enemy_type[i] = 0;

   latch_mem = enemy_lm;
   mask_buff = enemy_mb;
   ami_buff = enemy_ami;

   for (i = 0; i < MAX_ENEMIES; i++)
   {
      if (scrn.actor_type[i] > 0)
      {
         r = load_enemy(scrn.actor_type[i]);
         if (r >= 0)
         {
            memcpy(&actor[i + 3], &enemy[r], sizeof(ACTOR));
            d = scrn.actor_dir[i];
            //       scrn.actor_type[i] &= 0x3f;
            setup_actor(&actor[i + 3], i + 3, d, (scrn.actor_loc[i] % 20) * 16,
                        (scrn.actor_loc[i] / 20) * 16);
            actor[i + 3].init_dir = scrn.actor_dir[i];
            actor[i + 3].pass_value = scrn.actor_value[i];
            if (actor[i + 3].move == 23)
            { //spinball
               if (actor[i + 3].pass_value & 1)
                  actor[i + 3].move = 24;
            }
            if (scrn.actor_invis[i])
               actor[i + 3].used = 0;
         }
         etype[i] = r;
      }
   }
}
//===========================================================================
int16_t load_enemy(int16_t type)
{
   int16_t i, f, d, e;
   ACTOR *enm;

   for (i = 0; i < MAX_ENEMIES; i++)
      if (enemy_type[i] == type)
         return i;

   if (!load_actor(1, type))
   {
      return -1;
   }
   enm = (ACTOR *)(tmp_buff + 5120);

   e = -1;
   for (i = 0; i < MAX_ENEMIES; i++)
   {
      if (!enemy_type[i])
      {
         e = i;
         break;
      }
   }
   if (e == -1)
      return -1;

   memcpy(&enemy[e], enm, sizeof(ACTOR_NFO));

   make_actor_mask(&enemy[e]);
   enemy_type[e] = type;
   enemy[e].shot_type = 0;

   if (enemy[e].shots_allowed)
   {
      enemy[e].shot_type = e + 1;
      enm = (ACTOR *)(tmp_buff + 5160);
      memcpy(&shot[e], enm, sizeof(ACTOR_NFO));
      for (d = 0; d < shot[e].directions; d++)
      {
         for (f = 0; f < shot[e].frames; f++)
         {
            if (shot[e].directions < shot[e].frames)
               make_mask(&shot[e].pic[d][f], latch_mem,
                         &tmp_buff[4096 + (256 * ((d * 4) + f))], 16, 16);
            else
               make_mask(&shot[e].pic[f][d], latch_mem,
                         &tmp_buff[4096 + (256 * ((f * 4) + d))], 16, 16);
            latch_mem += 144;
            if (latch_mem > 65421u)
            {
               exit_code(0);
               printf("Too Many Actor Frames\r\n");
               exit(0);
            }
         }
      }
   }
   return e;
}
//===========================================================================
int16_t actor_visible(int16_t invis_num)
{
   int16_t i, d;

   for (i = 0; i < MAX_ENEMIES; i++)
   {
      if (scrn.actor_invis[i] == invis_num)
      {
         if (etype[i] >= 0 && !actor[i + 3].used)
         {
            memcpy(&actor[i + 3], &enemy[etype[i]], sizeof(ACTOR));
            d = scrn.actor_dir[i];
            //       scrn.actor_type[i] &= 0x3f;
            setup_actor(&actor[i + 3], i + 3, d, (scrn.actor_loc[i] % 20) * 16,
                        (scrn.actor_loc[i] / 20) * 16);
            actor[i + 3].init_dir = scrn.actor_dir[i];
            actor[i + 3].pass_value = scrn.actor_value[i];
            return i;
         }
         else
            return -1;
      }
   }
   return -1;
}
//===========================================================================
void setup_magic_item(int16_t item)
{
   int16_t i;
   uint8_t *ami;
   uint8_t *mb;

   mb = mask_buff;
   mask_buff = magic_mask_buff;

   ami = ami_buff;
   ami_buff = magic_ami;

   for (i = 0; i < 4; i++)
   {
      make_mask(&magic_item[item].pic[i / 4][i % 4], 55200u + (144 * i), &magic_pic[item][256 * i], 16, 16);
   }
   ami_buff = ami;
   mask_buff = mb;
}
//===========================================================================
void load_new_thor(void)
{
   int16_t rep;
   uint8_t *ami;
   uint8_t *mb;

   mb = mask_buff;
   ami = ami_buff;

   mask_buff = mask_store1;
   ami_buff = ami_store1;

   load_actor(0, 100 + thor_info.armor); //load thor
   for (rep = 0; rep < 16; rep++)
   {
      make_mask(&actor[0].pic[rep / 4][rep % 4], PAGE3 + (144 * rep), &tmp_buff[256 * rep], 16, 16);
   }
   mask_buff = mask_store2;
   ami_buff = ami_store2;
   load_actor(0, 103 + thor_info.armor); //load hammer
   for (rep = 0; rep < 16; rep++)
   {
      make_mask(&actor[1].pic[rep / 4][rep % 4], 52464u + (144 * rep), &tmp_buff[256 * rep], 16, 16);
   }

   ami_buff = ami;
   mask_buff = mb;
}
