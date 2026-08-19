#include <windows.h>
#include <stdio.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "gfuncs.h"     //  graphics primitives
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphic objects
#include "alg_selector.h"

#define  COLOR_DECREMENT   2
//***********************************************************************
flames::flames(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
// , fire_palette(0)  //  cannot init array in init list
, fire_palette{}
, fire_palette_init(0)
, fire_palette_record(NULL)
, fire_char_width(0)
, fire_char_height(0)
, fire_rows(0)
, fire_cols(0)
{ 
   //lint -esym(1401, flames::fire_palette)
}

//lint -esym(1714, flames::dump_fire_palette)
//************************************************************************
void flames::dump_fire_palette(void)
{
   // FILE *fd = fopen("fire_palette.txt", "wt") ;
   unique_file fd(fopen("fire_palette.txt", "wt")) ;
   if (fd != 0) {
      unsigned idx ;
      for (idx=0; idx<256; idx++) {
         if ((idx % 4) == 0) {
            fprintf(fd.get(), "\n") ;
         }
         rgb_p fptr  = &fire_palette[idx] ;
         fprintf(fd.get(), "{ 0x%02X, 0x%02X, 0x%02X }, ",
            fptr->red, fptr->green, fptr->blue) ;
      }
      fprintf(fd.get(), "} ;\n") ;
      // fclose(fd) ;
   }
}

//************************************************************************
void flames::set_fire_palette(unsigned index, u8 red, u8 green, u8 blue)
{
   rgb_p fptr  = &fire_palette[index] ;
   fptr->red   = red ;
   fptr->green = green ;
   fptr->blue  = blue ;
}

//************************************************************************
unsigned flames::get_record_index(unsigned x, unsigned y)
{
   return ((y * fire_cols) + x) ;
}

//************************************************************************
void flames::set_palette_index(unsigned x, unsigned y, u8 color_idx)
{
   // unsigned idx = (y * fire_cols) + x ;
   if (fire_palette_record != 0)
      *(fire_palette_record + get_record_index(x, y)) = color_idx ;
}

//************************************************************************
unsigned flames::get_palette_index(unsigned x, unsigned y)
{
   // unsigned idx = (y * fire_cols) + x ;
   if (fire_palette_record != 0)
      return (unsigned) *(fire_palette_record + get_record_index(x, y)) ;
   else
      return 0 ;
}

//************************************************************************
//  copy fire palette into global palette table, so we can
//  display the revised palette on the "color bars" page.
//  Ultimately, though, we should probably just replace our local
//  fire table with the global palette, and save some space.
//************************************************************************
extern rgb_t dac_table28[256] ;

void flames::update_global_palette28(void)
{
   unsigned idx ;
   for (idx=0; idx<256; idx++) {
      dac_table28[idx].red   = fire_palette[idx].red ;
      dac_table28[idx].green = fire_palette[idx].green ;
      dac_table28[idx].blue  = fire_palette[idx].blue ;
   }
}

//************************************************************************
#define  LOW_STEPS   90
#define  MID_STEPS   90
#define  FIRE_STEPS  (LOW_STEPS+MID_STEPS)

void flames::init_fire_palette(void)
{
   unsigned num ;
   // if (fire_rows == 0  ||  fire_cols == 0)
   //    return ;

   //  init palette record
   if (fire_palette_record != 0) {
      delete[](fire_palette_record) ;
   }
   unsigned fire_elements = (fire_rows+1) * (fire_cols+1) ;
   fire_palette_record = new u8[fire_elements] ;   // NOLINT(cppcoreguidelines-owning-memory)
   memset(fire_palette_record, 0, fire_elements) ;
   // wsprintf(tempstr, "fire record [0x%08X]: %u x %u = %u\n", 
   //    (unsigned) fire_palette_record,
   //    fire_rows, fire_cols, fire_elements) ;
   // OutputDebugString(tempstr) ;

   //  only init the actual palette array once
   if (fire_palette_init)
      return ;

   //  0-63, red 0-63
   // for(num=0;num!=64;num++) palette(num, num, 0, 0);
   // for(num=0;num!=64;num++)    set_fire_palette(num, num*4,   0, 0);
   for(num=0; num < LOW_STEPS; num++) {
      set_fire_palette(num, (u8) (num*256/LOW_STEPS),   0, 0);
   }

   //  64-127, red 63, green 64-128
   // for(num=64;num!=128;num++) palette(num, 63, num, 0);
   // for(num=64;num!=128;num++)  set_fire_palette(num,  255, num*4, 0);
   for(num=LOW_STEPS; num < FIRE_STEPS; num++) {
      // set_fire_palette(num, 255, num*4, 00);
      set_fire_palette(num, 255, (u8) (num*256/MID_STEPS), 0);
   }

   //  128-255  black
   // for(num=128;num!=256;num++) palette(num, 0, 0, 0);
   // for(num=128;num!=256;num++) set_fire_palette(num,   0,   0, 0);
   for(num=(FIRE_STEPS); num!=256; num++) set_fire_palette(num,   0,   0, 0);
   fire_palette_init = 1 ;
   // dump_fire_palette() ;
   update_global_palette28() ;
}

//************************************************************************
COLORREF flames::get_fire_palette(unsigned index)
{
   rgb_p fptr  = &fire_palette[index] ;
   return RGB(fptr->red, fptr->green, fptr->blue);
}

//************************************************************************
void flames::draw_fire_element(HDC hdc, unsigned x, unsigned y, unsigned color)
{
   set_palette_index(x, y, (u8) color) ;
   unsigned col = x * fire_char_width ;
   unsigned row = y * fire_char_height ;
   SolidRect(hdc, col, row, col+fire_char_width, row+fire_char_height, get_fire_palette(color));
   // SetPixel(hdc, x, cyGFrame, get_fire_palette(color)) ;
}

//****************************************************************************
// THVGA.H -- Thrillhaus VGA Packet by Matt Gulden (webmaster@thrillhaus.com)
// This file originated at http://www.karland.com/code/c/
// It is public domain and may be modified and/or redistributed.
//****************************************************************************
void flames::update_display()
{
   int x, y, x_temp, color ;
   static unsigned y_thold2 = 0 ;

   if (pause_the_race  ||  cxGFrame == 0  ||  cyGFrame == 0)
      return ;

   cycle_count++ ;
   HDC hdc = get_gframe_dc() ;
   if (we_should_redraw) {
      Clear_Window(hdc, 0);

      fire_char_width  = 4 ;
      fire_char_height = 4 ;
      fire_cols = (unsigned) cxGFrame / fire_char_width ;
      fire_rows = (unsigned) cyGFrame / fire_char_height ;
      // y_thold2 = fire_rows * 65 / 199 ;
      y_thold2 = 8 ;
      // syslog("fire: wspace=%ux%u, element size=%ux%u, count=%ux%u\n", 
      //    cxClient, cyGFrame,
      //    fire_char_width, fire_char_height,
      //    fire_cols, fire_rows) ;

      //  call this *after* fire params are initialized!!
      init_fire_palette() ;
   }

   //**********************************************************************
   //  The lowest row of pixels consist of random colors selected
   //  from the last 15 (by default) colors in the fire palette.
   //  This number determines how much variation (in color) exists in 
   //  the initial drawn row.
   //**********************************************************************

   //**********************************************************************
   //  Draw randomly colored line at bottom of screen
   //**********************************************************************
   // for(x=0;x!=320;x++)   pixel(x, 199, 127-random(15));
   for(x=0; x < (int) fire_cols; x++) {
      // pixel(x, 199, 127-random(15));
      color = FIRE_STEPS - random_int(15) ;
      // color = FIRE_STEPS - random_int(30) ;
      draw_fire_element(hdc, x, fire_rows, color) ;
   }

   //**********************************************************************
   //  draw all subsequent rows
   //**********************************************************************
   for(y = y_thold2; y < (int) fire_rows; y++) {
   // for(x=0;x!=320;x++) {
      for(x=0; x < (int) fire_cols; x++) {
      // for(y=115;y!=199;y++) {
         //  Each time a row is drawn, the target color is decremented by
         //  COLOR_DECREMENT.  Thus, the "current color index" walks down
         //  through the palette until it reaches 0.
         //  Making COLOR_DECREMENT smaller makes the flames taller,
         //  and vice-versa.
         // color = point(x, y+1) - 3 ;
         color = get_palette_index(x, y+1) - COLOR_DECREMENT ;
         if (color > 0) {
            //  the random decrement here causes flames to lean to the left,
            //  since the color is copied to a pixel positioned to the left
            //  of where it was read from the previous row.
            // pixel(x-random_int(2), y, color);
            x_temp = x - random_int(2) ;
            if (x_temp < 0) 
                x_temp += (int) fire_cols ;
            draw_fire_element(hdc, x_temp, y, color) ;
         }
      }
   }

   release_gframe_dc(hdc) ;
}
