#include <windows.h>
#include <utility>      //  std::move

#include "common.h"     //  u8, etc
#include "gtstuff.h" 
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

#define  MAX_CIRCLE_DIAM   60

//***********************************************************************
circles::circles(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void circles::update_display(void)
{
   HBRUSH hBrush ;
   int x, y, w ;

   if (pause_the_race)
      return ;

   cycle_count++ ;
   if (use_solid_pattern)
      hBrush = CreateSolidBrush (random_colorref()) ;
      // hBrush = CreateSolidBrush (random_palette_ref()) ;
   else
      hBrush = CreateHatchBrush (fill_patterns[random_int(6)], random_colorref()) ;
      
   HDC hdc = get_gframe_dc() ;
   SetBkColor(hdc, random_colorref()) ;
   HGDIOBJ hOldBrush = SelectObject(hdc, hBrush) ;

   x = random_int(cxGFrame) ;
   y = random_int(cyGFrame) ;
   if (!use_solid_pattern) {
      w = 1 + random_int(MAX_CIRCLE_DIAM) ;
   } else {
      w = CONST_SIZE / 2 ;
   }
   Ellipse(hdc, x-w, y-w, x+w, y+w) ;

   //  Claude 08/17/26 - restore the DC's original brush *before* releasing
   //  it and deleting ours -- deleting a GDI object while it's still
   //  selected into a DC is undefined behavior per MSDN. squares.cpp avoids
   //  this entirely by using FillRect() (which takes the brush as a plain
   //  parameter, no SelectObject needed); gfuncs.cpp's Line()/LineCR()/Box()
   //  already do this correctly for pens. This was the one place that
   //  didn't follow that pattern.
   SelectObject(hdc, hOldBrush) ;
   release_gframe_dc(hdc) ;

   DeleteObject (hBrush) ;
}
