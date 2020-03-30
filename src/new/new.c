// THOR - The God of Thunder
// See LICENSE for details

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <Windows.h>

#include "../define.h"
#include "../proto.h"
#include "../res_man.h"
#include "new.h"

//===========================================================================
void* movedata (void* dest0, void* dest1, void* src0, void* src1, size_t bytes) {
    assert(dest0 == dest1);
    assert(src0 == src1);
    return memmove(dest0, src0, bytes);
}
//===========================================================================
void delay(int16_t ms) {
    Sleep((DWORD)ms);
}
//===========================================================================
void read_joystick(void) {
    return; // TODO
}
//===========================================================================
void xsetmode(void) {
    return; // TODO
}
//===========================================================================
void xshowpage(unsigned page) {
    return; // TODO
}
//===========================================================================
void xfillrectangle(int16_t StartX, int16_t StartY, int16_t EndX, int16_t EndY,
    uint16_t PageBase, int16_t Color) {
    return; // TODO
}
//===========================================================================
void xpset(int16_t X, int16_t Y, uint16_t PageBase, int16_t Color) {
    return; // TODO
}
//===========================================================================
int16_t xpoint(int16_t X, int16_t Y, uint16_t PageBase) {
    return 0; // TODO
}
//===========================================================================
void xput(int16_t x,int16_t y,uint16_t pagebase,char *buff) {
    return; // TODO
}
//===========================================================================
void xtext(int16_t x,int16_t y,uint16_t pagebase,char *buff,int16_t color) {
    return; // TODO
}
//===========================================================================
void xtext1(int16_t x,int16_t y,uint16_t pagebase,char *buff,int16_t color) {
    return; // TODO
}
//===========================================================================
void xtextx(int16_t x,int16_t y,uint16_t pagebase,char *buff,int16_t color) {
    return; // TODO
}
//===========================================================================
void xfput(int16_t x,int16_t y,uint16_t pagebase,char *buff) {
    return; // TODO
}
//===========================================================================
void xfarput(int16_t x,int16_t y,uint16_t pagebase,char *buff) {
    return; // TODO
}
//===========================================================================
void xcopys2d(int16_t SourceStartX, int16_t SourceStartY,
    int16_t SourceEndX, int16_t SourceEndY, int16_t DestStartX,
    int16_t DestStartY, char* SourcePtr, uint16_t DestPageBase,
    int16_t SourceBitmapWidth, int16_t DestBitmapWidth) {
    return; // TODO
}
//===========================================================================
void xcopyd2d(int16_t SourceStartX, int16_t SourceStartY,
    int16_t SourceEndX, int16_t SourceEndY, int16_t DestStartX,
    int16_t DestStartY, uint16_t SourcePageBase,
    uint16_t DestPageBase, int16_t SourceBitmapWidth,
    int16_t DestBitmapWidth) {
    return; // TODO
}
//===========================================================================
void xdisplay_actors(ACTOR *act,uint16_t page) {
    return; // TODO
}
//===========================================================================
void xerase_actors(ACTOR *act,uint16_t page) {
    return; // TODO
}
//===========================================================================
void pal_fade_in(char *buff) {
    return; // TODO
}
//===========================================================================
void pal_fade_out(char *buff) {
    return; // TODO
}
//===========================================================================
void LZSS_decompress(char far *src,char far *dest,int16_t len) {
    return; // TODO (more like wat do)
}
//===========================================================================
uint16_t LZSS_compress(int32_t origsize,char far *input,char far *output) {
    return 0; // TODO (more like wat do)
}