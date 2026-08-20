#include <windows.h>

#include "common.h"     //  u8, etc
#include "gtstuff.h"
#include "palettes.h"   //  24-bit palette functions
#include "gobjects.h"   //  graphics-object classes
#include "gfuncs.h"     //  graphics primitives
#include "alg_selector.h"

//***********************************************************************
rect::rect() 
: graph_object() 
//  per http://www.acm.org/crossroads/xrds1-4/ovp.html
// , _v1(v1), _v2(v2), _v3(v3)
{ 
}

//************************************************************************
void rect::update_display()
{
   HDC hdc ;
   int layer,row,col,sx,sy,x,y,color;

   if (!we_should_redraw) 
      return ;

   syslog("Rect, palette #%d\n", get_curr_palette()) ;

   hdc = get_gframe_dc() ;
   Clear_Window(hdc, 0);
   //  these should derive from c[xy]GFrame
   sx   = cxGFrame/16;
   sy   = cyGFrame/12;
   for (layer = 0; layer < 5; layer++) {//  select layer
      for (row = 0; row < 6; row++) {//  select row
         for (col = 0; col < 8; col++) {  //  select column
            x   = col * cxGFrame/8 + layer * sx/5;
            y   = row * cyGFrame/6 + layer * sy/5;
            color = 16 + col + 8 * (row + 6 * layer);
            Solid_Rect(hdc, x, y, x+sx-1, y+sy-1, (BYTE) color);
         }
      }
   }

   release_gframe_dc(hdc) ;
}
