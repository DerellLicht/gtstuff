#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"       //  cycle_count

//***********************************************************************
rcolors::rcolors() 
: graph_object() 
, char_width(0)
, char_height(0)
, rows(0)
, columns(0)
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
   // char_width = 0 ;
   // char_height = 0 ;
   // rows = 0 ;
   // columns = 0 ;
}

//************************************************************************
void rcolors::log_char_dimens()
{
   char tempstr[81];
   wsprintf(tempstr, "char dimens: %ux%u", columns, rows);
   status_message(1, tempstr);
}

//************************************************************************
void rcolors::update_display()
{
   // rcolors32_hfont = build_font("FixedSys", 18, 0, 0, 0, 0) ;
   static LOGFONT const lfont = {
      18,   //  height
      0,    //  width
      0,    //  escapement
      0,    //  orientation
      FW_NORMAL,  //  FW_NORMAL or FW_BOLD
      0,    //  italic
      0,    //  underline
      0,    //  strikeout
      DEFAULT_CHARSET,
      0,
      0,
      0,
      DEFAULT_PITCH,
      "FixedSys"
   } ;
   static HFONT rcolors32_hfont = 0 ;
   char str[2] ;

   if (pause_the_race)
      return ;

   cycle_count++ ;
   // COLORREF pcolor = random_colorref() ;
   // COLORREF pcolor = get_palette_entry(random_int(256)) ;
   unsigned col = (unsigned) random_int(columns) * char_width ;
   unsigned row = (unsigned) random_int(rows) * char_height ;
   log_char_dimens();
   
   // str[0] = random_int(93) + 33 ;
   str[0] = (use_solid_pattern) ? (char) 0x7F : (char) random_int(256) ;

   //  Interestingly, *some* fixed-point fonts (such as Terminal and MS LineDraw)
   //  act as if they are non-fixed-point fonts, drawing with variable widths...
   //  Maybe later I can have the detection mechanism walk through all 256 characters
   //  and find max height/width...
   // hfont = build_font("Courier New", 20, 0, 0, 0, 0) ;

   // Another example: to create a GDI font handle in
   // Win32 you can set up a LOGFONT structure and call CreateFontIndirect(),
   // or you can call CreateFont(), which has a really long parameter list
   // consisting exactly of all the members of a LOGFONT. 
   // In CE, CreateFont() doesn't exist so you have to use CreateFontIndirect().
   if (rcolors32_hfont == 0)
       // rcolors32_hfont = build_font("FixedSys", 18, 0, 0, 0, 0) ;
       rcolors32_hfont = CreateFontIndirect(&lfont) ;

   HDC hdc = get_gframe_dc() ;
   SelectObject (hdc, rcolors32_hfont) ;
   if (we_should_redraw) {
      str[1] = 0 ;
      Clear_Window(hdc, 0);
      SetBkMode(hdc, OPAQUE) ;
      SIZE ssize ;
      GetTextExtentPoint32(hdc, str, 1, &ssize) ;
      char_width  = ssize.cx ;
      char_height = ssize.cy ;
      columns = (unsigned) cxGFrame / (unsigned) ssize.cx ;
      rows    = (unsigned) cyGFrame / (unsigned) ssize.cy ;
   }

   SetBkColor  (hdc, 0) ;
   // SetTextColor(hdc, random_palette_ref()) ;
   SetTextColor(hdc, random_colorref()) ;
   TextOut (hdc, col, row, str, 1);
   //  don't delete my font!!
   // DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT)));

   release_gframe_dc(hdc) ;
}
