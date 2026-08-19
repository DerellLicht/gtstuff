#include <windows.h>

#include "gstuff.h"     //  for timer functions
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"       //  we_should_redraw
#include "ezfont.h"

//************************************************************************
#define  DAT_XI   30
#define  DAT_YI   30
#define  DAT_DX   35
#define  DAT_DY   35
#define  DAT_FH   25

static char ascii_font_name[LF_FULLFACESIZE] = "Wingdings" ;
//***********************************************************************
ascii::ascii(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void ascii::set_font_name(char *new_font_name)
{
   if (new_font_name != 0) {
      strncpy(ascii_font_name, new_font_name, LF_FULLFACESIZE) ;
      we_should_redraw = 1 ;
   }
}

//************************************************************************
char *ascii::get_font_name(void)
{
   return ascii_font_name ;
}

//************************************************************************
void ascii::update_display()
{
   char str[2] ;

   if (!we_should_redraw) 
      return ;

   // hfont = build_font("Courier New", 20, 0, 0, 0, 0) ;
   HFONT hfont = build_font(ascii_font_name, DAT_FH, 0, 0, 0, 0) ;
   if (hfont == 0) {
      wsprintf(tempstr, "build_font: %s\n", get_system_message()) ;
      OutputDebugString(tempstr) ;
   }
   HDC HDC hdc = get_gframe_dc() ;
   SelectObject (hdc, hfont) ;
   Clear_Window(hdc, 0);
   SetBkMode(hdc, OPAQUE) ;
   SetBkColor  (hdc, 0) ;
   // SetTextColor(hdc, random_palette_ref()) ;
   SetTextColor(hdc, WIN_GREEN) ;
   unsigned row, col ;
   u8 idx = 0 ;
   unsigned row_pixel = DAT_YI ;
   for (row=0; row<16; row++) {
      unsigned col_pixel = DAT_XI ;
      for (col=0; col<16; col++) {
         str[0] = idx++ ;  //lint !e734
         SetTextColor(hdc, random_palette_ref()) ;
         TextOut (hdc, col_pixel, row_pixel, str, 1);
         col_pixel += DAT_DX ;
      }
      row_pixel += DAT_DY ;
   }
   DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));
   release_gframe_dc(hdc) ;
}

//************************************************************************
//  return TRUE if we handled this key,
//  FALSE if we did not
//************************************************************************
bool ascii::process_key(unsigned key_id)
{
   return FALSE ;
}

