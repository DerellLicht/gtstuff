#include <windows.h>
#include <utility>      //  std::move

#include "common.h"     //  u8, etc
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

#define  MAX_BOX_EDGE   100

//***********************************************************************
squares::squares(std::string title_text) 
: graph_object(std::move(title_text)) 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void squares::update_display(void)
{
   HBRUSH hBrush ;
   HDC    hdc ;
   RECT   rect ;
   // POINT poly[5];     /* Space to hold datapoints  */
   int xl, yl, xr, yu ;

   if (pause_the_race)
      return ;

   cycle_count++ ;
   if (use_solid_pattern)
      hBrush = CreateSolidBrush (random_colorref()) ;
      // hBrush = CreateSolidBrush (random_palette_ref()) ;
   else
      hBrush = CreateHatchBrush (fill_patterns[random_int(6)], random_colorref()) ;

   hdc = get_gframe_dc() ;
   SetBkColor(hdc, random_colorref()) ;

   xl = random_int(cxGFrame) ;
   yl = random_int(cyGFrame) ;
   if (use_solid_pattern == 0) {
      yu = yl + 1 + random_int(MAX_BOX_EDGE) ;
      xr = xl + 1 + random_int(MAX_BOX_EDGE) ;
   } else {
      xr = xl + (int) CONST_SIZE ;
      yu = yl + (int) CONST_SIZE ;
   }

   SetRect (&rect, xl, yl, xr, yu) ;
   FillRect (hdc, &rect, hBrush) ;
   Box(hdc, xl, yl, xr, yu, (COLORREF) 0) ;
   release_gframe_dc(hdc) ;

   DeleteObject (hBrush) ;
}
