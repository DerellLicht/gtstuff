#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

//***********************************************************************
bitblt::bitblt() 
: graph_object() 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
//*                         
//* Concentric_Rect(l, t, r, b)                  
//* Subroutine for the BitBlt tests, 
//  draws concentric rectangles in various colors                
//*                         
//************************************************************************/
void bitblt::Concentric_Rect(HDC hdc, int l, int t, int width, int height)
{
   int   r, b, dx, dy;
   r  = l + width - 1;
   b  = t + height -1;
   dx = (r-l)/16;
   dy = (b-t)/16;
   Solid_Rect(hdc, l + 1*dx, t + 1*dy, r - 1*dx, b - 1*dy, 4);
   Solid_Rect(hdc, l + 2*dx, t + 2*dy, r - 2*dx, b - 2*dy, 2);
   Solid_Rect(hdc, l + 3*dx, t + 3*dy, r - 3*dx, b - 3*dy, 1);
   Solid_Rect(hdc, l + 4*dx, t + 4*dy, r - 4*dx, b - 4*dy, 0);
}

//************************************************************************
void bitblt::update_display()
{
   if (!we_should_redraw) 
      return ;

   HDC hdc = get_gframe_dc() ;

   Clear_Window(hdc, 0);
   int dx = (cxGFrame+1)/3;
   int dy = (cyGFrame+1)/3;

   /****************************************************************/
   /* Copy BITBLT                   */
   /****************************************************************/

   Concentric_Rect(hdc, 0, 0, dx, dy);
   dBitBlt(hdc, 0, 0, dx/2, dy/2, dx, dy, ROP_SRC);

   /****************************************************************/
   /* Copy using XOR                */
   /****************************************************************/

   Concentric_Rect(hdc, 3*dx/2, 0, dx, dy);
   dBitBlt(hdc, 3*dx/2, 0, 4*dx/2, dy/2, dx, dy, ROP_XOR);

   /****************************************************************/
   /* Copy using AND                */
   /****************************************************************/

   Concentric_Rect(hdc, 0, 3*dy/2, dx, dy);
   dBitBlt(hdc, 0, 3*dy/2, dx/2, 4*dy/2, dx, dy, ROP_AND);

   /****************************************************************/
   /* Copy using OR                 */
   /****************************************************************/

   Concentric_Rect(hdc, 3*dx/2, 3*dy/2, dx, dy);
   dBitBlt(hdc, 3*dx/2, 3*dy/2, 4*dx/2, 4*dy/2, dx, dy, ROP_OR);

   Line(hdc, 0, 0, cxGFrame, 0, 255);
   Line(hdc, 0, cyGFrame/2, cxGFrame, cyGFrame/2, 255);
   Line(hdc, 0, cyGFrame, cxGFrame, cyGFrame, 255);
   Line(hdc, cxGFrame/2, 0, cxGFrame/2, cyGFrame, 255);
   Line(hdc, 0, 0, 0, cyGFrame, 255);
   Line(hdc, cxGFrame, 0, cxGFrame, cyGFrame, 255);

   release_gframe_dc(hdc) ;
}
