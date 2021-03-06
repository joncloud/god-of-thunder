// See LICENSE for details

//=========================== Include Files ===============================
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>

#include "DEFINE.H"
#include "PROTO.H"
#include "res_man.h"
#include "utility.h"
//====================== Functions Declarations============================
int16_t read_script_file(void);
void script_error(int16_t err_num);
int16_t get_command(void);
int16_t skip_colon(void);
int16_t calc_value(void);
int16_t get_next_val(void);
int16_t calc_string(int16_t mode);
void get_str(void);
int16_t get_internal_variable(void);
int16_t exec_command(int16_t num);
void script_entry(void);
void script_exit(void);
//============================ Externals ==================================
extern ACTOR *thor;
extern THOR_INFO thor_info;
extern int16_t current_level;
extern uint8_t odin[4][262];
extern uint8_t *tmp_buff;
extern uint8_t *sd_data;
extern uint8_t cheat;
extern volatile uint8_t key_flag[100];
extern int16_t key_magic;
extern char *object_names[];
extern SETUP setup;
extern uint8_t area;
extern uint16_t display_page, draw_page;
extern ACTOR actor[MAX_ACTORS];
extern int16_t new_level, current_level, new_level_tile;
extern LEVEL scrn;
extern uint8_t objects[NUM_OBJECTS][262];
extern uint8_t object_map[240];
extern uint8_t object_index[240];
extern int16_t thunder_flag;
//============================= Globals ==================================
int32_t num_var[26];       //numeric variables
char str_var[26][81];   //string vars
char line_label[32][9]; //line label look up table
char *line_ptr[32];     //line label pointers
char *new_ptr;
int16_t num_labels;       //number of labels
char *gosub_stack[32]; //stack for GOSUB return addresses
uint8_t gosub_ptr;        //GOSUB stack pointer
char *for_stack[10];   //FOR stack
int32_t for_val[10];      //current FOR value
uint8_t for_var[10];      //ending FOR value (target var)
uint8_t for_ptr;          //FOR stack pointer
char *buff_ptr;        //pointer to current command
char *buff_end;        //pointer to end of buffer
char *buffer;          //buffer space (malloc'ed)
int32_t scr_index;
uint8_t *scr_pic;
int32_t lvalue;
int32_t ltemp;
char temps[255];

#define SCR_BUFF_SIZE 5000
char *scr_command[] = {"!@#$%", "END", "GOTO", "GOSUB", "RETURN", "FOR", "NEXT",
                          "IF", "ELSE", "RUN",
                          "ADDJEWELS", "ADDHEALTH", "ADDMAGIC", "ADDKEYS",
                          "ADDSCORE", "SAY", "ASK", "SOUND", "PLACETILE",
                          "ITEMGIVE", "ITEMTAKE", "ITEMSAY", "SETFLAG", "LTOA",
                          "PAUSE", "TEXT", "EXEC", "VISIBLE", "RANDOM",
                          NULL};

char *internal_variable[] = {"@JEWELS", "@HEALTH", "@MAGIC", "@SCORE",
                                "@SCREEN", "@KEYS",
                                "@OW", "@GULP", "@SWISH", "@YAH", "@ELECTRIC",
                                "@THUNDER", "@DOOR", "@FALL", "@ANGEL", "@WOOP",
                                "@DEAD", "@BRAAPP", "@WIND", "@PUNCH", "@CLANG",
                                "@EXPLODE", "@FLAG", "@ITEM", "@THORTILE",
                                "@THORPOS", NULL};

char *scr_error[] = {"!@#$%", "Out of Memory", "Can't Read Script",
                        "Too Many Labels", "No END",
                        "Syntax", "Out of Range", "Undefined Label",
                        "RETURN Without GOSUB", "Nesting",
                        "NEXT Without FOR", NULL};
#define ERROR_MAX 10
//============================ Functions ==================================
void execute_script(int32_t index, uint8_t *pic)
{
  int16_t i, ret, re_execute;

  for (i = 0; i < MAX_ACTORS; i++)
    actor[i].show = 0;
  xdisplay_actors(&actor[MAX_ACTORS - 1], draw_page);
  xshowpage(draw_page);
  xerase_actors(actor, display_page);
  xdisplay_actors(&actor[MAX_ACTORS - 1], display_page);
  xshowpage(display_page);
  xerase_actors(actor, draw_page);

  scr_index = index;
  scr_pic = pic;

  memset(&num_var, 0, 4 * 26);
  memset(&str_var, 0, 81 * 26);

  re_execute = 0;
run_script: //jump point16_t for RUN command

  memset(line_label, 0, 32 * 9); //clear line label buffer
  memset(line_ptr, 0, 4 * 32);   //clear line ptrs

  memset(gosub_stack, 0, 52); //clear gosub stack
  gosub_ptr = 0;
  for (i = 0; i < 10; i++)
  {
    for_var[i] = 0;
    for_val[i] = 0;
    for_stack[i] = 0;
  }
  for_ptr = 0;

  i = read_script_file();
  if (i != 0)
  {
    script_error(i);
    script_exit();
    return;
  }
  if (!re_execute)
    script_entry();

  buff_ptr = buffer;
  while (1)
  {
    if (cheat && key_flag[_B])
      break;
    ret = get_command();
    if (ret == -1)
      break; //ignore NO END error
    else if (ret == -2)
    {
      script_error(5); // syntax error
      break;
    }
    else if (ret > 0)
    {
      ret = exec_command(ret);
      if (ret == -100)
      { //RUN command
        re_execute = 1;
        if (buffer)
          free(buffer);
        goto run_script;
      }
      if (!ret)
        break;
    }
  }
  script_exit();
  return;
}
//=========================================================================
void script_entry(void)
{
}
//=========================================================================
void script_exit(void)
{

  //xshowpage(display_page);
  if (buffer)
    free(buffer);
}
//=========================================================================
int16_t skip_colon(void)
{

  while (*buff_ptr == 0 || *buff_ptr == ':')
  {
    buff_ptr++;
    if (buff_ptr > buff_end)
      return 0;
  }
  return 1;
}
//=========================================================================
int16_t get_command(void)
{
  int16_t ret, i, len;

  if (!skip_colon())
    return -1;

  i = 0;
  while (1)
  {
    if (!scr_command[i])
      break; //lookup command
    len = strlen(scr_command[i]);
    if (!strncmp(buff_ptr, (char *)scr_command[i], len))
    {
      buff_ptr += len;
      return i;
    }
    i++;
  }
  if (isalpha(*buff_ptr))
  {
    if (*(buff_ptr + 1) == '=')
    { //num var assignment
      i = (*buff_ptr) - 65;
      buff_ptr += 2;
      ret = calc_value();
      if (!ret)
        return -2;
      else
      {
        num_var[i] = lvalue;
        return 0;
      }
    }
    else if (*(buff_ptr + 1) == '$' && *(buff_ptr + 2) == '=')
    {
      i = (*buff_ptr) - 65;
      buff_ptr += 3;
      ret = calc_string(0); //string var assignment
      if (ret == 0)
        return -2;
      else if (ret == -1)
        return -3;
      if (strlen(temps) > 80)
        return -3;
      strcpy(str_var[i], temps);
      return 0;
    }
  }
  return -2;
}
//=========================================================================
int16_t calc_string(int16_t mode)
{ //if mode==1 stop at comma
  char varstr[255];
  uint8_t varnum;

  strcpy(varstr, "");

  if (!skip_colon())
    return 0;

strloop:
  if (*buff_ptr == '"')
  {
    get_str();
    if (strlen(varstr) + strlen(temps) < 255)
      strcat((char *)varstr, temps);
    goto nextstr;
  }
  if (isalpha(*buff_ptr))
  {
    if (*(buff_ptr + 1) == '$')
    {
      varnum = (*buff_ptr) - 65;
      if (strlen(varstr) + strlen(str_var[varnum]) < 255)
        strcat((char *)varstr, str_var[varnum]);
      buff_ptr += 2;
      goto nextstr;
    }
  }
  return 0;

nextstr:
  if (*buff_ptr == 0 || *buff_ptr == 58)
  {
    buff_ptr++;
    goto strdone;
  }
  if (*buff_ptr == ',' && mode == 1)
    goto strdone;

  if (*buff_ptr == '+')
  {
    buff_ptr++;
    goto strloop;
  }
  return 0;

strdone:
  if (strlen(varstr) > 255)
    return -1;
  strcpy(temps, (char *)varstr);
  return 1;
}
//=========================================================================
void get_str(void)
{
  int16_t t;

  buff_ptr++;
  t = 0;
  while (1)
  {
    if (*buff_ptr == '"' || *buff_ptr == 0)
    {
      temps[t] = 0;
      if (*buff_ptr == '"')
        buff_ptr++;
      return;
    }
    temps[t++] = *buff_ptr;
    buff_ptr++;
  }
}
//=========================================================================
int16_t calc_value(void)
{
  int32_t tmpval2;
  uint8_t exptype;
  uint8_t ch;

  tmpval2 = 0;
  exptype = 1;

  while (1)
  {
    if (!get_next_val())
      return 0;
    switch (exptype)
    {
    case 0:
      tmpval2 = tmpval2 * ltemp;
      break;
    case 1:
      tmpval2 = tmpval2 + ltemp;
      break;
    case 2:
      tmpval2 = tmpval2 - ltemp;
      break;
    case 3:
      if (ltemp != 0)
        tmpval2 = tmpval2 / ltemp;
      break;
    }
    ch = *buff_ptr;
    switch (ch)
    {
    case 42:
      exptype = 0; /* multiply */
      break;
    case 43:
      exptype = 1; /* add */
      break;
    case 45:
      exptype = 2; /* minus */
      break;
    case 47:
      exptype = 3; /* divide */
      break;
    default:
      lvalue = tmpval2;
      return 1;
    }
    buff_ptr++;
  }
}
//=========================================================================
int16_t get_next_val(void)
{
  char ch;
  char tmpstr[25];
  int16_t t;

  ch = *buff_ptr;
  if (ch == 0 || ch == ':')
    return 0;
  if (ch == 64)
    return get_internal_variable();

  if (isalpha(ch))
  {
    buff_ptr++;
    ltemp = num_var[ch - 65];
    return 1;
  }
  t = 0;
  if (strchr("0123456789-", ch))
  {
    tmpstr[0] = ch;
    t++;
    buff_ptr++;
    while (strchr("0123456789", *buff_ptr) && *buff_ptr != 0)
    {
      tmpstr[t] = *buff_ptr;
      buff_ptr++;
      t++;
    }
    tmpstr[t] = 0;
    if (t > 10)
      return 0;
    ltemp = parse_decimal_int32_t(tmpstr);
    return 1;
  }
  return 0;
}
//=========================================================================
int16_t get_internal_variable(void)
{
  int16_t i, len;
  uint8_t b;
  uint8_t *sp;

  i = 0;
  while (1)
  {
    if (!internal_variable[i])
      return 0; //lookupint16_ternal variable
    len = strlen(internal_variable[i]);
    if (!strncmp(buff_ptr, internal_variable[i], len))
    {
      buff_ptr += len;
      break;
    }
    i++;
  }
  switch (i)
  {
  case 0:
    ltemp = thor_info.jewels;
    break;
  case 1:
    ltemp = thor->health;
    break;
  case 2:
    ltemp = thor_info.magic;
    break;
  case 3:
    ltemp = thor_info.score;
    break;
  case 4:
    ltemp = current_level;
    break;
  case 5:
    ltemp = thor_info.keys;
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
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
    ltemp = (int32_t)(i - 5l);
    break;
  case 22:
    if (!calc_value())
      return 0;
    i = (int)lvalue;
    if (i < 1 || i > 64)
      return 0;
    sp = (uint8_t *)&setup;
    sp += (i / 8);
    b = 1;
    b = b << (i % 8);
    if (*sp & b)
      ltemp = 1;
    else
      ltemp = 0;
    break;
  case 23:
    if (thor_info.inventory & 64)
      ltemp = thor_info.object;
    else
      ltemp = 0;
    break;
  case 24:
    ltemp = scrn.icon[(thor->y + 8) / 16][(thor->x + 7) / 16];
    break;
  case 25:
    ltemp = (((thor->y + 8) / 16) * 20) + ((thor->x + 7) / 16);
    break;
  default:
    return 0;
  }
  return 1;
}
//=========================================================================
int16_t get_line(char *src, char *dst)
{
  int16_t cnt;

  cnt = 0;
  while (*src != 13)
  {
    if (*src != 10)
    {
      *dst = *src;
      dst++;
    }
    cnt++;
    src++;
  }
  *dst = 0;
  cnt++;
  src++;
  return cnt;
}
//=========================================================================
int16_t read_script_file(void)
{
  char temp_buff[255];
  uint8_t quote_flag;
  int16_t i, len, p, ret, cnt;
  char ch;
  char tmps[255];
  char *sb;
  char *sbuff;
  char str[21];

  buffer = (char*)malloc(SCR_BUFF_SIZE);
  if (!buffer)
  {
    ret = 1;
    goto done;
  };
  buff_ptr = buffer;
  memset(buffer, 0, SCR_BUFF_SIZE);

  sbuff = (char*)malloc(25000l);
  if (!sbuff)
  {
    ret = 1;
    goto done;
  };
  sb = sbuff;

  sprintf(tmps, "%d", area);
  strcpy(str, "SPEAK");
  strcat(str, tmps);
  if (res_read(str, (uint8_t*)sb) < 0)
  {
    ret = 6;
    goto done;
  }

  sprintf(str, "%d", scr_index);
  strcpy(temp_buff, "|");
  strcat(temp_buff, str);

  while (1)
  {
    cnt = get_line(sb, (char *)tmps);
    sb += cnt;
    if (!strcmp(tmps, "|EOF"))
    {
      ret = 2;
      goto done;
    }
    if (!strcmp(tmps, temp_buff))
      break;
  }
  num_labels = 0;
  while (1)
  {
    cnt = get_line(sb, (char *)tmps);
    if (!strcmp(tmps, "|STOP"))
    {
      if (buff_ptr != buffer)
      {
        buff_end = buff_ptr;
        ret = 0;
        goto done;
      }
      ret = 2;
      goto done;
    }
    sb += cnt;
    len = strlen(tmps);
    if (len < 2)
    {
      *buff_ptr = 0;
      buff_ptr++;
      continue;
    }
    quote_flag = 0;
    p = 0;
    for (i = 0; i < len; i++)
    {
      ch = tmps[i];
      if (ch == 34)
        quote_flag ^= 1;
      else if (ch == 13 || ch == 10)
      { //check for CR
        temp_buff[p] = 0;
        break;
      }
      else if ((ch == 39 || ch == 96) && !quote_flag)
      {
        temp_buff[p] = 0;
        break;
      }
      if (!quote_flag)
        ch = toupper(ch);
      if (quote_flag || ch > 32)
      {
        temp_buff[p++] = ch;
      }
    }
    temp_buff[p] = 0;

    len = strlen(temp_buff);
    if (len < 10 && temp_buff[len - 1] == ':')
    { //line label
      temp_buff[len - 1] = 0;
      line_ptr[num_labels] = buff_ptr;
      strcpy(line_label[num_labels++], (char *)temp_buff);
      if (num_labels > 31)
      {
        ret = 3;
        goto done;
      }
      *buff_ptr = 0;
      buff_ptr++;
      continue;
    }
    strcpy(buff_ptr, (char *)temp_buff);
    buff_ptr += strlen(temp_buff);
    *buff_ptr = 0;
    buff_ptr++;
  }
done:
  if (sbuff)
    free(sbuff);
  return ret;
}
//=========================================================================
void script_error(int16_t err_num)
{
  int16_t line_num;
  char s[17];
  char *tb;
  char ts[81];

  line_num = 1;
  tb = buffer;

  while (1)
  {
    if (*tb == 0)
      line_num++;
    if (tb >= buff_ptr)
      break;
    tb++;
  }

  if (err_num > ERROR_MAX)
    err_num = 5; //unknown=syntax

  strcpy((char *)ts, scr_error[err_num]);
  sprintf(s, "%d", line_num);
  strcat(ts, " in Line #");
  strcat(ts, s);

  if (cheat)
  {
    play_sound(DEAD, 1);
    xfillrectangle(0, 0, 320, 10, PAGES, 0);
    xprint(0, 0, ts, PAGES, 14);
  }
}
//=========================================================================
int16_t get_string(void)
{

  memset(temps, 0, 255);
  if (*buff_ptr == '"')
  {
    get_str();
  }
  else if (isalpha(*buff_ptr))
  {
    if ((*buff_ptr + 1) == '$')
    {
      strcpy(temps, str_var[(*buff_ptr) - 65]);
      buff_ptr += 2;
    }
  }
  else
    return 5;

  if (strlen(temps) > 80)
    return 6;
  return 0;
}
//=========================================================================
int16_t cmd_goto(void)
{
  int16_t i, len;
  char s[255];
  char *p;

  strcpy(s, buff_ptr);
  p = strchr(s, ':');
  if (p)
    *p = 0;

  for (i = 0; i < num_labels; i++)
  {
    len = strlen(s);
    if (len == 0)
      break;
    if (!strcmp((char *)s, line_label[i]))
    {
      new_ptr = line_ptr[i];
      buff_ptr += len;
      return 0;
    }
  }
  return 8;
}
//=========================================================================
int16_t cmd_if(void)
{
  int32_t tmpval1, tmpval2;
  uint8_t exptype, ch;

  if (!calc_value())
    return 5;
  tmpval1 = lvalue;
  exptype = *buff_ptr;
  buff_ptr++;

  ch = *buff_ptr;
  if (ch == 60 || ch == 61 || ch == 62)
  {
    if (exptype == *buff_ptr)
      return 5;
    exptype += *buff_ptr;
    buff_ptr++;
  }
  if (!calc_value())
    return 5;
  tmpval2 = lvalue;
  buff_ptr += 4;
  switch (exptype)
  {
  case 60: /* less than */
    if (tmpval1 < tmpval2)
      goto iftrue;
    goto iffalse;
  case 61: /* equal */
    if (tmpval1 == tmpval2)
      goto iftrue;
    goto iffalse;
  case 62: /* greater than */
    if (tmpval1 > tmpval2)
      goto iftrue;
    goto iffalse;
  case 121: /* less than or equal */
    if (tmpval1 <= tmpval2)
      goto iftrue;
    goto iffalse;
  case 122: /* less or greater (not equal) */
    if (tmpval1 != tmpval2)
      goto iftrue;
    goto iffalse;
  case 123: /* greater than or equal */
    if (tmpval1 >= tmpval2)
      goto iftrue;
    goto iffalse;
  default:
    return 5;
  }

iffalse:
  while (*buff_ptr != 0)
    buff_ptr++;
  while (*buff_ptr == 0)
    buff_ptr++;

  if (!strncmp(buff_ptr, "ELSE", 4))
    buff_ptr += 4;

iftrue:
  return 0;
}
//=========================================================================
int16_t cmd_run(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  scr_index = lvalue;
  return -100;
}
//=========================================================================
int16_t cmd_addjewels(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  add_jewels((int16_t)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_addhealth(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  add_health((int16_t)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_addmagic(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  add_magic((int16_t)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_addkeys(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  add_keys((int16_t)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_addscore(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  add_score((int16_t)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_say(int16_t mode, int16_t type)
{
  uint8_t *p;
  int16_t obj;

  if (mode)
  {
    if (!calc_value())
      return 5;
    buff_ptr++;
    obj = (int16_t)lvalue;
    if (obj < 0 || obj > 32)
      return 6;
    if (obj)
      obj += 10;
  }
  else
    obj = 0;

  memset(tmp_buff, 0, TMP_SIZE);
  p = (uint8_t *)tmp_buff;
  while (calc_string(0))
  {
    strcpy((char *)p, temps);
    p += strlen(temps);
    *(p) = 10;
    p++;
  }
  *(p - 1) = 0;
  display_speech(obj, (uint8_t *)scr_pic, type);
  d_restore();
  return 0;
}
//=========================================================================
int16_t cmd_ask(void)
{
  int16_t i, v, p;
  char title[41];
  char *op[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  char opts[10][41];

  memset(tmp_buff, 0, TMP_SIZE);
  memset(opts, 0, 10 * 41);

  if (!skip_colon())
    return 5;

  if (isalpha(*buff_ptr))
  {
    v = *buff_ptr - 65;
    buff_ptr++;
    if (*buff_ptr != ',')
      return 5;
    buff_ptr++;
  }
  else
    return 5;

  if (!calc_string(1))
    return 5;
  strncpy((char *)title, temps, 41);
  title[40] = 0;

  if (*buff_ptr == ',')
  {
    buff_ptr++;
    if (!calc_value())
      return 5;
    buff_ptr++;
    p = (int)lvalue;
  }
  else
    return 5;

  i = 0;
  while (calc_string(0))
  {
    strncpy((char *)opts[i], temps, 41);
    opts[i][40] = 0;
    op[i] = opts[i];
    i++;
    if (i > 9)
      return 3;
  }
  if (p > i)
    p = 0;
  num_var[v] = select_option(op, title, p - 1);
  d_restore();
  return 0;
}
//=========================================================================
int16_t cmd_sound(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  if (lvalue < 1 || lvalue > 16)
    return 6;
  play_sound((int)lvalue - 1, 1);
  return 0;
}
//=========================================================================
int16_t cmd_settile(void)
{
  int16_t screen, pos, tile;
  LEVEL *lvl;

  if (!calc_value())
    return 5;
  buff_ptr++;
  screen = (int16_t)lvalue;
  if (!calc_value())
    return 5;
  buff_ptr++;
  pos = (int16_t)lvalue;
  if (!calc_value())
    return 5;
  tile = (int16_t)lvalue;
  if (screen < 0 || screen > 119)
    return 6;
  if (pos < 0 || pos > 239)
    return 6;
  if (tile < 0 || tile > 230)
    return 6;
  if (screen == current_level)
  {
    place_tile(pos % 20, pos / 20, tile);
  }
  else
  {
    lvl = (LEVEL *)(sd_data + ((size_t)screen * 512));
    lvl->icon[pos / 20][pos % 20] = tile;
  }
  return 0;
}
//=========================================================================
int16_t cmd_itemgive(void)
{
  int16_t i;

  if (!calc_value())
    return 5;
  buff_ptr++;
  i = (int16_t)lvalue;
  if (i < 1 || i > 15)
    return 6;

  thor_info.inventory |= 64;
  thor_info.item = 7;
  thor_info.object = i;
  display_item();
  thor_info.object_name = object_names[thor_info.object - 1];
  return 0;
}
//=========================================================================
int16_t cmd_itemtake(void)
{

  delete_object();
  return 0;
}
//=========================================================================
int16_t cmd_setflag(void)
{
  int16_t i;
  uint8_t b;
  uint8_t *sp;

  if (!calc_value())
    return 5;
  i = (int)lvalue;
  if (i < 1 || i > 64)
    return 6;
  sp = (uint8_t *)&setup;
  sp += (i / 8);
  b = 1;
  b = b << (i % 8);
  *sp |= b;
  return 0;
}
//=========================================================================
int16_t cmd_ltoa(void)
{
  int16_t sv;
  char str[21];

  if (!calc_value())
    return 5;
  buff_ptr++;

  if (isalpha(*buff_ptr))
  {
    if (*(buff_ptr + 1) == '$')
    {
      sv = (*buff_ptr) - 65;
      buff_ptr += 2;
    }
    else
      return 5;
  }
  else
    return 5;

  sprintf(str, "%d", lvalue);
  strcpy(str_var[sv], (char *)str);
  return 0;
}
//=========================================================================
int16_t cmd_pause(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  if (lvalue < 1 || lvalue > 65535l)
    return 6;

  pause((int)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_visible(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  if (lvalue < 1 || lvalue > 16)
    return 6;

  actor_visible((int)lvalue);
  return 0;
}
//=========================================================================
int16_t cmd_random(void)
{
  int16_t v, r;

  if (isalpha(*buff_ptr))
  {
    v = *buff_ptr - 65;
    buff_ptr++;
    if (*buff_ptr != ',')
      return 5;
    buff_ptr++;
  }
  else
    return 5;

  if (!calc_value())
    return 5;
  buff_ptr++;
  r = (int16_t)lvalue;
  if (r < 1 || r > 1000)
    return 6;

  num_var[v] = rand() % r;
  return 0;
}
//=========================================================================
void scr_func1(void)
{

  play_sound(FALL, 1);
  if (key_flag[_FOUR])
    return;
  new_level = 109;
  new_level_tile = 215;
  thor->x = (new_level_tile % 20) * 16;
  thor->y = ((new_level_tile / 20) * 16) - 2;

  thor->last_x[0] = thor->x;
  thor->last_x[1] = thor->x;
  thor->last_y[0] = thor->y;
  thor->last_y[1] = thor->y;
  thor->show = 2;
}
//=========================================================================
char *offense[] = {
    "Cussing",
    "Rebellion",
    "Kissing Your Mother Goodbye",
    "Being a Thunder God",
    "Door-to-Door Sales",
    "Carrying a Concealed Hammer",
};

char *reason[] = {
    "We heard you say 'Booger'.",
    "You look kind of rebellious.",
    "Your mother turned you in.",
    "We don't want you here.",
    "Nobody wants your sweepers.",
    "That's a dangerous weapon.",
};
//=========================================================================
void scr_func2(void)
{
  int16_t r;

  r = rnd(6);
  strcpy(str_var[0], offense[r]);
  strcpy(str_var[1], reason[r]);
}
//=========================================================================
void scr_func3(void)
{
  int16_t p, x, y, o;

  p = (((thor->y + 8) / 16) * 20) + ((thor->x + 7) / 16);
  y = p / 20;
  x = p % 20;
  if (y < 0 || x < 0 || y > 11 || x > 19)
  {
    play_sound(BRAAPP, 1);
    while (key_flag[key_magic])
      rotate_pal();
    return;
  }
  if (scrn.icon[y][x] < 174 || scrn.icon[y][x] > 178)
  {
    play_sound(BRAAPP, 1);
    while (key_flag[key_magic])
      rotate_pal();
    return;
  }
  num_var[0] = 1;
  play_sound(WOOP, 1);
  if (current_level == 106 && p == 69)
  {
    place_tile(x, y, 220);
    while (key_flag[key_magic])
      rotate_pal();
    return;
  }

  while (key_flag[key_magic])
    rotate_pal();
  place_tile(x, y, 191);
  if ((rand() % 100) < 25 || (current_level == 13 && p == 150 && !setup.f26 && setup.f28))
  {
    if (!object_map[p] && scrn.icon[y][x] >= 140)
    { //nothing there and solid
      o = (rand() % 5) + 1;
      if (current_level == 13 && p == 150 && !setup.f26 && setup.f28)
        o = 20;
      object_map[p] = o;
      object_index[p] = 31; //actor is 3-15
      x = (p % 20) * 16;
      y = (p / 20) * 16;
      xfput(x, y, PAGE2, (uint8_t *)objects[o - 1]);
      xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, draw_page, 320, 320);
      xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, display_page, 320, 320);
      pause(30);
    }
  }
}
//=========================================================================
void scr_func4(void)
{

  thunder_flag = 60;
}
//=========================================================================
void scr_func5(void)
{

  scrn.actor_loc[0] -= 2;
  scrn.actor_loc[1] -= 2;
  scrn.actor_loc[2] -= 2;
  scrn.actor_loc[3] -= 2;
  actor[3].i1 = 16;
}
//=========================================================================
void scr_func1(void);

void (*scr_func[])() =
    {
        scr_func1,
        scr_func2,
        scr_func3,
        scr_func4,
        scr_func5,
};
//=========================================================================
int16_t cmd_exec(void)
{

  if (!calc_value())
    return 5;
  buff_ptr++;
  if (lvalue < 1 || lvalue > 10)
    return 6;

  scr_func[(int)(lvalue - 1)]();
  return 0;
}
//=========================================================================
int16_t exec_command(int16_t num)
{
  int16_t ret;
  uint8_t ch;

  ret = 0;
  switch (num)
  {
  case 1: //end
    return 0;
  case 2: //goto
    ret = cmd_goto();
    if (!ret)
      buff_ptr = new_ptr;
    break;
  case 3: //gosub
    ret = cmd_goto();
    if (!ret)
    {
      gosub_ptr++;
      if (gosub_ptr > 31)
      {
        ret = 10;
        break;
      }
      gosub_stack[gosub_ptr] = buff_ptr;
      buff_ptr = new_ptr;
    }
    break;
  case 4: //return
    if (!gosub_ptr)
    {
      ret = 9;
      break;
    }
    buff_ptr = gosub_stack[gosub_ptr--];
    break;
  case 5: //for
    for_ptr++;
    if (for_ptr > 10)
    {
      ret = 10;
      break;
    }
    ch = *buff_ptr;
    if (!isalpha(ch))
    {
      ret = 5;
      break;
    }
    ch -= 65;
    for_var[for_ptr] = ch;
    buff_ptr += 2;
    if (!calc_value())
    {
      ret = 5;
      break;
    }
    num_var[for_var[for_ptr]] = lvalue;
    buff_ptr += 2;
    if (!calc_value())
    {
      ret = 5;
      break;
    }
    for_val[for_ptr] = lvalue;
    for_stack[for_ptr] = buff_ptr;
    break;
  case 6: //next
    if (!for_ptr)
    {
      ret = 11;
      break;
    }
    num_var[for_var[for_ptr]] = num_var[for_var[for_ptr]] + 1;
    if (num_var[for_var[for_ptr]] <= for_val[for_ptr])
      buff_ptr = for_stack[for_ptr];
    else
      for_ptr--;
    break;
  case 7: //if
    ret = cmd_if();
    break;
  case 8: //else
    while (*buff_ptr != 0)
      buff_ptr++;
    break;
  case 9: //run
    ret = cmd_run();
    if (ret < 0)
      return -100;
    break;
  case 10: //addjewels
    ret = cmd_addjewels();
    break;
  case 11: //addhealth
    ret = cmd_addhealth();
    break;
  case 12: //addmagic
    ret = cmd_addmagic();
    break;
  case 13: //addkeys
    ret = cmd_addkeys();
    break;
  case 14: //addscore
    ret = cmd_addscore();
    break;
  case 15: //say
    ret = cmd_say(0, 1);
    break;
  case 16: //ask
    ret = cmd_ask();
    break;
  case 17: //sound
    ret = cmd_sound();
    break;
  case 18: //settile
    ret = cmd_settile();
    break;
  case 19: //itemgive
    ret = cmd_itemgive();
    break;
  case 20: //itemtake
    ret = cmd_itemtake();
    break;
  case 21: //itemsay
    ret = cmd_say(1, 1);
    break;
  case 22: //setflag
    ret = cmd_setflag();
    break;
  case 23: //ltoa
    ret = cmd_ltoa();
    break;
  case 24: //pause
    ret = cmd_pause();
    break;
  case 25: //text
    ret = cmd_say(0, 0);
    break;
  case 26: //exec
    ret = cmd_exec();
    break;
  case 27: //visible
    ret = cmd_visible();
    break;
  case 28: //random
    ret = cmd_random();
    break;
  default:
    ret = 5;
  }
  if (ret > 0)
  {
    script_error(ret);
    return 0;
  }
  return 1;
}
