#include <windows.h>

#include "common.h"     
#include "gtstuff.h"    //  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

#define  PIX_DX   3
#define  PIX_DY   3

//***********************************************************************
pixels::pixels() 
: graph_object() 
, dp_char_width(0)
, dp_char_height(0)
, rows(0)
, columns(0)
, color(0)
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void pixels::log_pixel_dimens()
{
   char tempstr[81];
   wsprintf(tempstr, "pixel dimens: %ux%u", columns, rows);
   status_message(1, tempstr);
}

//************************************************************************
void pixels::update_display()
{
   COLORREF pcolor ;

   if (pause_the_race)
      return ;

   // HDC hdc = GetDC (hwnd) ;
   HDC hdc = get_gframe_dc() ;
   if (we_should_redraw) {
      color = 0 ;
      Clear_Window(hdc, 0);
      dp_char_width  = PIX_DX ;
      dp_char_height = PIX_DY ;
      columns = (unsigned) cxGFrame / dp_char_width ;
      rows    = (unsigned) cyGFrame / dp_char_height ;
      
      log_pixel_dimens();
   }
   cycle_count++ ;
   // COLORREF pcolor = random_colorref() ;
   if (use_solid_pattern) {
      pcolor = random_colorref() ;
   } else {
      pcolor = get_palette_entry(color) ;
      if (++color >= get_palette_entries())
         color = 0 ;
   }
   unsigned col = random_int(columns) ;
   unsigned row = random_int(rows) ;
   col *= dp_char_width ;
   row *= dp_char_height ;
   SolidRect(hdc, col, row, col+PIX_DX, row+PIX_DY, pcolor);
   release_gframe_dc(hdc) ;
}
