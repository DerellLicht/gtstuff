#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

#define  DLG_X0   8
#define  DLG_Y0   8
#define  DLG_DX   5
#define  DLG_DY   6

//***********************************************************************
triangles::triangles() 
: graph_object() 
{ 
}

//************************************************************************
//  Claude - free any cached pens from a previous palette so they can be
//  recreated (lazily, on next use) with the new palette's colors. Safe
//  to call even if some/all entries are still nullptr.
//************************************************************************
void triangles::release_cached_pens()
{
   // for (int i = 0; i < 256; i++) {
   for (auto &s_pen : s_pens) {
      if (s_pen) {
         DeleteObject(s_pen) ;
         s_pen = nullptr ;
      }
   }
}

//************************************************************************
void triangles::update_display()
{
   int j, x, y, x1, y1 ;
   double m ;
   static unsigned attr = 1 ;
   // HPEN hPen ;
   POINT pts[4] ;
   unsigned max_colors = get_palette_entries() ;

   if (pause_the_race)
      return ;
   cycle_count++ ;
   
   HDC hdc = get_gframe_dc() ;
   if (we_should_redraw) {
      Clear_Window(hdc, 0);
      
      unsigned active_palette_index = get_curr_palette() ;
      if (current_palette_index != active_palette_index) {  // whatever your global/accessor is called
         release_cached_pens() ;
         current_palette_index = active_palette_index ;
      }
   }

   x = DLG_X0 ; //  x offset
   y = DLG_Y0 ; //  y offset
   x1 = cxGFrame - DLG_X0 ;
   y1 = cyGFrame - DLG_Y0 ;
   for (j=1; j<=20; j++) {
      // hPen = CreatePen(PS_SOLID, 1, get_palette_entry(attr)) ;
      if (!s_pens[attr]) {
         s_pens[attr] = CreatePen(PS_SOLID, 1, get_palette_entry(attr)) ;
      }
      SelectObject(hdc, s_pens[attr]) ;

      pts[0].x = x ;
      pts[0].y = y ;
      pts[1].x = x1 ;
      pts[1].y = y ;
      pts[2].x = x1 ;
      pts[2].y = y1 ;
      pts[3].x = x ;
      pts[3].y = y ;
      Polyline(hdc, &pts[0], 4) ;

      // SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
      // DeleteObject (hPen) ;

      //  update our points
      m = (double) ((double) (x1 - x + 1) 
                  / (double) (y1 - y + 1)) ;
      if (m >= 0.0) {
         x += (int) (5.0 * m) ;   //  x delta
         y += DLG_DX ;   //  y delta
      } else {
         y += (int) (5.0 * m) ;   //  x delta
         x += DLG_DX ;   //  y delta
      }
      x1 -= DLG_DX ;
      y1 -= DLG_DY ;
      if (++attr >= max_colors) {
         attr = 1 ;
      }
   }

   release_gframe_dc(hdc) ;
}
