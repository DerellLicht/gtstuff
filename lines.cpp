#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"  
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

//************************************************************************
void lines::update_display()
{
   int idx ;

   if (pause_the_race)
      return ;

   // SetWindowText(hwnd, title) ;
   cycle_count++ ;
   HDC hdc = get_gframe_dc() ;
   // COLORREF attr = RGB(random_int(256), random_int(256), random_int(256)) ;
   orient ^= 1 ;
   if (orient) {
      //  horizontal, pick random y
      idx = random_int(cyGFrame) ;  /* Set the y coord of point    */
      LineCR(hdc, 0, idx, cxGFrame, idx, random_colorref());
   } else {
      idx = random_int(cxGFrame) ;  /* Set the y coord of point    */
      LineCR(hdc, idx, 0, idx, cyGFrame, random_colorref());
   }

   release_gframe_dc(hdc) ;
}
